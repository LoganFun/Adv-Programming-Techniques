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
//const int NUM_THREADS = 1;
std::atomic<int> threadsStatus[100]; 
std::mutex mtxCout;

void writeToConsole(const std::string& strOut)
{
    std::unique_lock<std::mutex> lck1(mtxCout);
}

int determine_Rounds(int total_Charge_Num)
{
    int n = total_Charge_Num / NUM_THREADS;
    return n;
}

int determine_Rounds_Assist(int total_Charge_Num)
{
    int n = total_Charge_Num % NUM_THREADS;
    return n;
}

// Thread working process
void workerThread(int id, int total_Charge_Num, int m, 
    std::vector<std::vector<ECE_ElectricField>>* matrix_addr,
    double x, double y, double z
    , ECE_ElectricField* final_E_addr
)
{
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
        int rounds = determine_Rounds(total_Charge_Num);
        int rounds_assist = determine_Rounds_Assist(total_Charge_Num);
        //std::cout << "total_Charge_Num: " << total_Charge_Num << std::endl;
        //std::cout << "rounds: " << rounds << std::endl;
        //std::cout << "rounds_assist: " << rounds_assist << std::endl;
        
        // Do the calculation -- hard
        if (id<rounds_assist)
        {
            //std::cout << "Working id: " << id << std::endl;
            for (int k = 0; k < rounds +1; k++)
            {
                int location = (rounds+1) * id+k;
                int n_new = location / m;
                int m_new = location % m;
                //std::cout << "                     coordinate: " << n_new << m_new << std::endl;       
                //(*matrix_addr)[n_new][m_new].Zero();
                (*matrix_addr)[n_new][m_new].computeFieldAt(x, y, z);
                (*matrix_addr)[n_new][m_new].exe();
                //matrix[n_new][m_new].getElectricField_self();
                (*final_E_addr).find_sum((*matrix_addr)[n_new][m_new]);
                
            }
        }
        else
        {
            //std::cout << "Working id: " << id << std::endl;
            for (int k = 0; k < rounds ; k++)
            {
                int location = (rounds + 1) * (rounds_assist ) + rounds * (id - rounds_assist) + k;
                int n_new = location / m;
                int m_new = location % m;
                //std::cout << "x,y,z!!!!!!!" << x << y << z << std::endl;
                //(*matrix_addr)[n_new][m_new].Zero();
                (*matrix_addr)[n_new][m_new].computeFieldAt(x, y, z);
                (*matrix_addr)[n_new][m_new].exe();
                //matrix[n_new][m_new].getElectricField_self();
                (*final_E_addr).find_sum((*matrix_addr)[n_new][m_new]);
            }
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
            //lck1.unlock();
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

bool ifNatural(const int num)
{
    if (num>=0) {
        return true;
    }
    return false; 
}

void query_user_1(int &n, int &m, double &q, double& x_D, double& y_D)
{
    std::cout << "Please enter the number of rows and colomns in the N * M array: ";
    std::cin >> n >> m;
    while(! (ifNatural(n) && ifNatural(m)))
    {
        std::cout << "Please enter the number of rows and colomns in the N * M array again: ";
        std::cin >> n >> m;
    }

    std::cout << "Please enter the x and y separation distances in meters: ";
    std::cin >> x_D >> y_D;
    while (!((x_D>0) && (y_D > 0)))
    {
        std::cout << "Please enter the x and y separation distances in meters again: ";
        std::cin >> x_D >> y_D;
    }

    std::cout << "Please enter the common charge on the points in micro C: ";
    std::cin >> q;
    while (q==0)
    {
        std::cout << "Please enter the common charge on the points in micro C again: ";
        std::cin >>q;
    }

    std::cout << std::endl;
}\

bool ifVaild(int x, int y, int z,int n, int m,  std::vector<std::vector<ECE_ElectricField>>& matrix)
{
    bool signal = 1;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (matrix[i][j].find_x() == x
                && matrix[i][j].find_y() == y
                && matrix[i][j].find_z() == z)
                signal = 0;
        }
    }
    return signal;
}

void query_user_2(double& x, double& y, double& z, int n, int m, std::vector<std::vector<ECE_ElectricField>>& matrix)
{
    std::cout << "Please enter the location in space to determine the electric field (x, y, z) in meters: ";
    std::cin >> x >> y >> z;
    while (!ifVaild(x, y, z, n, m, matrix))
    {
        std::cout << "Please enter the location in space to determine the electric field (x, y, z) in meters agin: ";
        std::cin >> x >> y >> z;
    }

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
    return 0;
}

int main()
{
    bool repeat = 0;
    do 
    {
        // Initialize Signals
        bool AllFinished = 0;
        int n = 0;
        int m = 0;
        double q = 0;
        double x_D, y_D = 0.0;
        double x, y, z = 0.0;
        repeat = 0;
        std::vector<std::vector<ECE_ElectricField>> matrix;
        std::vector<std::vector<ECE_ElectricField>>* matrix_addr = &matrix;

        // Threads vectors 
        std::vector<std::thread> threads;

        ECE_ElectricField final_E;
        ECE_ElectricField* final_E_addr = &final_E;
        std::cout << std::endl;
        // Task1 Max Concurrently Threads
        concurrent_Thread(NUM_THREADS);

        // Task 2 Query the User of N, M, q
        query_user_1(n, m, q, x_D, y_D);
        //Total number
        int total_Charge_Num = n * m;
        // Task Create 2D Matrix
        charge_Matrix(n, m, q, x_D, y_D, matrix);

        // Task 3 Multi Thread Computing
        //Initialize all the threads state
        for (int i = 0; i < NUM_THREADS; ++i) {
            threadsStatus[i] = eWaiting;
        }

        query_user_2(x, y, z, n, m, matrix);
        final_E.At(x, y, z);
        auto start = std::chrono::high_resolution_clock::now();

        // Launch worker threads and begin workerThread
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.push_back(std::thread(workerThread, i, total_Charge_Num, m, matrix_addr, x, y, z, final_E_addr));
        }

        std::ostringstream ss;  

        // Round 
        //for (int round = 0; round < 3; ++round)
        ss << "****** Starting threads"  << "\n" ;
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

        auto end = std::chrono::high_resolution_clock::now();

        //// Check if threads should be reset
        //for (int j = 0; j < NUM_THREADS; ++j)
        //{
        //    threadsStatus[j] = eWaiting;
        //}


        // Tell all the threads to exit
        for (int j = 0; j < NUM_THREADS; ++j)
        {
            threadsStatus[j] = eExit;
        }

        // Join threads 
        for (auto& thread : threads)
        {
            thread.join();
        }

        //// test all point charge is calculated
        //for (int j = 0; j < n; j++)
        //{
        //    for (int l = 0; l < m; l++)
        //    {
        //        if (matrix[j][l].exe_signal == 0)
        //            std::cout << "wrong!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        //    }
        //}

        // Output E
        final_E.getElectricField_self();

        // Output Time 
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "The calculation took " << duration.count() << " microsec!" << std::endl;

        // Do the repeat or not 
        repeat = whether_Repeat();
        matrix.clear();

    }while (repeat == 1);

    // int Main Return
    return 0;
}