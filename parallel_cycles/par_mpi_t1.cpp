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
    puts("OpenMPI версия. Эталонная программа.");
    printf("Размер матрицы %d x %d\n", ISIZE, JSIZE);

    int procRank = 0;
    int procsCount = 0;

    EXEC_MPI(MPI_Init(&argc, &argv));
    EXEC_MPI(MPI_Comm_size(MPI_COMM_WORLD, &procsCount));
    EXEC_MPI(MPI_Comm_rank(MPI_COMM_WORLD, &procRank));

    Matrix a{ISIZE, JSIZE};
    Init(a);

    //auto time1 = std::chrono::high_resolution_clock::now();
    double time1 = MPI_Wtime();
    // требуется обеспечить измерение времени работы данного цикла

    int partSize = ISIZE / procsCount;
    int offset = ISIZE % procsCount;
    int startI = partSize * procRank + offset * (procRank != 0);
    int stopI = partSize * (procRank + 1) + offset;
    int thisPartSize = stopI - startI;

    for (int i = startI; i < stopI; i++)
    {
        for (int j = 0; j < JSIZE; j++)
        {
            a[i][j] = sin(2*a[i][j]);
        }
    }

    if (procRank == 0)
    {
        for (int procRecvRank = 1; procRecvRank < procsCount; procRecvRank++)
        {
            startI = partSize * procRecvRank + offset;
            stopI = partSize * (procRecvRank + 1) + offset;
            thisPartSize = stopI - startI;
            MPI_Status status;
            EXEC_MPI(MPI_Recv(a.GetPtr() + startI * JSIZE, thisPartSize * JSIZE, MPI_DOUBLE, procRecvRank, 0, MPI_COMM_WORLD, &status));
        }
    }
    else
    {
        EXEC_MPI(MPI_Send(a.GetPtr() + startI * JSIZE, thisPartSize * JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD));
    }

    double time2 = MPI_Wtime();
    //auto time2 = std::chrono::high_resolution_clock::now();

    // char buf[256] = "";
    // sprintf(buf, "obj/par_mpi_t1_%d.txt", procRank);
    // Write(buf, a);

    if (procRank == 0)
    {
        printf("Execution time = %lf\n", time2 - time1);
        if (DIFF_FILES)
        {
            Write("obj/par_mpi_t1.txt", a);

            const char* const cmd = "diff obj/seq_t1.txt obj/par_mpi_t1.txt";
            printf("Сравнение результатов с последовательной версией.\n"
                "%s: %d\n", cmd, system(cmd));
        }
    }


    EXEC_MPI(MPI_Finalize());
    return 0;
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///