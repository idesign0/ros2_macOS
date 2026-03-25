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

#ifndef GZ_PLUGIN_EXAMPLES_PLUGINS_INTEGRATORS_HH_
#define GZ_PLUGIN_EXAMPLES_PLUGINS_INTEGRATORS_HH_

#include <vector>
#include <functional>

namespace gz
{
  namespace plugin
  {
    namespace examples
    {
      /// \brief The NumericalIntegrator class defines an interface for plugins
      /// that can perform numerical integration on an arbitrary system of
      /// ordinary differential equations.
      class NumericalIntegrator
      {
        // Define types for the input and the output of the ODE system.
        public: using State = std::vector<double>;
        public: using Derivative = std::vector<double>;
        public: using Time = double;
        public: using TimeStep = double;

        public: using SystemODE = std::function<Derivative(Time, const State&)>;
        public: using ExactSolution = std::function<State(Time)>;

        /// \brief Set the function that defines the system of ordinary
        /// differential equations that this integrator should operate on.
        /// \param[in] _func A std::function that defines a system of ordinary
        /// differential equations.
        public: virtual void SetFunction(const SystemODE &_func) = 0;

        /// \brief Set the amount of time that this integrator should step each
        /// time the Integrate() function is called.
        /// \param[in] The desired size of the step, in units of seconds.
        /// \return False if the integrator does not support the requested step
        /// size.
        public: virtual bool SetTimeStep(TimeStep _step) = 0;

        /// \brief Get the time step size that the integrator is currently set
        /// to use.
        /// \return Current time step size.
        public: virtual TimeStep GetTimeStep() const = 0;

        /// \brief Integrate the input state forward in time, and return it
        /// as output.
        /// \param[in] _currentTime The current simulation time.
        /// \param[in] _state The current state, corresponding to _currentTime
        /// \return The result of integrating _state forward in time. The time
        /// of the output is equal to _currentTime + GetTimeStep().
        public: virtual State Integrate(
          Time _currentTime, const State &_state) const = 0;

        /// \brief Virtual destructor
        public: virtual ~NumericalIntegrator() = default;
      };

      /// \brief A system of ordinary differential equations that each
      /// NumericalIntegrator implementation can be tested against.
      struct ODESystem
      {
        /// \brief Name of this system
        public: std::string name;

        /// \brief A system of Ordinary Differential Equations to test the
        /// numerical integrators.
        public: NumericalIntegrator::SystemODE ode;

        /// \brief The initial time of the system.
        public: NumericalIntegrator::Time initialTime;

        /// \brief The initial state of the system.
        public: NumericalIntegrator::State initialState;

        /// \brief A function that describes the exact solution of the ODE
        /// system that is being tested.
        public: NumericalIntegrator::ExactSolution exact;
      };

      /// \brief This interface allows us to create plugins that can provide
      /// systems of differential equations to test the numerical integrators
      /// against.
      class ODESystemFactory
      {
        /// \brief Create a set of systems of ordinary differential equations.
        public: virtual std::vector<ODESystem> CreateSystems() = 0;

        /// \brief Virtual destructor
        public: virtual ~ODESystemFactory() = default;
      };
    }
  }
}

#endif
