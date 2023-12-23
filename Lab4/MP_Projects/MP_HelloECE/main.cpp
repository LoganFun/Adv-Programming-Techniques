
#include <stdio.h>
#include <omp.h>
int main()
{
#pragma omp parallel // Creates parallel region
    {
        printf("Hello ECE 4122 6122\n");
    }

    return 0;
}

