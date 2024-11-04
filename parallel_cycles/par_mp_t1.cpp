#include "common1.h"
#include <omp.h>

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    Matrix a{ISIZE, JSIZE};
    puts("OpenMP версия. Эталонная программа.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    Init(a);

    auto time1 = std::chrono::high_resolution_clock::now();

    // требуется обеспечить измерение времени работы данного цикла

    #pragma omp parallel
    {
        #pragma omp for schedule(static) collapse(2)
        for (int i = 0; i < ISIZE; i++)
        {
            for (int j = 0; j < JSIZE; j++)
            {
                a[i][j] = sin(2*a[i][j]);
            }
        }
    }

    auto time2 = std::chrono::high_resolution_clock::now();

    printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
    
    if (DIFF_FILES)
    {
        Write("obj/par_mp_t1.txt", a);

        const char* const cmd = "diff obj/seq_t1.txt obj/par_mp_t1.txt";
        // diff пишет двоичные файлы различаются.
        // Если файлы равны, возвращает 0.
        printf("Сравнение результатов с последовательной версией.\n"
            "%s: %d\n", cmd, system(cmd));
    }
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///