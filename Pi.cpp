#include <iomanip>
#include <iostream>
#include <chrono>
#include <numbers>
#include <string>
#include <sstream>
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

static inline std::string DoubleToString(double number, std::streamsize prec = 36)
{
    std::stringstream pi_str;
    pi_str.precision(prec);
    pi_str 
        << std::showpos
        << std::fixed
        << number
        << std::noshowpos;
    return pi_str.str();
}

static const int ROOT_PROC_RANK  = 0;
static const int PARTIAL_SUM_TAG = 1;

int main(int argc, char* argv[])
{
    // Compute pi using formula: \pi / 4 = \sum_{n=0}^{+\infty} \frac{ 2 }{ (4n+1)(4n+3) }
    double progExecTimeStart = MPI_Wtime();

    int procRank = 0;
    int procsCount = 0;
    EXEC_MPI(MPI_Init(&argc, &argv));
    EXEC_MPI(MPI_Comm_size(MPI_COMM_WORLD, &procsCount));
    EXEC_MPI(MPI_Comm_rank(MPI_COMM_WORLD, &procRank));
    
    if (argc != 2)
    {
        std::cout 
            << "Enter count of terms in series as a first argument. Exponential (10^9 = 1e9) notation is possible." 
            << std::endl;
        return -1;
    }

    size_t opersCount = static_cast<size_t>(std::stod(argv[1]));

    size_t procOpersCount = opersCount / procsCount;
    double sum = 0;

    if (procRank == ROOT_PROC_RANK)
        procOpersCount += opersCount - procOpersCount * procsCount;

    for (size_t n = procRank * procOpersCount; n < (procRank + 1) * procOpersCount; n += 1)
    {
        sum += 2.0 / static_cast<double>((4*n + 1) * (4*n + 3));
    }

    // size_t procOpersCount = opersCount / procsCount;
    // double sum = 0;

    // if (procRank == ROOT_PROC_RANK)
    //     procOpersCount += opersCount - procOpersCount * procsCount;

    // for (size_t n = procRank; n < procOpersCount; n += procsCount)
    // {
    //     sum += 2.0 / static_cast<double>((4*n + 1) * (4*n + 3));
    // }

    if (procRank == ROOT_PROC_RANK)
    {
        for (int st = 0; st < procsCount - 1; st++)
        {
            double partialSum = 0;
            MPI_Status status;
            EXEC_MPI(MPI_Recv(&partialSum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status));
            sum += partialSum;
        }

        double pi = sum * 4;

        std::cout 
            << "Computed pi  = "
            << DoubleToString(pi)
            << "\n"

            << "Reference pi = "
            << DoubleToString(std::numbers::pi)
            << "\n"

            << "Difference   = "
            << DoubleToString(pi - std::numbers::pi)
            << "\n"

            << "Accuracy     = "
            << std::scientific
            << std::setprecision(2)
            << (pi - std::numbers::pi) / std::numbers::pi * 100.0
            << "%"
            << std::endl;

        double progExecTimeStop = MPI_Wtime();

        std::cout
            << std::endl;

        std::cout
            << std::fixed
            << std::setprecision(6)
            << "Program execution time = "
            << progExecTimeStop - progExecTimeStart
            << " seconds."
            << std::endl;
    }
    else
    {
        EXEC_MPI(MPI_Send(&sum, 1, MPI_DOUBLE, ROOT_PROC_RANK, PARTIAL_SUM_TAG, MPI_COMM_WORLD));
    }

    EXEC_MPI(MPI_Finalize());
    return 0;
}