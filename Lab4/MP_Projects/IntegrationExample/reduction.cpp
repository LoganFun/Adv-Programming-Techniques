/*
OpenMP Reduction Clause

    reduction(op : list)

    A local copy of each list variable is made and 
    initialized depending on the operation for each thread.
    for example: "+" initialized to 0
    for example: "*" initialized to 1

    Updates occur on the local thread's copy

    At the end the local copies are reduced into a single 
    value and combined with the original global value
*/
#include <stdio.h>
const int MAX_SIZE = 100000;
int main_r()
{

    double ave = 0.0;
    double inputArray[MAX_SIZE];

    
    int ii;
#pragma omp parallel for reduction (+:ave)
    for (ii = 0; ii < MAX_SIZE; ++ii)
    {
        ave += inputArray[ii];
    }

    ave /= (double)MAX_SIZE;

}