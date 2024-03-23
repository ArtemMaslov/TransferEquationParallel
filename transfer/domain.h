#pragma once

#include <cstddef>

#include "mesh.h"

class Domain
{
private:
    double xLeft;
    ::Mesh Mesh;
    ::Direction Direction;

private:
    double GetXRight();

public:
    Domain(const size_t meshSize, const double x1, ::Direction direction);

    void ComputeStartBoundary3(double t);
    void ComputeStopBoundary3(double t);

    void ComputeStartBoundary4(double t);
    void ComputeStopBoundary4(double t);

    void ComputeStartBoundary2(double t);
    void ComputeStopBoundary2(double t);

    void ComputeInnerCells3(double t);
    void ComputeInnerCells4(double t);
    void ComputeInnerCells2(double t);

    void SetSpatialBoundary();

    void SetTimeBoundary(double t);
    void ApproximateTimeBoundary(double t);

    double GetStartInnerCell();
    double GetStopInnerCell();

    void SetStartBoundary(double value);
    void SetStopBoundary(double value);

    void NextTimeStep();

    const ::Mesh& GetMesh() const;

    void Print(Time time) const;
};