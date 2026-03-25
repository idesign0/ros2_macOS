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

#include <cassert>

#include <gz/plugin/Register.hh>

#include "integrators.hh"

namespace gz{
namespace plugin {
namespace examples {
namespace RungeKutta4 {

/// \brief Add two vectors
std::vector<double> add(const std::vector<double> &v1,
                        const std::vector<double> &v2)
{
  assert(v1.size() == v2.size());
  std::vector<double> result = v1;

  for(std::size_t i=0; i < v1.size(); ++i)
  {
    result[i] += v2[i];
  }

  return result;
}

/// \brief Multiply a vector by a scalar
std::vector<double> times(const double s,
                          const std::vector<double> &v)
{
  std::vector<double> result = v;
  for(double& c : result)
    c *= s;

  return result;
}

/// \brief RK4 implementation of a numerical integrator
class Integrator : public gz::plugin::examples::NumericalIntegrator
{
  // Documentation inherited
  public: void SetFunction(
    const std::function<Derivative(Time, const State&)> &_func) override
  {
    function = _func;
  }

  // Documentation inherited
  public: bool SetTimeStep(TimeStep _step) override
  {
    if(_step <= 0.0)
      return false;

    timeStep = _step;
    return true;
  }

  // Documentation inherited
  public: TimeStep GetTimeStep() const override
  {
    return timeStep;
  }

  // Documentation inherited
  public: State Integrate(Time _currentTime, const State &_state) const override
  {
    const Time tn = _currentTime;
    const TimeStep h = timeStep;
    const State &yn = _state;

    // TODO: If we ever start to leverage Eigen or another linear algebra
    // library besides gz-math, then clean up these expressions.
    const Derivative &k1 = function(tn, yn);
    const Derivative &k2 = function(tn + h/2.0, add(yn, times(h/2.0, k1)));
    const Derivative &k3 = function(tn + h/2.0, add(yn, times(h/2.0, k2)));
    const Derivative &k4 = function(tn + h, add(yn, times(h, k3)));

    return add(yn, times(h/6.0, add(k1, add(times(2.0, k2),
                                            add(times(2.0, k3), k4)))));
  }

  /// \brief The time step that will be used when integrating
  private: TimeStep timeStep;

  /// \brief The function that represents the system of ordinary differential
  /// equations.
  private: SystemODE function;

};

GZ_ADD_PLUGIN(Integrator, NumericalIntegrator)

}
}
}
}
