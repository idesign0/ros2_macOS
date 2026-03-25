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

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cassert>
#include <cmath>
#include <set>

#include <gz/plugin/Loader.hh>
#include <gz/common/SystemPaths.hh>

#include "plugins/integrators.hh"

#ifdef HAVE_BOOST_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
namespace bpo = boost::program_options;
#endif

using NumericalIntegrator = gz::plugin::examples::NumericalIntegrator;
using ODESystem = gz::plugin::examples::ODESystem;
using ODESystemFactory = gz::plugin::examples::ODESystemFactory;

// The macro that this uses is provided as a compile definition in the
// examples/CMakeLists.txt file.
const std::string PluginLibDir = GZ_PLUGIN_EXAMPLES_LIBDIR;

/// \brief Return structure for numerical integration test results. If the name
/// is blank, that means the test was not run.
struct TestResult
{
  /// \brief Name of the test run
  public: std::string name;

  /// \brief Name of the actual time spent computing
  public: long timeSpent_us;

  /// \brief The percent error in each component of the state when compared to
  /// an exact solution.
  public: std::vector<double> percentError;
};

/// \brief A simple container to hold a plugin and the name that it was
/// instantiated from.
struct PluginHolder
{
  std::string name;
  gz::plugin::PluginPtr plugin;
};

/// \brief Compute the component-wise percent error of the estimate produced by
/// a numerical integrator, compared to the theoretical exact solution of the
/// system.
std::vector<double> ComputeError(
    const NumericalIntegrator::State &_estimate,
    const NumericalIntegrator::State &_exact)
{
  assert(_estimate.size() == _exact.size());
  std::vector<double> result(_estimate.size());

  for(std::size_t i = 0 ; i < _estimate.size(); ++i)
    result[i] = (_estimate[i] - _exact[i])/_exact[i] * 100.0;

  return result;
}

/// \brief Pass in a plugin that provides the numerical integration interface.
/// The numerical integration results of the plugin will be tested against the
/// results of the _exactFunction.
TestResult TestIntegrator(
    const PluginHolder &_pluginHolder,
    const ODESystem &_system,
    const double _timeStep,
    const unsigned int _numSteps)
{
  const gz::plugin::PluginPtr &plugin = _pluginHolder.plugin;
  NumericalIntegrator* integrator =
      plugin->QueryInterface<NumericalIntegrator>();

  if(!integrator)
  {
    std::cout << "The plugin named [" << _pluginHolder.name << "] does not "
              << "provide a NumericalIntegrator interface. It will not be "
              << "tested." << std::endl;
    return TestResult();
  }

  TestResult result;
  result.name = _pluginHolder.name;

  integrator->SetFunction(_system.ode);
  integrator->SetTimeStep(_timeStep);

  double time = _system.initialTime;
  NumericalIntegrator::State state = _system.initialState;

  auto performanceStart = std::chrono::high_resolution_clock::now();
  for(std::size_t i=0; i < _numSteps; ++i)
  {
    state = integrator->Integrate(time, state);
    time += integrator->GetTimeStep();
  }
  auto performanceStop = std::chrono::high_resolution_clock::now();

  result.timeSpent_us = std::chrono::duration_cast<std::chrono::microseconds>(
        performanceStop - performanceStart).count();

  result.percentError = ComputeError(state, _system.exact(time));

  return result;
}

/// \brief Print out the result of the test.
void PrintResult(const TestResult &_result)
{
  if (_result.name.empty())
  {
    // An empty name means that something went wrong with the test.
    return;
  }

  std::cout << "\nMethod: " << _result.name << "\n";

  std::cout << "Runtime(us): " << std::setfill(' ') << std::setw(8)
            << std::right << _result.timeSpent_us << "\n";

  std::cout << std::setw(0);

  std::cout << "Component-wise error: ";
  for (const double result : _result.percentError)
  {
    std::cout << std::setfill(' ') << std::setw(9) << std::right
              << std::setprecision(3) << std::fixed << result;

    std::cout << std::setw(0) << "%";
  }

  std::cout << "\n";
}

/// \brief Test a set of plugins against each system, using the specified
/// parameters.
void TestPlugins(
    const std::vector<PluginHolder> &_factories,
    const std::vector<PluginHolder> &_integrators,
    const double _timeStep,
    const unsigned int _numSteps)
{
  bool quit = false;
  if (_factories.empty())
  {
    std::cout << "You did not specify any ODE System plugins to test against!"
#ifdef HAVE_BOOST_PROGRAM_OPTIONS
              << "\n -- Pass in the -a flag to automatically use all plugins"
#endif
              << std::endl;
    quit = true;
  }

  if (_integrators.empty())
  {
    std::cout << "You did not specify any numerical integrator plugins to test "
              << "against!"
#ifdef HAVE_BOOST_PROGRAM_OPTIONS
              << "\n -- Pass in the -a flag to automatically use all plugins"
#endif
              << std::endl;
    quit = true;
  }

  if(quit)
    return;

  for (const PluginHolder &factory : _factories)
  {
    const std::vector<ODESystem> systems =
        factory.plugin->QueryInterface<ODESystemFactory>()->CreateSystems();

    for (const ODESystem &system : systems)
    {
      std::cout << "\n\n ================================================== \n";
      std::cout << "System [" << system.name << "] from factory ["
                << factory.name << "]\n";

      for (const PluginHolder &integrator : _integrators)
      {
        const TestResult result = TestIntegrator(
              integrator, system, _timeStep, _numSteps);

        PrintResult(result);
      }
    }
  }
}

