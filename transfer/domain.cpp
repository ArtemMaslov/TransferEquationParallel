#include "constant.h"
#include "functions.h"
#include "domain.h"

Domain::Domain(const size_t meshSize, const double x1, ::Direction direction) :
    xLeft(x1),
    Mesh(meshSize, xLeft),
    Direction(direction)
{
    SetSpatialBoundary();
}

double Domain::GetXRight()
{
    return xLeft + h * (Mesh.MeshSize + 1);
}

void Domain::ComputeStartBoundary(double t)
{
    double u_k_mm1 = Mesh.GetStartBoundaryValue(Time::Prev);
    double u_k_m   = Mesh.GetValue(0, Time::Prev);
    double u_k_mp1 = Mesh.GetValue(1, Time::Prev);
    double f_k_m   = ComputeGeneratorFunction(xLeft + h, t - tau);
    
    double value = ComputeCellCentral4Points(u_k_mm1, u_k_m, u_k_mp1, f_k_m);
    Mesh.SetValue(0, Time::Curr, value);
}

void Domain::ComputeStopBoundary(double t)
{
    double u_k_mm1 = Mesh.GetValue(Mesh.MeshSize - 2, Time::Prev);
    double u_k_m   = Mesh.GetValue(Mesh.MeshSize - 1, Time::Prev);
    double u_k_mp1 = Mesh.GetStopBoundaryValue(Time::Prev);
    double f_k_m   = ComputeGeneratorFunction(GetXRight() - h, t - tau);
    
    double value = ComputeCellCentral4Points(u_k_mm1, u_k_m, u_k_mp1, f_k_m);
    Mesh.SetValue(Mesh.MeshSize - 1, Time::Curr, value);
}

void Domain::ComputeInnerCells(double t)
{
    if (Direction == Direction::Fwd)
    {
        double x = xLeft + 2 * h;
        for (size_t st = 1; st < Mesh.MeshSize - 1; st++)
        {
            double u_k_mm1 = Mesh.GetValue(st - 1, Time::Prev);
            double u_k_m   = Mesh.GetValue(st,     Time::Prev);
            double u_k_mp1 = Mesh.GetValue(st + 1, Time::Prev);
            double f_k_m   = ComputeGeneratorFunction(x, t - tau);
            double value   = ComputeCellCentral4Points(u_k_mm1, u_k_m, u_k_mp1, f_k_m);
            Mesh.SetValue(st, Time::Curr, value);
            x += h;
        }
    }
    else
    {
        double x = GetXRight() - 2 * h;
        for (int st = Mesh.MeshSize - 2; st >= 1; st--)
        {
            double u_k_mm1 = Mesh.GetValue(st - 1, Time::Prev);
            double u_k_m   = Mesh.GetValue(st,     Time::Prev);
            double u_k_mp1 = Mesh.GetValue(st + 1, Time::Prev);
            double f_k_m   = ComputeGeneratorFunction(x, t - tau);
            double value   = ComputeCellCentral4Points(u_k_mm1, u_k_m, u_k_mp1, f_k_m);
            Mesh.SetValue(st, Time::Curr, value);
            x -= h;
        }
    }
}

void Domain::SetSpatialBoundary()
{
    double value = ComputeSpatialBoundary(xLeft);
    Mesh.SetStartBoundaryValue(Time::Prev, value);
    double x = xLeft + h;
    for (size_t st = 0; st < Mesh.MeshSize; st++)
    {
        value = ComputeSpatialBoundary(x);
        Mesh.SetValue(st, Time::Prev, value);
        x += h;
    }
    value = ComputeSpatialBoundary(x);
    Mesh.SetStopBoundaryValue(Time::Prev, value);
}

void Domain::SetTimeBoundary(double t)
{
    double value = ComputeTimeBoundary(t);
    Mesh.SetStartBoundaryValue(Time::Curr, value);
}

void Domain::ApproximateTimeBoundary(double t)
{
    double u_k_mm1 = Mesh.GetValue(Mesh.MeshSize - 1, Time::Prev);
    double u_k_m   = Mesh.GetStopBoundaryValue(Time::Prev);
    double f_k_m   = ComputeGeneratorFunction(GetXRight(), t - tau);
    double value   = ComputeCellLeftCorner(u_k_mm1, u_k_m, f_k_m);
    Mesh.SetStopBoundaryValue(Time::Curr, value);
}

double Domain::GetStartInnerCell()
{
    return Mesh.GetValue(0, Time::Curr);
}

double Domain::GetStopInnerCell()
{
    return Mesh.GetValue(Mesh.MeshSize - 1, Time::Curr);
}

void Domain::SetStartBoundary(double value)
{
    Mesh.SetStartBoundaryValue(Time::Curr, value);
}

void Domain::SetStopBoundary(double value)
{
    Mesh.SetStopBoundaryValue(Time::Curr, value);
}

void Domain::NextTimeStep()
{
    Mesh.NextTimeStep();
}

const ::Mesh& Domain::GetMesh() const
{
    return Mesh;
}

std::stringstream Domain::Print(Time time) const
{
    return Mesh.Print(time);
}

const Mesh::MeshCell* Domain::GetLeftCell() const
{
    return Mesh.GetStartCell();
}

const Mesh::MeshCell* Domain::GetRightCell() const
{
    return Mesh.GetStopCell();
}