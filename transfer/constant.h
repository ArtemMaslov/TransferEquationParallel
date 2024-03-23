#pragma once

#include "cstddef"

// Constant boundary conditions.
// const double a   = 1; // m/sec

// const double T   = 10; // sec
// const double X   = 1; // m
// const size_t MeshXIntervals = 1e3;
// const size_t MeshXPoints = MeshXIntervals - 1;
// const size_t MeshTPoints = 1e5;
// const double tau = T / MeshTPoints;
// const double h   = X / MeshXIntervals;
// const double Co  = a * tau / h;
// const double DomainSize = MeshXPoints * MeshTPoints;

// Harmonic boundary conditions.
const double a = 1; // m/sec
const double Amplitude = 1;

const double PeriodT = 1; // sec
const double PeriodX = a*PeriodT; // m

const double T = PeriodT * 1;
const double X = PeriodX * 3; // m

const size_t MeshXIntervals = 0.5e4;
const size_t MeshXPoints = MeshXIntervals - 1;
const size_t MeshTPoints = 0.3e4;
const double tau = T / MeshTPoints;
const double h   = X / MeshXIntervals;
const double Co  = a * tau / h;
const double DomainSize = MeshXPoints * MeshTPoints;