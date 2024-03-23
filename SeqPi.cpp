#include <iomanip>
#include <iostream>
#include <chrono>
#include <numbers>
#include <string>
#include <sstream>

static inline std::string DoubleToString(double number, std::streamsize prec = 36)
{
    std::stringstream pi_str;
    pi_str.precision(prec);
    pi_str 
        << std::showpos
        << std::fixed
        << number
        << std::noshowpos;
    return pi_str.str();
}

int main(int argc, char* argv[])
{
    // Compute pi using formula: \pi / 4 = \sum_{n=0}^{+\infty} \frac{ 2 }{ (4n+1)(4n+3) }
    auto progExecTimeStart = std::chrono::high_resolution_clock::now();
    
    if (argc != 2)
    {
        std::cout 
            << "Enter count of terms in series as a first argument. Exponential (10^9 = 1e9) notation is possible." 
            << std::endl;
        return -1;
    }

    size_t operationsCount = static_cast<size_t>(std::stod(argv[1]));
    double sum = 0;

    for (size_t n = 0; n < operationsCount; n++)
    {
        sum += 2.0 / static_cast<double>((4*n + 1) * (4*n + 3));
    }

    double pi = sum * 4;

    std::cout 
        << "Computed pi  = "
        << DoubleToString(pi)
        << "\n"

        << "Reference pi = "
        << DoubleToString(std::numbers::pi)
        << "\n"

        << "Difference   = "
        << DoubleToString(pi - std::numbers::pi)
        << "\n"

        << "Accuracy     = "
        << std::scientific
        << std::setprecision(2)
        << (pi - std::numbers::pi) / std::numbers::pi * 100.0
        << "%"
        << std::endl;

    std::cout
        << std::endl;

    auto progExecTimeStop = std::chrono::high_resolution_clock::now();

    std::cout
        << std::fixed
        << std::setprecision(6)
        << "Program execution time = "
        << std::chrono::duration<double>(progExecTimeStop - progExecTimeStart).count()
        << " seconds."
        << std::endl;

    return 0;
}