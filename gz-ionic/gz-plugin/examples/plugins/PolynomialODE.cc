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
namespace PolynomialODE {


/// \brief Create a simple parabola, like an object falling under the influence
/// of gravity.
ODESystem CreateParabolicODE(
    const double t0 = 0.0,
    const double p0 = 0.0,
    const double v0=1.0,
    const double a=2.0)
{
  ODESystem parabola;

  parabola.name = "parabolic ode";

  parabola.initialTime = t0;

  parabola.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return {0.5*a*std::pow(_t+t0, 2) + v0*(_t+t0) + p0};
  };

  parabola.initialState = parabola.exact(t0);

  parabola.ode = [=](const NumericalIntegrator::Time _t,
                     const NumericalIntegrator::State & /*_state*/)
      -> NumericalIntegrator::State
  {
    return {a*(_t+t0) + v0};
  };

  return parabola;
}

/// \brief Create a parabolic system of equations. Similar to the ParabolicODE,
/// except this is expressed as a system instead of a single function of time.
ODESystem CreateParabolicSystem(
    const double t0 = 0.0,
    const double p0 = 0.0,
    const double v0=1.0,
    const double a=2.0)
{
  ODESystem parabola;

  parabola.name = "parabolic system";

  parabola.initialTime = t0;

  parabola.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return { 0.5*a*std::pow(_t+t0, 2) + v0*(_t+t0) + p0,
                 a*std::pow(_t+t0, 1) + v0 };
  };

  parabola.initialState = parabola.exact(t0);

  parabola.ode = [=](const NumericalIntegrator::Time /*_t*/,
                     const NumericalIntegrator::State & _state)
      -> NumericalIntegrator::State
  {
    return {_state[1], a};
  };

  return parabola;
}

/// \brief Create a cubic trajectory, like a car whose level acceleration is
/// linearly increasing.
ODESystem CreateCubicODE(
    const double t0 = 0.0,
    const double p0 = 1.0,
    const double v0 = 2.0,
    const double a0 = 3.0,
    const double jerk = 4.0)
{
  ODESystem cubic;
  cubic.name = "cubic ode";

  cubic.initialTime = t0;

  cubic.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return {  1.0/6.0*jerk*std::pow(_t+t0, 3)
            +   1.0/2.0*a0*std::pow(_t+t0, 2)
            +           v0*std::pow(_t+t0, 1)
            +           p0*std::pow(_t+t0, 0) };
  };

  cubic.initialState = cubic.exact(t0);

  cubic.ode = [=](const NumericalIntegrator::Time _t,
                  const NumericalIntegrator::State & /*_state*/)
      -> NumericalIntegrator::State
  {
    return { 1.0/2.0*jerk*std::pow(_t+t0, 2)
            +          a0*std::pow(_t+t0, 1)
            +          v0*std::pow(_t+t0, 0) };
  };

  return cubic;
}

/// \brief Create a cubic system of equations. Similar to the CubicODE, except
/// this is expressed as a system instead of a single function of time.
ODESystem CreateCubicSystem(
    const double t0 = 0.0,
    const double p0 = 1.0,
    const double v0 = 2.0,
    const double a0 = 3.0,
    const double jerk = 4.0)
{
  ODESystem cubic;
  cubic.name = "cubic system";

  cubic.initialTime = t0;

  cubic.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return {  1.0/6.0*jerk*std::pow(_t+t0, 3)
            +   1.0/2.0*a0*std::pow(_t+t0, 2)
            +           v0*std::pow(_t+t0, 1)
            +           p0*std::pow(_t+t0, 0)
          ,
              1.0/2.0*jerk*std::pow(_t+t0, 2)
            +           a0*std::pow(_t+t0, 1)
            +           v0*std::pow(_t+t0, 0)
          ,
                      jerk*std::pow(_t+t0, 1)
            +           a0*std::pow(_t+t0, 0) };
  };

  cubic.initialState = cubic.exact(t0);

  cubic.ode = [=](const NumericalIntegrator::Time /*_t*/,
                  const NumericalIntegrator::State &_state)
      -> NumericalIntegrator::State
  {
    return { _state[1], _state[2], jerk };
  };

  return cubic;
}

/// \brief A factory that provides the parabolic ODE systems
class ParabolicFactory : public ODESystemFactory
{
  // Documentation inherited
  public: std::vector<ODESystem> CreateSystems() override
  {
    return {
      CreateParabolicODE(),
      CreateParabolicSystem()
    };
  }
};

/// \brief A factory that provides cubic ODE systems
class CubicFactory : public ODESystemFactory
{
  // Documentation inherited
  public: std::vector<ODESystem> CreateSystems() override
  {
    return {
      CreateCubicODE(),
      CreateCubicSystem()
    };
  }
};

}
}
}
}

// Register multiple plugins for this library
GZ_ADD_PLUGIN(
    gz::plugin::examples::PolynomialODE::ParabolicFactory,
    gz::plugin::examples::ODESystemFactory)

GZ_ADD_PLUGIN(
    gz::plugin::examples::PolynomialODE::CubicFactory,
    gz::plugin::examples::ODESystemFactory)

