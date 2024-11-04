#include "common1.h"
#include <omp.h>

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    Matrix res{ISIZE, JSIZE};
    Init(res);
    puts("Последовательная версия 1д.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    Matrix a{ISIZE, JSIZE};
    Init(a);
    
    auto time1 = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        #pragma omp for schedule(static) collapse(2)
        for (int i = 0; i < ISIZE - 1; i++)
        {
            for (int j = 6; j < JSIZE; j++)
            {
                res[i][j] = sin(0.2*a[i+1][j-6]);
            }
        }
    }
    
    auto time2 = std::chrono::high_resolution_clock::now();

    printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
    if (DIFF_FILES)
    {
        Write("obj/par_mp_t1d.txt", res);

        const char* const cmd = "diff obj/seq_t1d.txt obj/par_mp_t1d.txt";
        printf("Сравнение результатов с последовательной версией.\n"
            "%s: %d\n", cmd, system(cmd));
    }
    
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///