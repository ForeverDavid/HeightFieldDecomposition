#ifndef TRI_CUBIC_INTERPOLATOR_H
#define TRI_CUBIC_INTERPOLATOR_H

#include <Eigen/Core>
#include "lib/common/arrays.h"
#include "lib/common/point.h"

namespace TricubicInterpolator {

    void getoCoefficients(Array4D<double>& coeffs, const Array3D<double> &weights);

    double getValue(const Pointd &p, const std::vector<double> coeffs);
}

#endif