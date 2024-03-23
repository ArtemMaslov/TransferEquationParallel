#include <cmath>

#include "constant.h"
#include "double.h"

double ComputeGeneratorFunction(__attribute__((unused)) double x, __attribute__((unused)) double t)
{
    // Constant boundary conditions.
    // return 0; // value/sec

    // Harmonic boundary conditions.
    return 0;
}

double ComputeTimeBoundary(__attribute__((unused)) double t)
{
    // Constant boundary conditions.
    // return 1; // value

    // Harmonic boundary conditions.
    return -Amplitude * sin(M_PI * 2 * t / PeriodT);
}

double ComputeSpatialBoundary(__attribute__((unused)) double x)
{
    // Constant boundary conditions.
    // if (Double::IsEqual(x, 0))
    //     return ComputeTimeBoundary(0); // value
    // else
    //     return 0;

    // Harmonic boundary conditions.
    return Amplitude * sin(M_PI * 2 * x / PeriodX);
}

double ComputeCellCentral3Points(double u_k_mm1, double u_k_mp1, double f_k_m)
{
    // 1/tau (u^{k+1}_m - 1/2 (u^k_{m+1} + u^k_{m-1})) + a 1/2h (u^k_{m+1} - u^k_{m-1}) = f^k_m.
    double value = (f_k_m - a * 1/(2*h) * (u_k_mp1 - u_k_mm1)) * tau + 1/2 * (u_k_mp1 + u_k_mm1);
    return value;
}

double ComputeCellCentral4Points(double u_k_mm1, double u_k_m, double u_k_mp1, double f_k_m)
{
    // 1/tau (u^{k+1}_m - u^k_{m}) + a 1/2h (u^k_{m+1} - u^k_{m-1}) - a^2 * tau /2h^2 (u^k_{m+1} - 2 u^k_m + u^k_{m-1}) = f^k_m.
    double value = (f_k_m - a/(2*h) * (u_k_mp1 - u_k_mm1) + a*a * tau/(2*h*h) * (u_k_mp1 - 2 * u_k_m + u_k_mm1)) * tau + u_k_m;
    return value;
}

double ComputeCellLeftCorner(double u_k_mm1, double u_k_m, double f_k_m)
{
    // 1/tau (u^{k+1}_m - u^k_m) + a/h (u^k_m - u^k_{m-1}) = f^k_m.
    return (f_k_m - a * 1/h * (u_k_m - u_k_mm1)) * tau + u_k_m;
}