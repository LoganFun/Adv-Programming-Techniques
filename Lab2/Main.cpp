// -----------------------------------------------------------------------//

// Version: 1
// Description :
    // This is the Main cpp file of the project in Lab 2.
    // use OpenMP multithread to do the Electrical Field Calculation
    // make sure the input is correct
// -----------------------------------------------------------------------//

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sstream>      
#include <string>       
#include <chrono>
#include <omp.h>
#include "ECE_ElectricField.h"
#include "ECE_PointCharge.h"

//enum enStatus { eWaiting, eRun, eFinished, eExit };
//const int NUM_THREADS = std::thread::hardware_concurrency();


// get num of thread
int concurrent_Thread()
{
    int x = 0;
    //std::cout << "The max number of concurrently running threads: " << NUM_THREADS << std::endl;
    std::cout << "Please input the number of threads you want (1, 2, 4, 8, 16):";
    std::cin >> x;
    std::cout << std::endl;
    return x;
}

// query n m q x_d y_d
void query_user_1(int &n, int &m, double &q, double& x_D, double& y_D)
{

    while (true)
    {
        std::cout << "Please enter the number of rows and colomns in the N * M array : ";
        std::cin >> m >> n;

        if (n > 0)
        {
            if (m > 0)
            {
                break;
            }
        }
    }
        
        while (true)
        {
            std::cout << "Please enter the x and y separation distances in meters : ";
            std::cin >> x_D >> y_D;

            if (x_D > 0)
            {
                if (y_D > 0)
                {
                    break;
                }
            }
        }

        while (true)
        {
            std::cout << "Please enter the common charge on the points in micro C: ";
            std::cin >> q;

            if (q != 0)
            {
                break;
            }
        }


}

// check whether the coordinate is valid or not
bool ifVaild(double x, double y, double z,int n, int m,  std::vector<std::vector<ECE_ElectricField>>& matrix)
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

//query q
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

// build the q matrix
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
        int num_threads = 0;
        std::vector<std::vector<ECE_ElectricField>> matrix;
        ECE_ElectricField final_E;
        std::cout << std::endl;

        // Task1 Max Concurrently Threads
        num_threads = concurrent_Thread();
        if (num_threads !=1)
        {
            num_threads = num_threads - 1;
        }

        // Task 2 Query the User of N, M, q
        query_user_1(n, m, q, x_D, y_D);
        //Total number
        int total_Charge_Num = n * m;
        // Task Create 2D Matrix
        charge_Matrix(n, m, q, x_D, y_D, matrix);
        query_user_2(x, y, z, n, m, matrix);
        final_E.At(x, y, z);
               
        // Tell threads to start calcualtions
        int rounds = total_Charge_Num / num_threads;
        int rounds_assist = total_Charge_Num % num_threads;

        auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for
            for (int j = 0; j < num_threads; ++j)
            {
                // Allocate the work using id

                int id = omp_get_thread_num();

                // Do the calculation -- hard
                if (id < rounds_assist)
                {
                    //std::cout << "Working id: " << id << std::endl;
                    for (int k = 0; k < rounds + 1; k++)
                    {
                        int location = (rounds + 1) * id + k;
                        int n_new = location / m;
                        int m_new = location % m;
                        matrix[n_new][m_new].computeFieldAt(x, y, z);
                    }
                }
                else
                {
                    for (int k = 0; k < rounds; k++)
                    {
                        int location = (rounds + 1) * (rounds_assist)+rounds * (id - rounds_assist) + k;
                        int n_new = location / m;
                        int m_new = location % m;
                        matrix[n_new][m_new].computeFieldAt(x, y, z);
                    }
                }
            }

        auto end = std::chrono::high_resolution_clock::now();
        
        for (int k = 0; k < n; ++k)
        {
            for (int j = 0; j < m; ++j)
            {
                final_E.find_sum(matrix[k][j]);
            }
        }
        
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
