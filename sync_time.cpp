#include <iostream>
#include <mpi.h>

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

int main(int argc, char* argv[])
{
    int procRank = 0;
    int procsCount = 0;
    EXEC_MPI(MPI_Init(&argc, &argv));
    EXEC_MPI(MPI_Comm_size(MPI_COMM_WORLD, &procsCount));
    EXEC_MPI(MPI_Comm_rank(MPI_COMM_WORLD, &procRank));

    double sum = 0;
    size_t count = 1e5;

    double tick = MPI_Wtick();

    if (procRank == 0)
    {
        std::cout << "Wtick = " << tick << " sec" << std::endl;

        const size_t arrayCount = 10;
        double firstComm[arrayCount] = {};

        EXEC_MPI(MPI_Ssend(&tick, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD));

        double startTotal = MPI_Wtime();

        for (size_t st = 0; st < arrayCount; st++)
        {
            double start = MPI_Wtime();
            EXEC_MPI(MPI_Ssend(&tick, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD));
            double stop = MPI_Wtime();
            sum += stop - start;
            firstComm[st] = stop - start;
        }

        for (size_t st = 0; st < count - arrayCount; st++)
        {
            double start = MPI_Wtime();
            EXEC_MPI(MPI_Ssend(&tick, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD));
            double stop = MPI_Wtime();
            sum += stop - start;
        }

        double stopTotal = MPI_Wtime();
        double totalTime = stopTotal - startTotal;

        std::cout << "Communication times:\n";
        for (size_t st = 0; st < arrayCount; st++)
            std::cout << "\t" << st+1 << ". " << firstComm[st] << " sec\n"; 
        std::cout << std::endl;

        std::cout << "Average communication time = " << sum / count << " sec" << std::endl;
        std::cout << "Total time = " << totalTime << " sec" << std::endl;
        std::cout << "(total_time)/(communication_count) = " << totalTime / count << " sec" << std::endl;
    }
    else
    {
        double res = 0;
        MPI_Status status = {};
        EXEC_MPI(MPI_Recv(&res, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
        for (size_t st = 0; st < count; st++)
            EXEC_MPI(MPI_Recv(&res, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
    }

    EXEC_MPI(MPI_Finalize());
}