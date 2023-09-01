#include <cmath>

namespace random
{

void seed(unsigned int val)
{
    srand(val);
}

int Rnd()
{
    return rand();
}

double RndFloat()
{
    return float(Rnd()) / float(2147483647 - 2);
}

int Rnd(int zmin, int zmax)
{
    if (zmin <= zmax)
        return zmin + (Rnd() % (zmax - zmin + 1));
    else
        return zmax + (Rnd() % (zmin - zmax + 1));
}

double RndFloat(double zmin, double zmax)
{
    return zmin + RndFloat() * (zmax - zmin);
}

} // namespace random

double RND(int from, int to)
{
    return ((double)random::Rnd() * (1.0 / (RAND_MAX)) * (fabs(double((to) - (from)))) + (from));
}

float FRND(int x)
{
    return ((float)(RND(0, (x))));
}

float FSRND(int x)
{
    return (FRND(2.0f * (x)) - float(x));
}

int IRND(int n)
{
    return static_cast<int>(std::round(RND(0, double(n) - 0.55)));
}