/// \brief Load all the plugins that implement _interface.
std::vector<PluginHolder> LoadPlugins(
    const gz::plugin::Loader &_loader,
    const std::string &_interface)
{
  // Fill in the holders object with each plugin.
  std::vector<PluginHolder> holders;

  const auto pluginNames = _loader.PluginsImplementing(_interface);

  for (const std::string &name : pluginNames)
  {
    gz::plugin::PluginPtr plugin = _loader.Instantiate(name);
    if (!plugin)
    {
      std::cout << "Failed to load [" << name << "] as a class"
                << std::endl;
      continue;
    }

    holders.push_back({name, plugin});
  }

  return holders;
}

/// \brief Load all plugins that implement the NumericalIntegrator interface.
std::vector<PluginHolder> LoadIntegratorPlugins(
    const gz::plugin::Loader &_loader)
{
  return LoadPlugins(
        _loader, "gz::plugin::examples::NumericalIntegrator");
}

/// \brief Load all plugins that implement the ODESystemFactory interface
std::vector<PluginHolder> LoadSystemFactoryPlugins(
    const gz::plugin::Loader &_loader)
{
  return LoadPlugins(
        _loader, "gz::plugin::examples::ODESystemFactory");
}

/// \brief Prime the plugin loader with the paths and library names that it
/// should try to get plugins from.
void PrimeTheLoader(
    gz::common::SystemPaths &_paths, /* TODO: This should be const */
    gz::plugin::Loader &_loader,
    const std::set<std::string> &_pluginNames)
{
  for (const std::string &name : _pluginNames)
  {
    const std::string pluginPath = _paths.FindSharedLibrary(name);
    if (pluginPath.empty())
    {
      std::cout << "Failed to find path for plugin library [" << name << "]"
#ifdef HAVE_BOOST_PROGRAM_OPTIONS
              << "\n -- Use the -I flag to specify the plugin library directory"
#endif
                << std::endl;
      continue;
    }

    std::cout << "Path for [" << name << "] is [" << pluginPath << "]"
              << std::endl;

    if (_loader.LoadLib(pluginPath).empty())
    {
      std::cout << "Failed to load [" << name << "] as a plugin library"
                << std::endl;
    }
  }
}

int main(int argc, char *argv[])
{
  // Create an object that can search the system paths for the plugin libraries.
  gz::common::SystemPaths paths;

  // Create a plugin loader
  gz::plugin::Loader loader;

  // Add the build directory path for the plugin libraries so the SystemPaths
  // object will know to search through it.
  paths.AddPluginPaths(PluginLibDir);

  // Add the default plugins
  std::set<std::string> pluginNames = {
    "ForwardEuler", "RungeKutta4",
    "PolynomialODE", "ExponentialODE"
  };

#ifdef HAVE_BOOST_PROGRAM_OPTIONS

  double timeStep;
  unsigned int numSteps;

  std::string usage;
  usage +=
      "The 'integrators' example performs benchmark tests on numerical\n"
      "integrator plugins, testing them against differential equation plugins."
      "\nNumerical integrator plugins must inherit the NumericalIntegrator \n"
      "interface, and differential equation plugins must inherit the \n"
      "ODESystemFactory interface. Both interfaces can be found in the header\n"
      "gz-plugin/examples/plugins/Interfaces.hh.\n\n"

      "Custom plugins can be used by passing in the custom plugin library\n"
      "directory to the -I flag, and the library name(s) to the -p flag,\n"
      "as described below";

  bpo::options_description desc(usage);
  desc.add_options()

      ("help,h", "Print this usage message")

      ("plugins,p", bpo::value<std::vector<std::string>>(),
       "Plugins libraries to use")

      ("all,a", "Use all plugin libraries that come with this example")

      ("include-dirs,I", bpo::value<std::vector<std::string>>()->multitoken(),
       "Additional directories that may contain plugin libraries")

      ("timestep,s", bpo::value<double>(&timeStep)->default_value(0.01),
       "Size of the time step (s) to take")

      ("numsteps,n", bpo::value<unsigned int>(&numSteps)->default_value(10000),
       "Number of time steps to take")
      ;

  bpo::positional_options_description p;
  p.add("plugins", -1);

  bpo::variables_map vm;
  bpo::store(bpo::command_line_parser(argc, argv)
             .options(desc).positional(p).run(), vm);
  bpo::notify(vm);

  if (vm.count("help") > 0)
  {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("all") == 0)
  {
    pluginNames.clear();
  }

  if (vm.count("plugins"))
  {
    const std::vector<std::string> inputPlugins =
        vm["plugins"].as<std::vector<std::string>>();

    for (const std::string &input : inputPlugins)
      pluginNames.insert(input);
  }

  if (vm.count("include-dirs"))
  {
    const std::vector<std::string> inputDirs =
        vm["include-dirs"].as<std::vector<std::string>>();

    for (const std::string &input : inputDirs)
    {
      std::cout << "Including additional plugin directory: ["
                << input << "]" << std::endl;

      paths.AddPluginPaths(input);
    }
  }

#else

  const double timeStep = 0.01;
  const unsigned int numSteps = 10000;

  std::cout
      << "boost::program_options was not found when this example was\n"
      << "compiled, so we will default to using all the plugin libraries\n"
      << "that came with this example program. We will also default to:\n"
      << " -- time step = " << timeStep << "\n"
      << " -- num steps = " << numSteps << "\n"
      << std::endl;

#endif

  PrimeTheLoader(paths, loader, pluginNames);

  // Load the plugins
  const std::vector<PluginHolder> integrators = LoadIntegratorPlugins(loader);
  const std::vector<PluginHolder> systems = LoadSystemFactoryPlugins(loader);

  TestPlugins(systems, integrators, timeStep, numSteps);
}
