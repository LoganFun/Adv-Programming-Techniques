/*
Author: Zilong Fan
Class: ECE6122 
Last Date Modified: 28/11/2023
Description:
LAB 6 mpi ESTIMATE
*/


#include <cmath>
#include <ctime>
#include <math.h>
#include <random>
#include <iostream>
#include <cstdlib>
#include <mpi.h>


// Function prototypes
double x_square(double x);
double function_e(double x);
double monteCarloEstimate(int choice,  unsigned long num_samples, int rank, int size);

int main(int argc, char** argv) {
    // Initialize MPI Settings
    MPI_Init(&argc, &argv);

    // Get the Size and position
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Parse command line arguments
    // Integral choice is the second argument
    int functionChoice = std::atoi(argv[2]);  
    // Number of samples is the fourth argument
    unsigned long sampleNumber = std::atoll(argv[4]);  
    // Seed the random number generator
    srand(std::time(0) + world_rank); 

    // Calculate per node
    double estimate_value = monteCarloEstimate(functionChoice, sampleNumber, world_rank, world_size);

    
    double all_sum;
    // Sum all the value
    MPI_Reduce(&estimate_value, &all_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print out once
    if (world_rank == 0) 
    {
        // Adjust according to the function range if necessary
        all_sum = (all_sum / world_size); 
        
        std::cout << "The estimated value of integral "<< functionChoice << " is " << all_sum << std::endl;
        std::cout << "Bye !" << std::endl;
    }

    MPI_Finalize();
    return 0;
}

// Function f(x) = x^2
double x_square(double x) 
{
    return x * x;
}

// Function f(x) = e^(-x^2)
double function_e(double x) 
{
    return exp(-x * x);
}

double monteCarloEstimate(int choice, unsigned long num_samples, int rank, int size) {
    double sum = 0.0;
    unsigned long samples_pernode = num_samples / size;
    double random_value;

    for (unsigned long i = 0.0; i < samples_pernode; ++i) 
    {
        // Generate random x in [0,1]
        random_value = static_cast<double>(rand()) / RAND_MAX; 

        if (choice == 1) 
        {
            sum += x_square(random_value);
        } 
        else if (choice == 2) 
        {
            sum += function_e(random_value);
        }
        else
        {
            std::cerr << "Wrong Function Choice " << std::endl;
            MPI_Finalize();
        }
    }

    // Multiply by interval width
    double average_value = (sum / samples_pernode) * 1.0; 

    return average_value;
}