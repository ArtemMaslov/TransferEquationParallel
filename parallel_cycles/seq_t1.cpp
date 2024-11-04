#include "common1.h"

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    Matrix a{ISIZE, JSIZE};
    puts("Последовательная версия. Эталонная программа.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    Init(a);

    auto time1 = std::chrono::high_resolution_clock::now();
    // требуется обеспечить измерение времени работы данного цикла
    for (int i = 0; i < ISIZE; i++)
    {
        for (int j = 0; j < JSIZE; j++)
        {
            a[i][j] = sin(2*a[i][j]);
        }
    }
    auto time2 = std::chrono::high_resolution_clock::now();

    printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
    if (DIFF_FILES)
    {
        Write("obj/seq_t1.txt", a);
    }
    
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///