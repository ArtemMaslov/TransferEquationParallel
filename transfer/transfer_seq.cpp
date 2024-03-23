#include <iostream>
#include <fstream>
#include <mpi.h>

#include "double.h"
#include "constant.h"
#include "domain.h"
#include "mesh.h"

int main()
{
    double startTime = MPI_Wtime();

    std::cout 
        << "Mesh:\n"
        << "\tX [0, " << X << "] m, step = h   = " << h << " m\n"
        << "\tT [0, " << T << "] s, step = tau = " << tau << " s\n"
        << "Velocity = " << a << " m/s\n"
        << "Courant number = " << Co << "\n"
        << std::endl;

    size_t meshSize = MeshXPoints;

    Direction direction = Direction::Inv;

    double x1 = 0;
    Domain domain{meshSize, x1, direction};
    
    double t = tau;

    bool printTimeSteps = false;

    if (printTimeSteps)
    {
        std::cout << "Boundary conditions" << std::endl;
        domain.Print(Time::Prev);
    }

    while (Double::IsLessEqual(t, T))
    {
        domain.ComputeStartBoundary4(t);

        domain.ComputeInnerCells4(t);
        domain.ComputeStopBoundary4(t);

        domain.SetTimeBoundary(t);
        domain.ApproximateTimeBoundary(t);

        if (printTimeSteps)
        {
            std::cout << "Time = " << t << std::endl;
            domain.Print(Time::Curr);
        }

        domain.NextTimeStep();
        t += tau;
    }

    auto fullMesh = std::make_unique<Mesh::MeshCell[]>(MeshXPoints);

    for (size_t st = 0; st < meshSize; st++)
        fullMesh.get()[st] = domain.GetMesh().GetInnerCells()[st];

    std::ofstream outFile;
    outFile << std::fixed;
    outFile.open("result.txt", std::ios::out);
    if (outFile.is_open())
    {
        outFile
            << "t x u\n";
        for (size_t st = 0; st < MeshXPoints; st++)
        {
            outFile 
                << " "
                << t - tau
                << " "
                << fullMesh.get()[st].x
                << " "
                << fullMesh.get()[st].GetValue(Time::Prev, domain.GetMesh().GetType()) << "\n";
        }
    }
    outFile.close();

    double stopTime = MPI_Wtime();

    std::cout << "Execution time = " << stopTime - startTime << " sec" << std::endl;

    return 0;
}