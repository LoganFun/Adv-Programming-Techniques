#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include <chrono>
#include "ECE_ElectricField.h"
#include "ECE_PointCharge.h"

enum enStatus { eWaiting, eRun, eFinished, eExit };
const int NUM_THREADS = (std::thread::hardware_concurrency() - 1);

//????????????????????????????????????????????????????????
std::atomic<int> threadsStatus[100]; 
std::atomic<int> round;
std::mutex mtxCout;

void writeToConsole(const std::string& strOut)
{
    // Manage mutex lock named lck1
    // Meaning ???????
    std::unique_lock<std::mutex> lck1(mtxCout);
    // Output Str
    std::cout << strOut;
}

// Thread working process
void workerThread(int id, int total_Charge_Num, int n, 
    std::vector<std::vector<ECE_ElectricField>> matrix, 
    double x, double y, double z)
{
    // Deal string as std input and output
    std::ostringstream ss;
    while (true)
    {
        // Wait until signaled to start
        while (threadsStatus[id] == eWaiting)
        {
            // give up this thread 
            std::this_thread::yield();
        }

        // Perform some calculation
        ss << "Thread " << id << " is working...\n";
        // create unique lock and output str
        writeToConsole(ss.str()); 
        // clear ss stringstream
        ss.str(""); ss.clear();

        // Allocate the work using id

        int location = round * NUM_THREADS + id;
        if (location <total_Charge_Num)
        {
            int n_new = location / n;
            int m_new = location % n;
            matrix[n_new][m_new].computeFieldAt(x, y, z);
        }

        ss << "Thread " << id << " finished work.\n";
        // create unique lock and output str
        writeToConsole(ss.str());
        // clear ss stringstream
        ss.str(""); ss.clear();

        threadsStatus[id] = eFinished;

        // Wait until signaled what to do next
        while (threadsStatus[id] == eFinished)
        {
            std::this_thread::yield();
        }

        if (threadsStatus[id] == eExit)
        {
            break;
        }
    }

    ss << "Thread " << id << " terminating.\n";
    writeToConsole(ss.str());
    ss.str(""); ss.clear();
}

void concurrent_Thread(const int NUM_THREADS)
{
    std::cout << "The number of concurrently running threads: " << NUM_THREADS + 1 << std::endl;
}

void query_user_1(int &n, int &m,double& q, double& x_D, double& y_D)
{
    std::cout << "Please enter the number of rows and colomns in the N * M array: ";
    std::cin >> n >> m;
    std::cout << "Please enter the x and y separation distances in meters: ";
    std::cin >> x_D >> y_D;
    std::cout << "Please enter the common charge on the points in micro C: ";
    std::cin >> q;
}

void query_user_2(double& x, double& y, double& z)
{
    std::cout << "Please enter the location in space to determine the electric field (x, y, z) in meters: ";
    std::cin >> x >> y >> z;
}

void charge_Matrix(const int& n, const int& m, const double& q, const double& x_D, 
    const double& y_D, std::vector<std::vector<ECE_ElectricField>> & matrix)
{
    // for all the N*M charges
    for (int i = 0; i < n; i++)
    {
        std::vector<ECE_ElectricField> row;
        for (int j = 0; j < m; j++)
        {
            // Coordinate Calculation
            double x = (-1) * (m - 1) * x_D/2 + j * x_D;
            double y = (-1) * (n - 1) * y_D / 2 + i * y_D;
            double z = 0.0; 
            // Initialize Charge
            ECE_ElectricField  obj;
            obj.setLocation(x, y, z);
            obj.setCharge(q);         
            row.push_back(obj);
        }
        matrix.push_back(row);
    }
}

int determine_Rounds( int total_Charge_Num)
{
    int n = total_Charge_Num / NUM_THREADS;
    
    return n + 1;
}

bool whether_Repeat()
{
    char answer;
    bool mistake = 0;
    do 
    {
        if (mistake==1)
            std::cout << "Please input again";
        mistake = 0;
        std::cout << "Do you want to enter a new location (Y/N)? ";
        std::cin >> answer;
        if (answer == 'Y' || answer == 'y')
            return 1;
        else if (answer == 'N' || answer == 'n')
            return 0;
        else
        {
            mistake = 1;
        }
    } while (mistake == 1);
}

int main()
{
    // Initialize Signals
    bool AllFinished;
    int n, m,
    double q;
    double x_D, y_D;
    double x, y, z;
    bool repeat;
    std::vector<std::vector<ECE_ElectricField>> matrix;

    //Total number
    int total_Charge_Num = n * m;

    // Threads vectors 
    std::vector<std::thread> threads;

    do 
    {
        std::cout << std::endl;
        // Task1 Max Concurrently Threads
        concurrent_Thread(NUM_THREADS);

        // Task 2 Query the User of N, M, q
        query_user_1(n, m, q, x_D, y_D);
        query_user_2(x, y, z);
        auto start = std::chrono::high_resolution_clock::now();

        // Task Create 2D Matrix
        charge_Matrix(n, m, q, x_D, y_D, matrix);

        // Task 3 Multi Thread Computing
        //Initialize all the threads state
        for (int i = 0; i < NUM_THREADS; ++i) {
            threadsStatus[i] = eWaiting;
        }

        // Determine Rounds
        int rounds = determine_Rounds(total_Charge_Num);

        // Launch worker threads and begin workerThread
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.push_back(std::thread(workerThread, i, total_Charge_Num, n, matrix, x, y, z));
        }

        std::ostringstream ss;  

        // Round 
        for (round = 0; round < rounds; ++round)
        {
            ss << "****** Starting threads in round: " << round + 1 << "\n";
            writeToConsole(ss.str());
            ss.str(""); ss.clear();

            // Tell threads to start calcualtions
            for (int j = 0; j < NUM_THREADS; ++j)
            {
                threadsStatus[j] = eRun;
            }

            // Do your part of the calculation

            // Wait for the calculations to finish
            do
            {
                AllFinished = true;
                for (int j = 0; j < NUM_THREADS; ++j)
                {
                    if (threadsStatus[j] != eFinished)
                    {
                        AllFinished = false;
                        break;
                    }
                }
                std::this_thread::yield();
            } while (!AllFinished);

            // Check if threads should be reset
            // ??????????????????
            if (round < (3 - 1))
            {
                for (int j = 0; j < NUM_THREADS; ++j)
                {
                    threadsStatus[j] = eWaiting;
                }
            }
            else
            {
                // Tell all the threads to exit
                for (int j = 0; j < NUM_THREADS; ++j)
                {
                    threadsStatus[j] = eExit;
                }

            }

        }



        // Join threads (Note: In this example, the threads run in an infinite loop, so this will hang)
        for (auto& thread : threads)
        {
            thread.join();
        }

        // Time 
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "The calculation took " << duration.count() << " microsec!" << std::endl;

        // Do the repeat or not 
        repeat = whether_Repeat();

    }while (repeat == 1);

    // int Main Return
    return 0;
}