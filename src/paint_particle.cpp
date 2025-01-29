#include "paint_particle.hpp"

flo::paint_particle::paint_particle(double volume, const std::vector<double>& mixture) :
    volume_(volume), mixture_(mixture)
{
}

flo::paint_particle::paint_particle(double volume, int n) :
    volume_(volume), mixture_(n, 0.0)
{
}

double flo::paint_particle::volume() const
{
    return volume_;
}

const std::vector<double>& flo::paint_particle::mixture() const
{
    return mixture_;
}
