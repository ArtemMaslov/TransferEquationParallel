#pragma once

double ComputeGeneratorFunction(double x, double t);

double ComputeTimeBoundary(double t);

double ComputeSpatialBoundary(double x);

double ComputeCellCentral4Points(double u_k_mm1, double u_k_m, double u_k_mp1, double f_k_m);

double ComputeCellCentral3Points(double u_k_mm1, double u_k_mp1, double f_k_m);

double ComputeCellLeftCorner(double u_k_mm1, double u_k_m, double f_k_m);