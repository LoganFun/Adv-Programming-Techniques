// Integration problem from the first homework assignment
#include <omp.h>
#include <chrono>
#include <iostream>
using namespace std;

int main_m()
{
    double start_time = omp_get_wtime();
    long ii;
    double dx = 0.0;
    double dAns = 0.0;
    double dSum = 0.0;
    long lNumSteps = 500'000'000;
    double dStep = 1.0 / (double)lNumSteps;

    for (ii = 0; ii < lNumSteps; ++ii)
    {
        dx = (ii + 0.5)*dStep;
        dSum += 4.0 / (1.0 + dx * dx);
    }

    dAns = dStep * dSum;

    cout << "The answer is: " << dAns << " in " << 1000.0*(omp_get_wtime()-start_time) << " millsec with main thread" << endl;
    
    system("pause");

    return 0;
}
