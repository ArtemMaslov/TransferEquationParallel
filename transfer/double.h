#pragma once

class Double
{
public:
    static constexpr double Eps = 1e-9;

    static inline double Abs(double x);

    static inline bool IsEqual(double a, double b, double eps = Eps);

    static inline bool IsLessEqual(double a, double b, double eps = Eps);

    static inline bool IsMoreEqual(double a, double b, double eps = Eps);
};

double Double::Abs(double x)
{
    return (x > 0) ? x : -x;
}

bool Double::IsEqual(double a, double b, double eps)
{
    return Double::Abs(a - b) < eps;
}

bool Double::IsLessEqual(double a, double b, double eps)
{
    return a < b || Double::IsEqual(a, b, eps);
}

bool Double::IsMoreEqual(double a, double b, double eps)
{
    return a > b || Double::IsEqual(a, b, eps);
}