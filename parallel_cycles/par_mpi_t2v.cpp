#include "common1.h"
#include <mpi.h>
#include <iostream>

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

#define EXEC_MPI(action)                              \
    {                                                 \
        int _mpi_err_code = action;                   \
        if (_mpi_err_code != MPI_SUCCESS)             \
        {                                             \
            std::cout                                 \
                << "MPI error. Function:\n"           \
                << "\t\"" #action "\"\n"              \
                << "\tin " __FILE__  "\n"             \
                << "\tat "                            \
                << std::to_string(__LINE__)           \
                << " line\n"                          \
                << "\texited with error code = "      \
                << _mpi_err_code                      \
                << std::endl;                         \
            std::exit(-1);                            \
        }                                             \
    }

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

int main(int argc, char **argv)
{
    int procRank = 0;
    int procsCount = 0;

    EXEC_MPI(MPI_Init(&argc, &argv));
    EXEC_MPI(MPI_Comm_size(MPI_COMM_WORLD, &procsCount));
    EXEC_MPI(MPI_Comm_rank(MPI_COMM_WORLD, &procRank));

    if (procRank == 0)
    {
        puts("OpenMPI. Задача 2в.");
        printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);
    }

    Matrix a{ISIZE, JSIZE};
    Init(a);
    MPI_Status status;

    auto time1 = std::chrono::high_resolution_clock::now();
    // требуется обеспечить измерение времени работы данного цикла

    int allSize = JSIZE - 2;

    int partSize = allSize / procsCount;
    int offset = allSize % procsCount;

    int firstPartSize = partSize + offset;
    int startJ = 0;
    int stopJ = 0;

    // Включительно.
    int firstStopJ = allSize - 1;
    // Включительно.
    int firstStartJ = allSize - firstPartSize;

    // start <= j <= stop.

    if (procRank == 0)
    {
        stopJ = firstStopJ;
        startJ = firstStartJ;
    }
    else
    {
        stopJ = firstStartJ - 1 - partSize * (procRank - 1);
        startJ  = firstStartJ - partSize * procRank;
    }

    printf("Proc rank = %d, start = %d, stop = %d.\n", procRank, startJ, stopJ);

    for (int i = 3; i < ISIZE; i++)
    {
        for (int j = startJ; j <= stopJ; j++)
        {
            a[i][j] = sin(3*a[i - 3][j + 2]);
        }

        double* ptr = a.GetPtr() + i * JSIZE;
        // обмен на границе.
        // 3 <- 2, 1 <- 0
        // 2 <- 1
        if (procRank % 2 == 0)
        {
            if (procRank != procsCount - 1)
                EXEC_MPI(MPI_Send(ptr + startJ, 2, MPI_DOUBLE, procRank + 1, 0, MPI_COMM_WORLD));

            if (procRank != 0)
                EXEC_MPI(MPI_Recv(ptr + stopJ + 1, 2, MPI_DOUBLE, procRank - 1, 0, MPI_COMM_WORLD, &status));
        }
        else
        {
            EXEC_MPI(MPI_Recv(ptr + stopJ + 1, 2, MPI_DOUBLE, procRank - 1, 0, MPI_COMM_WORLD, &status));

            if (procRank != procsCount - 1)
                EXEC_MPI(MPI_Send(ptr + startJ, 2, MPI_DOUBLE, procRank + 1, 0, MPI_COMM_WORLD));
        }
    }

    // Склеить данные в строку и послать одним сообщением.
    if (procRank == 0)
    {
        int size = partSize * (ISIZE - 3);
        double* buffer = (double*)calloc(size, sizeof(double));
        for (int procRecvRank = 1; procRecvRank < procsCount; procRecvRank++)
        {
            stopJ = firstStartJ - 1 - partSize * (procRecvRank - 1);
            startJ  = firstStartJ - partSize * procRecvRank;
            EXEC_MPI(MPI_Recv(buffer, size, MPI_DOUBLE, procRecvRank, 0, MPI_COMM_WORLD, &status));

            int idx = 0;
            for (int i = 3; i < ISIZE; i++)
            {
                for (int j = startJ; j <= stopJ; j++)
                {
                    a[i][j] = buffer[idx++];
                }
            }
        }
        free(buffer);
    }
    else
    {
        int size = partSize * (ISIZE - 3);
        double* message = (double*)calloc(size, sizeof(double));
        int idx = 0;
        for (int i = 3; i < ISIZE; i++)
        {
            for (int j = startJ; j <= stopJ; j++)
            {
                message[idx++] = a[i][j];
            }
        }
        EXEC_MPI(MPI_Send(message, size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD));
        free(message);
    }

    auto time2 = std::chrono::high_resolution_clock::now();

    // char buf[256] = "";
    // sprintf(buf, "obj/par_mpi_t1_%d.txt", procRank);
    // Write(buf, a);

    if (procRank == 0)
    {
        printf("Execution time = %lf\n", std::chrono::duration<double>(time2 - time1).count());
        if (DIFF_FILES)
        {
            Write("obj/par_mpi_t2v.txt", a);

            const char* const cmd = "diff obj/seq_t2v.txt obj/par_mpi_t2v.txt";
            printf("Сравнение результатов с последовательной версией.\n"
                "%s: %d\n", cmd, system(cmd));
        }
    }


    EXEC_MPI(MPI_Finalize());
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///