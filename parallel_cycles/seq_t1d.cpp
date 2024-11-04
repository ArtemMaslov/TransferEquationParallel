#include "common1.h"

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    Matrix a{ISIZE, JSIZE};
    puts("Последовательная версия 1д.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    Init(a);

    auto time1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ISIZE - 1; i++)
    {
        for (int j = 6; j < JSIZE; j++)
        {
            a[i][j] = sin(0.2*a[i+1][j-6]);
        }
    }

    auto time2 = std::chrono::high_resolution_clock::now();

    printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
    if (DIFF_FILES)
    {
        Write("obj/seq_t1d.txt", a);
    }
    
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///