#include <iostream>
#include <fstream>
#include <sstream>
#include <mpi.h>

#include "double.h"
#include "constant.h"
#include "domain.h"
#include "mesh.h"

const int SyncBoundary    = 1;
const int SyncWrite       = 2;
const int SyncEndBoundary = 3;

int main(int argc, char* argv[])
{
    double startTime = MPI_Wtime();

    int procRank = 0;
    int procsCount = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procsCount);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    MPI_File log;
    // delete file if it exist.
    MPI_File_open(MPI_COMM_WORLD, "log.txt", MPI_MODE_WRONLY | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &log);
    MPI_File_close(&log);
    log = {};
    MPI_File_open(MPI_COMM_WORLD, "log.txt", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &log);

    if (procRank == 0)
    {
        std::stringstream str;
        str << "Mesh:\n"
            << "\tX [0, " << X << "] m, step = h   = " << h << " m\n"
            << "\tT [0, " << T << "] s, step = tau = " << tau << " s\n"
            << "Velocity = " << a << " m/s\n"
            << "Courant number = " << Co << "\n"
            << "\n"
            << "Procs count = " << procsCount << "\n"
            << std::endl;
        
        MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    size_t indMeshSize = MeshXPoints / procsCount;
    size_t meshSize = indMeshSize;
    size_t lastMeshSize = meshSize +  MeshXPoints - meshSize * procsCount;
    if (procRank == procsCount - 1)
        meshSize = lastMeshSize;

    {
        std::stringstream str;
        str << "ProcRank = " << procRank << "\nMeshSize = " << meshSize << std::endl;
        MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
    }
    
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

    double x1 = procRank * indMeshSize * h;
    Domain domain{meshSize, x1, direction};
    
    MPI_Request startRequest = {};
    MPI_Request stopRequest  = {};

    double startCellSender   = 0;
    double stopCellSender    = 0;

    double startCellReceiver = 0;
    double stopCellReceiver  = 0;

    double t = tau;

    bool printTimeSteps = false;

    if (printTimeSteps)
    {
        std::stringstream str;
        str << "ProcRank = " << procRank << "\nBoundary conditions" << std::endl;
        str << domain.Print(Time::Prev).str();
        MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
    }

    while (Double::IsLessEqual(t, T))
    {
        domain.ComputeStartBoundary(t);

        if (procRank % 2 == 0 && procRank + 1 < procsCount)
        {
            startCellSender = domain.GetStartInnerCell();

            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nSend startCell\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }

            MPI_Isend(&startCellSender, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, &startRequest);

            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nStartCell sended. Continuing...\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }
        }

        domain.ComputeInnerCells(t);
        domain.ComputeStopBoundary(t);

        if (procRank == 0)
        {
            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nSet time boundary.\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }

            domain.SetTimeBoundary(t);
        }
        if (procRank == procsCount - 1)
        {
            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nApproximate time boundary.\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }

            domain.ApproximateTimeBoundary(t);
        }

        if (printTimeSteps)
        {
            std::stringstream str;
            str << "ProcRank = " << procRank << "\nTime = " << t << std::endl;
            str << domain.Print(Time::Curr).str();
            MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
        }

        if (procRank % 2 == 1)
        {
            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nRecv start cell.\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }

            MPI_Recv(&startCellReceiver, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, nullptr);
            domain.SetStartBoundary(startCellReceiver);

            if (printTimeSteps)
            {
                std::stringstream str;
                str << "ProcRank = " << procRank << "\nSend start cell.\n" << std::endl;
                MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
            }

            startCellSender = domain.GetStartInnerCell();
            MPI_Isend(&startCellSender, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, &startRequest);

            if (procRank + 1 < procsCount)
            {
                if (printTimeSteps)
                {
                    std::stringstream str;
                    str << "ProcRank = " << procRank << "\nSend stop cell.\n" << std::endl;
                    MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
                }

                stopCellSender = domain.GetStopInnerCell();
                MPI_Isend(&stopCellSender, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, &stopRequest);

                if (printTimeSteps)
                {
                    std::stringstream str;
                    str << "ProcRank = " << procRank << "\nRecv stop cell.\n" << std::endl;
                    MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
                }
                    
                MPI_Recv(&stopCellReceiver, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, nullptr);
                domain.SetStopBoundary(stopCellReceiver);
            }

            MPI_Wait(&startRequest, nullptr);
        }
        else
        {
            if (procRank + 1 < procsCount)
            {
                if (printTimeSteps)
                {
                    std::stringstream str;
                    str << "ProcRank = " << procRank << "\nRecv start cell.\n" << std::endl;
                    MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
                }
                    
                MPI_Recv(&startCellReceiver, 1, MPI_DOUBLE, procRank + 1, SyncBoundary, MPI_COMM_WORLD, nullptr);
                domain.SetStartBoundary(startCellReceiver);
            }

            if (procRank - 1 > 0)
            {
                if (printTimeSteps)
                {
                    std::stringstream str;
                    str << "ProcRank = " << procRank << "\nRecv stop cell.\n" << std::endl;
                    MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
                }
                    
                MPI_Recv(&stopCellReceiver, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD, nullptr);
                domain.SetStopBoundary(stopCellReceiver);

                if (printTimeSteps)
                {
                    std::stringstream str;
                    str << "ProcRank = " << procRank << "\nSend stop cell.\n" << std::endl;
                    MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
                }

                stopCellSender = domain.GetStopInnerCell();
                MPI_Ssend(&stopCellSender, 1, MPI_DOUBLE, procRank - 1, SyncBoundary, MPI_COMM_WORLD);
            }
        }

        domain.NextTimeStep();
        t += tau;
    }

    if (procRank != 0)
    {
        if (printTimeSteps)
        {
            std::stringstream str;
            str << "ProcRank = " << procRank << "\nSend results.\n" << std::endl;
            MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
        }
        
        MPI_Ssend(domain.GetMesh().GetInnerCells(), meshSize * sizeof(Mesh::MeshCell), MPI_CHAR, 0, SyncWrite, MPI_COMM_WORLD);

        if (procRank == procsCount - 1)
            MPI_Ssend(domain.GetRightCell(), sizeof(Mesh::MeshCell), MPI_CHAR, 0, SyncEndBoundary, MPI_COMM_WORLD);
    }
    else
    {
        auto fullMesh = std::make_unique<Mesh::MeshCell[]>(MeshXPoints * 2);
        fullMesh[0] = *domain.GetLeftCell();

        for (size_t st = 0; st < meshSize; st++)
            fullMesh.get()[st + 1] = domain.GetMesh().GetInnerCells()[meshSize - st - 1];

        if (printTimeSteps)
        {
            std::stringstream str;
            str << "ProcRank = " << procRank << "\nRecv results.\n" << std::endl;
            MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
        }

        for (int st = 1; st < procsCount - 1; st++)
        {
            MPI_Recv(fullMesh.get() + meshSize * st + 1, meshSize * sizeof(Mesh::MeshCell), MPI_CHAR, st, SyncWrite, MPI_COMM_WORLD, nullptr);
            if (st % 2 == 0)
            {
                Mesh::MeshCell* start = fullMesh.get() + meshSize * st + 1;
                Mesh::MeshCell* stop  = fullMesh.get() + meshSize * st + 1 + meshSize - 1;
                while (start < stop)
                {
                    Mesh::MeshCell tmp = *start;
                    *start = *stop;
                    *stop = tmp;
                    start++;
                    stop--;
                }
            }
        }
        if (procsCount - 1 > 0)
        {
            MPI_Recv(fullMesh.get() + 1 + meshSize * (procsCount - 1), lastMeshSize * sizeof(Mesh::MeshCell), MPI_CHAR, procsCount - 1, SyncWrite, MPI_COMM_WORLD, nullptr);
            if ((procsCount - 1) % 2 == 0)
            {
                Mesh::MeshCell* start = fullMesh.get() + 1 + meshSize * (procsCount - 1);
                Mesh::MeshCell* stop  = fullMesh.get() + 1 + meshSize * (procsCount - 1) + lastMeshSize - 1;
                while (start < stop)
                {
                    Mesh::MeshCell tmp = *start;
                    *start = *stop;
                    *stop = tmp;
                    start++;
                    stop--;
                }
            }

            Mesh::MeshCell RB = {};
            MPI_Recv(&RB, sizeof(Mesh::MeshCell), MPI_DOUBLE, procsCount - 1, SyncEndBoundary, MPI_COMM_WORLD, nullptr);
            fullMesh[MeshXPoints + 1] = RB;
        }
        else
            fullMesh[MeshXPoints + 1] = *domain.GetRightCell();

        std::ofstream outFile;
        outFile << std::fixed;
        outFile.open("result.txt", std::ios::out);
        if (outFile.is_open())
        {
            outFile
                << "t x u\n";
            for (size_t st = 0; st < MeshXPoints + 2; st++)
            {
                outFile 
                    << " "
                    << t - tau
                    << " "
                    << fullMesh.get()[st].x
                    << " "
                    << fullMesh.get()[st].GetValue(Time::Prev, domain.GetMesh().GetType()) << "\n";
            }
            outFile.close();
        }

        double stopTime = MPI_Wtime();

        {
            std::stringstream str;
            str << "Execution time = " << stopTime - startTime << " sec" << std::endl;
            MPI_File_write_shared(log, str.str().c_str(), str.tellp(), MPI_CHAR, MPI_STATUS_IGNORE);
        }
    }

    MPI_File_close(&log);
    MPI_Finalize();

    return 0;
}