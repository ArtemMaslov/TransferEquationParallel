#include <iostream>
#include <cassert>
#include <memory>
#include <sstream>

#include "constant.h"
#include "mesh.h"

Mesh::Mesh(size_t innerCellsCount, double xLeft, Direction dir) :
    MeshSize(innerCellsCount),
    Type(MeshType::FirstIsPrev),
    CellsArray(std::make_unique<MeshCell[]>(MeshSize + 2)),
    StartBoundary(CellsArray.get() + 0),
    InnerCells(StartBoundary + 1),
    StopBoundary(InnerCells + MeshSize)
{
    if (dir == Direction::Fwd)
    {
        StartBoundary->x = xLeft;
        double x = xLeft + h;
        for (size_t st = 0; st < innerCellsCount; st++)
        {
            InnerCells[st].x = x;
            x += h;
        }
        StopBoundary->x = x;
    }
    else
    {
        StopBoundary->x = xLeft;
        double x = xLeft + h;
        for (int st = innerCellsCount - 1; st >= 0; st--)
        {
            InnerCells[st].x = x;
            x += h;
        }
        StartBoundary->x = x;
    }
}

double& Mesh::MeshCell::GetValue(Time time, MeshType type)
{
    //  CellType |    MeshType     | Result | Sum % 2
    //  Prev = 0 | FirstIsPrev = 0 | First  |    0
    //  Prev = 0 | FirstIsCurr = 1 | Second |    1
    //  Curr = 1 | FirstIsPrev = 0 | Second |    1
    //  Curr = 1 | FirstIsCurr = 1 | First  |    0
    bool second = (static_cast<int>(time) + static_cast<int>(type)) % 2;
    if (second)
        return Second;
    else
        return First;
}

double Mesh::GetValue(size_t xIndex, Time time) const
{
    assert(xIndex < MeshSize);
    return InnerCells[xIndex].GetValue(time, Type);
}

void Mesh::SetValue(size_t xIndex, Time time, double value)
{
    assert(xIndex < MeshSize);
    InnerCells[xIndex].GetValue(time, Type) = value;
}

double Mesh::GetStartBoundaryValue(Time time) const
{
    return StartBoundary->GetValue(time, Type);
}

void Mesh::SetStartBoundaryValue(Time time, double value)
{
    StartBoundary->GetValue(time, Type) = value;
}

double Mesh::GetStopBoundaryValue(Time time) const
{
    return StopBoundary->GetValue(time, Type);
}

void Mesh::SetStopBoundaryValue(Time time, double value)
{
    StopBoundary->GetValue(time, Type) = value;
}

void Mesh::NextTimeStep()
{
    switch (Type)
    {
        case MeshType::FirstIsPrev:
            Type = MeshType::FirstIsCurr;
            break;
        
        case MeshType::FirstIsCurr:
            Type = MeshType::FirstIsPrev;
            break;
    }
}

const Mesh::MeshCell* Mesh::GetInnerCells() const
{
    return InnerCells;
}

const Mesh::MeshCell* Mesh::GetStartCell() const
{
    return StartBoundary;
}

const Mesh::MeshCell* Mesh::GetStopCell() const
{
    return StopBoundary;
}

static void Align(std::stringstream& str, size_t alignLen)
{
    for (size_t st = 0; st < alignLen; st++)
        str << " ";
    str << "|";
}

template <typename T1, typename T2, typename T3>
static void PrintColumn(std::stringstream& capStr,
                        std::stringstream& valueStr, 
                        std::stringstream& xStr, 
                        T1 capText, 
                        T2 value, 
                        T3 x)
{
    capStr 
        << " "
        << capText
        << " ";

    valueStr 
        << " "
        << value
        << " ";

    xStr
        << " "
        << x
        << " ";

    capStr.seekp(0, std::ios::end);
    size_t capLen = capStr.tellp();

    valueStr.seekp(0, std::ios::end);
    size_t valueLen = valueStr.tellp();

    xStr.seekp(0, std::ios::end);
    size_t xLen = xStr.tellp();

    size_t alignMax = std::max(capLen, std::max(valueLen, xLen));
    
    Align(capStr, alignMax - capLen);
    Align(valueStr, alignMax - valueLen);
    Align(xStr, alignMax - xLen);
}

std::stringstream Mesh::Print(Direction dir, Time time) const
{
    std::stringstream cap;
    std::stringstream value;
    std::stringstream x;

    value.precision(4);
    value << std::fixed;
    x.precision(4);
    x << std::fixed;

    PrintColumn(cap, value, x, "", "value", "x");
        
    if (dir == Direction::Fwd)
    {
        PrintColumn(cap, value, x, "LB", StartBoundary->GetValue(time, Type), StartBoundary->x);
        for (size_t st = 0; st < MeshSize; st++)
            PrintColumn(cap, value, x, "", InnerCells[st].GetValue(time, Type), InnerCells[st].x);
        PrintColumn(cap, value, x, "RB", StopBoundary->GetValue(time, Type), StopBoundary->x);
    }
    else
    {
        PrintColumn(cap, value, x, "LB", StopBoundary->GetValue(time, Type), StopBoundary->x);
        for (int st = MeshSize - 1; st >= 0; st--)
            PrintColumn(cap, value, x, "", InnerCells[st].GetValue(time, Type), InnerCells[st].x);
        PrintColumn(cap, value, x, "RB", StartBoundary->GetValue(time, Type), StartBoundary->x);
    }

    if (time == Time::Curr)
        cap << "Time == current\n";
    else
        cap << "Time == previous\n";

    cap << "\n" << x.str() << "\n" << value.str() << "\n" << std::endl;
    return cap;
}