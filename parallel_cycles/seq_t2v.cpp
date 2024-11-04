#include "common1.h"

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    Matrix a{ISIZE, JSIZE};
    puts("Последовательная версия 2в.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    Init(a);

    auto time1 = std::chrono::high_resolution_clock::now();

    for (int i = 3; i < ISIZE; i++)
    {
        for (int j = 0; j < JSIZE - 2; j++)
        {
            a[i][j] = sin(3*a[i - 3][j + 2]);
        }
    }

    auto time2 = std::chrono::high_resolution_clock::now();

    printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
    if (DIFF_FILES)
    {
        Write("obj/seq_t2v.txt", a);
    }
    
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///