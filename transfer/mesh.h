#pragma once

#include <cstddef>
#include <sstream>
#include <memory>

enum class Time
{
    Prev = 0,
    Curr = 1
};

enum class Direction
{
    Fwd,
    Inv
};

class Mesh
{
public:
    enum class MeshType
    {
        FirstIsPrev = 0,
        FirstIsCurr = 1
    };

    struct MeshCell
    {
        double x;
        double First;
        double Second;

        double& GetValue(Time time, MeshType type);
    };

    const size_t MeshSize;

private:
    MeshType  Type;
    std::unique_ptr<MeshCell[]> CellsArray;
    MeshCell* const StartBoundary;
    MeshCell* const InnerCells;
    MeshCell* const StopBoundary;

public:
    Mesh(size_t meshSize, double xLeft);

    double GetValue(size_t xIndex, Time time) const;

    void SetValue(size_t xIndex, Time time, double value);

    double GetStartBoundaryValue(Time time) const;

    void SetStartBoundaryValue(Time time, double value);

    double GetStopBoundaryValue(Time time) const;

    void SetStopBoundaryValue(Time time, double value);

    void NextTimeStep();

    const MeshCell* GetInnerCells() const;
    const MeshCell* GetStartCell() const;
    const MeshCell* GetStopCell() const;

    std::stringstream Print(Time time) const;

    MeshType GetType() const
    {
        return Type;
    }
};