/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <cmath>

#include <gz/plugin/Register.hh>

#include "integrators.hh"

namespace gz {
namespace plugin {
namespace examples {
namespace ExponentialODE {

/// \brief Create an exponential ODE, like radioactive decay or continuously
/// compounding interest.
ODESystem CreateExponentialODE(
    const double t0 = 0.0,
    const double x0 = 1.0,
    const double base = std::exp(1),
    const double lambda = 1.0)
{
  ODESystem exponential;
  exponential.name = "exponential ode";

  exponential.initialTime = t0;

  exponential.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return { x0 * std::pow(base, lambda*_t) };
  };

  exponential.initialState = exponential.exact(t0);

  exponential.ode = [=](const NumericalIntegrator::Time /*_t*/,
                        const NumericalIntegrator::State &_state)
      -> NumericalIntegrator::State
  {
    const double ln_base = std::log(base);
    return { x0*lambda * std::pow(_state[0], lambda/ln_base) * ln_base };
  };

  return exponential;
}

/// \brief Create an ODE system that represents a circular trajectory.
ODESystem CreateCircularSystem(
    const double t0 = 0.0,
    const double theta0 = 0.0,
    const double R = 1.0,
    const double w = 1.0)
{
  ODESystem circular;
  circular.name = "circular system";

  circular.initialTime = t0;

  circular.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return { R * cos(w * (_t-t0) + theta0), R * sin(w * (_t-t0) + theta0) };
  };

  circular.initialState = circular.exact(t0);

  circular.ode = [=](const NumericalIntegrator::Time _t,
                     const NumericalIntegrator::State &_state)
      -> NumericalIntegrator::State
  {
    return { -w * _state[1], w * _state[0] };
  };

  return circular;
}

/// \brief Create an ODE system that represents an exponential spiral trajectory
ODESystem CreateSpiralSystem(
    const double t0 = 0.0,
    const double theta0 = 0.0,
    const double R0 = 0.1,
    const double lambda = 0.5,
    const double w = 1.0)
{
  ODESystem spiral;
  spiral.name = "exponential spiral system";

  spiral.initialTime = t0;

  spiral.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return {
      R0 * std::exp(lambda*(_t-t0)) * cos(w*(_t-t0) + theta0),
      R0 * std::exp(lambda*(_t-t0)) * cos(w*(_t-t0) + theta0)
    };
  };

  spiral.initialState = spiral.exact(t0);

  spiral.ode = [=](const NumericalIntegrator::Time /*_t*/,
                   const NumericalIntegrator::State &_state)
      -> NumericalIntegrator::State
  {
    return { lambda * _state[0] - w * _state[1],
             lambda * _state[1] + w * _state[0] };
  };

  return spiral;
}

/// \brief A factory that provides these exponential ODE systems
class Factory : public ODESystemFactory
{
  public: std::vector<ODESystem> CreateSystems() override
  {
    return {
      CreateExponentialODE(),
      CreateCircularSystem(),
      CreateSpiralSystem()
    };
  }
};

GZ_ADD_PLUGIN(Factory, ODESystemFactory)

}
}
}
}
