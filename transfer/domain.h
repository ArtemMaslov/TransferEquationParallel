#pragma once

#include <cstddef>
#include <sstream>
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

    void ComputeStartBoundary(double t);
    void ComputeStopBoundary(double t);

    void ComputeInnerCells(double t);

    void SetSpatialBoundary();

    void SetTimeBoundary(double t);
    void ApproximateTimeBoundary(double t);

    double GetStartInnerCell();
    double GetStopInnerCell();

    void SetStartBoundary(double value);
    void SetStopBoundary(double value);

    const Mesh::MeshCell* GetLeftCell() const;
    const Mesh::MeshCell* GetRightCell() const;

    void NextTimeStep();

    const ::Mesh& GetMesh() const;

    std::stringstream Print(Time time) const;
};