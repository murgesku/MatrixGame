#pragma once

namespace random
{

void seed(unsigned int val);

int Rnd();
double RndFloat();
int Rnd(int zmin, int zmax);
double RndFloat(double zmin, double zmax);

} // namespace random

double RND(int from, int to);
float FRND(int x);
float FSRND(int x);
int IRND(int n);