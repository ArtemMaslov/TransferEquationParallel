#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <chrono>

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

#ifndef ISIZE
    #define ISIZE 1000
#endif
#ifndef JSIZE
    #define JSIZE 1000
#endif

#ifndef DIFF_FILES
    #define DIFF_FILES false
#endif

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

class Row
{
public:
    Row(double* data, size_t size);

    double& operator [] (size_t columnIndex);

public:
    const size_t size = 0;

private:
    double* data;
};

class Matrix
{
public:
    Matrix(size_t rowsCount, size_t columnsCount);

    Row operator [] (size_t rowIndex);

    double* const GetPtr();

public:
    const size_t rowsCount = 0;
    const size_t columnsCount = 0;

private:
    std::unique_ptr<double[]> a;
};

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

Matrix::Matrix(size_t rowsCount, size_t columnsCount) :
    rowsCount(rowsCount),
    columnsCount(columnsCount),
    a(new double[rowsCount * columnsCount])
{
}

Row Matrix::operator [] (size_t rowIndex)
{
    assert(rowIndex < rowsCount);
    return Row{a.get() + rowIndex * columnsCount, columnsCount};
}

double* const Matrix::GetPtr()
{
    return a.get();
}

Row::Row(double* data, size_t size) :
    size(size),
    data(data)
{
}

double& Row::operator [] (size_t columnIndex)
{
    assert(columnIndex < size);
    return data[columnIndex];
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///

void Init(Matrix& a)
{
    for (int i = 0; i < ISIZE; i++)
    {
        for (int j = 0; j < JSIZE; j++)
        {
            a[i][j] = 10*i + j;
        }
    }
}

void Write(const char* const fileName, Matrix& a)
{
    FILE* ff = fopen(fileName, "w");
    assert(ff);    
    for(int i = 0; i < ISIZE; i++)
    {
        for (int j = 0; j < JSIZE; j++)
        {
            fprintf(ff, "%lf ", a[i][j]);
        }
        fprintf(ff, "\n");
    }
    fflush(ff);
    fclose(ff);
}

///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///***\\\***///