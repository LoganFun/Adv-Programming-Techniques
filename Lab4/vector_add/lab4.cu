/*
Author: Zilong Fan
Class:ECE 6122
Last Date Modified: 20231103

Description:
Random 2D Walker
*/

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <curand_kernel.h>
#include <iostream>

#define BLOCK_SIZE 256

inline cudaError_t checkCuda(cudaError_t result)
{
#if defined(DEBUG) || defined(_DEBUG)
  if (result != cudaSuccess) 
  {
    fprintf(stderr, "CUDA Runtime Error: %s\n", cudaGetErrorString(result));
    assert(result == cudaSuccess);
  }
#endif
  return result;
}


__global__ void randomWalker(float* positions, int numSteps, int numWalkers, 
    unsigned int seed) {
    //Find the id 
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    //Random Initialize
    curandState state;
    curand_init(seed, idx, 0, &state);

    if (idx < numWalkers) {
        int x = 0;
        int y = 0;

        for (int i = 0; i < numSteps; i++) {

            int direction = (int)(curand_uniform(&state) * 4);
            switch (direction) {
                case 3: x++; break; // west
                case 2: x--; break; // east
                case 1: y++; break; // south
                case 0: y--; break; // north
            }
        }

        // Store the final position.
        positions[idx * 2] = x;
        positions[idx * 2 + 1] = y;
        
    }
}

float average(float* positions, int numWalkers) {
    float totalDistance = 0.0;

    for (int i = 0; i < numWalkers; i++) {
        //take the value
        float y = positions[i * 2 + 1];
        float x = positions[i * 2];
        //square x^2 y^2
        float distance = sqrtf(y * y + x * x  );
        totalDistance = totalDistance + distance;
    }

    return totalDistance / numWalkers;
}

void func_CudaMalloc(float* d_positions,float* h_positions, int numSteps, int numWalkers,float& avgDist
    , int numBlocks)
{
    cudaMalloc((void**)&d_positions, numWalkers * 2 * sizeof(float));

    // Launch the randomWalk kernel
    
    randomWalker<<<numBlocks, BLOCK_SIZE>>>(d_positions, numSteps, numWalkers, time(NULL));
    checkCuda(cudaGetLastError());
    cudaDeviceSynchronize();  // Wait for the GPU to finish

    // Copy the positions back to host memory
    cudaMemcpy(h_positions, d_positions, numWalkers * 2 * sizeof(float), cudaMemcpyDeviceToHost);

    //calculate Distance
    avgDist = average(h_positions, numWalkers);
    
    // Free the memory
    cudaFree(d_positions);
    delete[] h_positions;
}

void func_CudaMallocHost(float* d_positions,float* h_positions, int numSteps, int numWalkers,float& avgDist, int numBlocks)
{
    checkCuda( cudaMallocHost((void**)&h_positions, numWalkers * 2 * sizeof(float)) );
    checkCuda( cudaMalloc((void**)&d_positions, numWalkers * 2 * sizeof(float)) );

    randomWalker<<<numBlocks, BLOCK_SIZE>>>(d_positions, numSteps, numWalkers, time(NULL));
    checkCuda(cudaGetLastError());
    cudaDeviceSynchronize();

    cudaMemcpy(h_positions, d_positions, numWalkers * 2 * sizeof(float), cudaMemcpyDeviceToHost);
    avgDist = average(h_positions, numWalkers);
    cudaFree(d_positions);
    cudaFreeHost(h_positions);
}

void func_CudaMallocManaged(float* h_positions, int numSteps, int numWalkers,float& avgDist, int numBlocks)
{
    checkCuda( cudaMallocManaged(&h_positions, numWalkers * 2 * sizeof(float)) );
    randomWalker<<<numBlocks, BLOCK_SIZE>>>(h_positions, numSteps, numWalkers, time(NULL));
    checkCuda(cudaGetLastError());
    cudaDeviceSynchronize();
    avgDist = average(h_positions, numWalkers);
    cudaFree(h_positions);
}


int main(int argc, char* argv[]) {
    float Time;
    // events for timing
    cudaEvent_t startEvent, stopEvent; 
    checkCuda( cudaEventCreate(&startEvent) );
    checkCuda( cudaEventCreate(&stopEvent) );    

    int numWalkers = atoi(argv[2]);
    int numSteps = atoi(argv[4]);
    int numBlocks = (numWalkers + BLOCK_SIZE - 1) / BLOCK_SIZE;

    float* d_positions;
    float* h_positions = new float[numWalkers * 2];
    float avgDist;

    // 1. Normal CUDA Memory Allocation
    // Start Initial

    func_CudaMallocManaged(h_positions,numSteps, numWalkers,avgDist,numBlocks);

    cudaEventRecord(startEvent, 0);

    func_CudaMalloc(d_positions,h_positions,numSteps,numWalkers,avgDist,numBlocks);

    //Stop Initial
    cudaEventRecord(stopEvent, 0);
    cudaEventSynchronize(stopEvent);
    cudaEventElapsedTime(&Time, startEvent, stopEvent);

    // Calculate the average distance on the host
    
    std::cout << "Normal CUDA memory Allocation: " << std::endl;
    std::cout << "  Time to calculate(ms): " << Time*1000 << std::endl;
    std::cout << "  Average distance from origin: " << avgDist << std::endl;     

    // 2. Pinned Memory
    cudaEventRecord(startEvent, 0);
    func_CudaMallocHost(d_positions,h_positions,numSteps,numWalkers,avgDist,numBlocks);

    cudaEventRecord(stopEvent, 0);
    cudaEventSynchronize(stopEvent);
    cudaEventElapsedTime(&Time, startEvent, stopEvent);
    
    std::cout << "Pinned Memory Allocation: " << std::endl;
    std::cout << "  Time to calculate(ms): " << Time*1000 << std::endl;
    std::cout << "  Average distance from origin: " << avgDist << std::endl;  

    // 3. Unified Memory
    cudaEventRecord(startEvent, 0);
    
    func_CudaMallocManaged(h_positions,numSteps, numWalkers,avgDist,numBlocks);

    cudaEventRecord(stopEvent, 0);
    cudaEventSynchronize(stopEvent);
    cudaEventElapsedTime(&Time, startEvent, stopEvent);
    
    std::cout << "Unified Memory Allocation: " << std::endl;
    std::cout << "  Time to calculate(ms): " << Time*1000 << std::endl;
    std::cout << "  Average distance from origin: " << avgDist << std::endl;  
    
    // Free unified memory
    //cudaFree(h_positions);

    return 0;
}

