// Integration problem from the first homework assignment
#include <omp.h>
#include <iostream>

using namespace std;

int main_omp()
{
    const int iNUM_THREADS = 2;
    double start_time = omp_get_wtime();
    long ii = 0;
    long nThreads = 0;
    double dAns = 0.0;

    long lNumSteps = 500'000'000;
    double dStep = 1.0 / (double)lNumSteps;

    omp_set_num_threads(iNUM_THREADS);

#pragma omp parallel
    {

        double dSum = 0.0; // Local variable
        long ii;
        long iID = omp_get_thread_num();
        long nThrds = omp_get_num_threads();
        double dx;

        if (iID == 0) // Main thread
        {
            // Just because we requested 2 thread does not guarantee we
            // actually got that many
            nThreads = nThrds; 
        }
        // Find the most computationally intensive loops
        // Make sure there are no loop carry dependences (loops are completely independent)
        for (ii = iID; ii < lNumSteps; ii=ii+nThrds)
        {
            dx = (ii + 0.5)*dStep;
            dSum += 4.0 / (1.0 + dx * dx);
        }
#pragma omp atomic //critical
        dAns += dSum;
    }
    // Implied barrier here at the end of the threading section

    dAns *= dStep;

    cout << "The answer is: " << dAns << " in " << 1000.0*(omp_get_wtime() - start_time) << " millsec with Number threads = " << nThreads << endl;

    system("pause");

    return 0;
}