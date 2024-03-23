#include <iostream>
#include <fstream>
#include <mpi.h>

#include "double.h"
#include "constant.h"
#include "domain.h"
#include "mesh.h"

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

const int SyncBoundary = 1;
const int SyncWrite    = 2;

int main(int argc, char* argv[])
{
    double startTime = MPI_Wtime();

    int procRank = 0;
    int procsCount = 0;
    EXEC_MPI(MPI_Init(&argc, &argv));
    EXEC_MPI(MPI_Comm_size(MPI_COMM_WORLD, &procsCount));
    EXEC_MPI(MPI_Comm_rank(MPI_COMM_WORLD, &procRank));

    if (procRank == 0)
    {
        std::cout 
            << "Mesh:\n"
            << "\tX [0, " << X << "] m, step = h   = " << h << " m\n"
            << "\tT [0, " << T << "] s, step = tau = " << tau << " s\n"
            << "Velocity = " << a << " m/s\n"
            << "Courant number = " << Co << "\n"
            << std::endl;
    }

    size_t meshSize = MeshXPoints / procsCount;
    if (procRank == procsCount - 1)
        meshSize += MeshXPoints - meshSize * procsCount;

    Direction direction = Direction::Fwd;
    if (procRank % 2 == 0)
        direction = Direction::Inv;

    // Synchronization plan:
    // --> = send
    // ->x = receive

    //  0 | 1 | 2 | 3 | 4 | 5
    // Inv Fwd Inv Fwd Inv Fwd

    // -->     -->     -->
    //    ->x      ->x     ->x
    //    <--      <--     <--
    // x<-     x<-     x<-
    
    //    -->      -->
    //         ->x     ->x
    //         <--     <--
    //    x<-      x<-

    double x1 = procRank * meshSize * h + h;
    Domain domain{meshSize, x1, direction};
    
    MPI_Request startRequest = {};
    MPI_Request stopRequest  = {};

    double startCellSender   = 0;
    double stopCellSender    = 0;

    double startCellReceiver = 0;
    double stopCellReceiver  = 0;

    double t = tau;

    std::cout << "Boundary conditions" << std::endl;
    domain.Print(Time::Prev);

    std::cout << "Current values" << std::endl;
    domain.Print(Time::Curr);

    while (Double::IsLessEqual(t, T))
    {
        domain.ComputeStartBoundary(t);

        if (procRank % 2 == 0 && procRank + 1 < procsCount)
        {
            startCellSender = domain.GetStartInnerCell();
            EXEC_MPI(MPI_Isend(&startCellSender, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, &startRequest));
        }

        domain.ComputeInnerCells(t);
        domain.ComputeStopBoundary(t);

        if (procRank == 0)
            domain.SetTimeBoundary(t);
        if (procRank == procsCount - 1)
            domain.ApproximateTimeBoundary(t);

        std::cout << "Time = " << t << std::endl;
        domain.Print(Time::Curr);

        if (procRank % 2 == 1)
        {
            EXEC_MPI(MPI_Recv(&startCellReceiver, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, nullptr));
            domain.SetStartBoundary(startCellReceiver);

            startCellSender = domain.GetStartInnerCell();
            EXEC_MPI(MPI_Isend(&startCellSender, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, &startRequest));

            if (procRank + 1 < procsCount)
            {
                stopCellSender = domain.GetStopInnerCell();
                EXEC_MPI(MPI_Isend(&stopCellSender, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, &stopRequest));

                EXEC_MPI(MPI_Recv(&stopCellReceiver, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, nullptr));
                domain.SetStopBoundary(stopCellReceiver);
            }

            EXEC_MPI(MPI_Wait(&startRequest, nullptr));
        }
        else
        {
            if (procRank + 1 < procsCount)
            {
                EXEC_MPI(MPI_Recv(&startCellReceiver, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, nullptr));
                domain.SetStartBoundary(startCellReceiver);
            }

            if (procRank - 1 > 0)
            {
                EXEC_MPI(MPI_Recv(&stopCellReceiver, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, nullptr));
                domain.SetStopBoundary(stopCellReceiver);

                stopCellSender = domain.GetStopInnerCell();
                EXEC_MPI(MPI_Ssend(&stopCellSender, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD));
            }
        }

        domain.NextTimeStep();
        t += tau;
    }

    if (procRank != 0)
    {
        std::unique_ptr<double[]> mesh = domain.GetMesh().GetInnerCells();
        EXEC_MPI(MPI_Send(mesh.get(), meshSize, MPI_DOUBLE, 0, SyncWrite, MPI_COMM_WORLD));
    }
    else
    {
        auto fullMesh = std::make_unique<double[]>(MeshXIntervals);

        for (size_t st = 0; st < meshSize; st++)
            fullMesh.get()[st] = domain.GetMesh().GetValue(st, Time::Prev);

        for (int st = 1; st < procsCount; st++)
            EXEC_MPI(MPI_Recv(fullMesh.get() + meshSize * st, meshSize, MPI_DOUBLE, st, SyncWrite, MPI_COMM_WORLD, nullptr));

        std::ofstream outFile;
        outFile.open("result.txt", std::ios::out);
        if (outFile.is_open())
        {
            for (size_t st = 0; st < MeshXIntervals; st++)
                outFile << fullMesh.get()[st] << "\n";
        }
        outFile.close();

        double stopTime = MPI_Wtime();

        std::cout << "Execution time = " << stopTime - startTime << " sec" << std::endl;
    }

    EXEC_MPI(MPI_Finalize());

    return 0;
}