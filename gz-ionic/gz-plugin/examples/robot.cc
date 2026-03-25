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

#include <array>
#include <cassert>
#include <iostream>
#include <set>

#include "plugins/robot.hh"

#include <gz/common/SystemPaths.hh>
#include <gz/plugin/SpecializedPluginPtr.hh>
#include <gz/plugin/Loader.hh>

#ifdef HAVE_BOOST_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
namespace bpo = boost::program_options;
#endif

/////////////////////////////////////////////////
// The macro that this uses is provided as a compile definition in the
// examples/CMakeLists.txt file.
const std::string PluginLibDir = GZ_PLUGIN_EXAMPLES_LIBDIR;

/////////////////////////////////////////////////
using namespace gz::plugin::examples;

/////////////////////////////////////////////////
using RobotPluginPtr = gz::plugin::SpecializedPluginPtr<
    Drive, ProximitySensor, GPSSensor, Compass, MapDatabase>;

/////////////////////////////////////////////////
using EnvironmentPluginPtr =
    gz::plugin::SpecializedPluginPtr<Environment>;

using PointPair = std::pair<gz::math::Vector2d, gz::math::Vector2d>;

/////////////////////////////////////////////////
using IntersectionResult = std::pair<bool, gz::math::Vector2d>;

/////////////////////////////////////////////////
const IntersectionResult NoIntersection =
    std::make_pair(false, gz::math::Vector2d());

/////////////////////////////////////////////////
IntersectionResult CheckIntersection(
    const PointPair &_points,
    const Wall &_wall)
{
  std::array<double,2> A;
  std::array<double,2> B;
  std::array<double,2> C;

  std::size_t i = 0;
  for (const PointPair &p : {_points, _wall})
  {
    A[i] = p.second.Y() - p.first.Y();
    B[i] = p.second.X() - p.first.X();
    C[i] = A[i]*p.first.X() + B[i]*p.first.Y();
    ++i;
  }

  const double det = A[0]*B[1] - A[1]*B[0];
  if (std::abs(det) < 1e-10)
  {
    // Lines are parallel
    return NoIntersection;
  }

  const double x = (B[1]*C[0] - B[0]*C[1])/det;
  const double y = (A[0]*C[1] - A[1]*C[0])/det;

  for (const PointPair &p : {_points, _wall})
  {
    if (x < std::min(p.first.X(), p.second.X()))
      return NoIntersection;

    if (std::max(p.first.X(), p.second.X() ) < x)
      return NoIntersection;
  }

  return std::make_pair(true, gz::math::Vector2d(x,y));
}

/////////////////////////////////////////////////
IntersectionResult CheckCollisions(
    const gz::math::Vector2d &_x1,
    const gz::math::Vector2d &_x0,
    const Layout &_layout)
{
  PointPair points = std::make_pair(_x1, _x0);

  for (const Wall &wall : _layout)
  {
    const IntersectionResult result = CheckIntersection(points, wall);
    if (result.first)
      return result;
  }

  return NoIntersection;
}

/////////////////////////////////////////////////
void Simulate(const EnvironmentPluginPtr &_environment,
              const RobotPluginPtr &_robot,
              const double _duration,
              const bool _printState)
{
  Environment *env = _environment->QueryInterface<Environment>();
  assert(env && "Environment interface is missing!");

  const Drive *drive = _robot->QueryInterface<Drive>();
  assert(drive && "The robot cannot drive!");

  // Get the layout from the environment.
  const Layout &layout = env->GenerateLayout();

  // If the robot has a MapUplink interface, send it the layout of the
  // environment.
  if (MapDatabase *uplink = _robot->QueryInterface<MapDatabase>())
    uplink->ReadMap(layout);

  double time = 0.0;
  gz::math::Vector2d x;
  double theta = 0.0;

  while (time < _duration)
  {
    // Update the proximity sensor.
    if (ProximitySensor *proximity = _robot->QueryInterface<ProximitySensor>())
    {
      const double range = proximity->MaxRange();
      const gz::math::Vector2d x_far = x + gz::math::Vector2d(
            range*cos(theta), range*sin(theta));
      IntersectionResult result = CheckCollisions(x, x_far, layout);

      if (result.first)
      {
        proximity->ReadProximity( (x-result.second).Length() );
      }
      else
      {
        proximity->ReadProximity(std::numeric_limits<double>::infinity());
      }
    }

    // Update the GPS sensor
    if (GPSSensor *gps = _robot->QueryInterface<GPSSensor>())
    {
      gps->ReadGPS(x);
    }

    // Update the compass sensor
    if (Compass *compass = _robot->QueryInterface<Compass>())
    {
      compass->ReadCompass(theta);
    }

    // Save the last state
    gz::math::Vector2d x_last = x;

    // Get the driving information
    const double dt = 1.0/drive->Frequency();
    const gz::math::Vector3d &vel = drive->Velocity();

    // We integrate the position components with a half-step taken in the angle,
    // which should smooth out the behavior of sharp turns.
    const double halfAngle = theta + 0.5*vel[2]*dt;
    const double dx = vel[0]*cos(halfAngle)*dt + vel[1]*sin(halfAngle)*dt;
    const double dy = - vel[0]*sin(halfAngle)*dt + vel[1]*cos(halfAngle)*dt;

    x[0] += dx;
    x[1] += dy;
    theta += vel[2]*dt;

    const IntersectionResult collision = CheckCollisions(x, x_last, layout);
    if (collision.first)
    {
      std::cout << "The robot has crashed at ("
                << collision.second.X() << "m, "
                << collision.second.Y() << "m)! Time: "
                << time << "s  |==|  Velocity output: "
                << vel << "\n" << std::endl;

      x[0] = collision.second.X();
      x[1] = collision.second.Y();

      break;
    }

    time += dt;

    if (_printState)
    {
      std::cout << "Location: (" << x[0] << "m, " << x[1] << "m) | Yaw: "
                << 180.0*theta/GZ_PI << "-degrees | Time: " << time
                << "s  |==|  Velocity output: " << vel << std::endl;
    }
  }

  std::cout << "The simulation has finished (time: " << time << "s).\n"
            << " -- Final robot location: (" << x[0] << "m, " << x[1] << "m)\n"
            << " -- Yaw: " << 180.0*theta/GZ_PI << "-degrees\n" << std::endl;
}

/////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  // Create an object that can search the system paths for the plugin libraries.
  gz::common::SystemPaths paths;

  // Create a plugin loader
  gz::plugin::Loader loader;

  // Add the build directory path for the plugin libraries so the SystemPaths
  // object will know to search through it.
  paths.AddPluginPaths(PluginLibDir);

  std::vector<std::string> knownRobotPlugins = { "CrashBot", "CautiousBot" };
  std::vector<std::string> knownEnvPlugins = { "BoxEnvironment" };

  std::string robotLib = knownRobotPlugins.front();
  std::string envLib = knownEnvPlugins.front();
  double duration = 300;
  bool printState = false;

#ifdef HAVE_BOOST_PROGRAM_OPTIONS

  std::string knownRobotString;
  for (const std::string &r : knownRobotPlugins)
    knownRobotString += r + "  ";

  std::string knownEnvString;
  for (const std::string &e : knownEnvPlugins)
    knownEnvString += e + "  ";


  std::string usage;
  usage +=
      "The 'robot' example creates a very simple 2D kinematic simulation of a\n"
      "robot, provided by a plugin, within an environment, also provided by a\n"
      "plugin. The robot may have any combination of several sensors, defined\n"
      "in plugins/robot.hh.\n\n"

      "Known robot plugin libraries include:  " + knownRobotString + "\n\n"

      "Known environment plugin libraries include:  " + knownEnvString + "\n\n"

      "Custom plugins can be used by passing in the custom plugin library\n"
      "directory to the -I flag";

  bpo::options_description desc(usage);
  desc.add_options()

      ("help,h", "Print this usage message")

      ("robot,r", bpo::value<std::string>(&robotLib),
       std::string("Robot plugin library (default: "+robotLib+")").c_str())

      ("environment,e", bpo::value<std::string>(&envLib),
       std::string("Environment plugin library (default: "+envLib+")").c_str())

      ("include-dirs,I", bpo::value<std::vector<std::string>>()->multitoken(),
       "Additional directories that may contain plugin libraries")

      ("duration,t", bpo::value<double>(&duration),
       std::string("How many seconds of simulation time to run for (default: "
                   +std::to_string(duration)+")").c_str())

      ("print,p", "Print out the state during each iteration")
      ;

  bpo::variables_map vm;
  bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
  bpo::notify(vm);

  if (vm.count("help") > 0)
  {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("print") > 0)
  {
    printState = true;
  }

  if (vm.count("include-dirs"))
  {
    const std::vector<std::string> inputDirs =
        vm["include-dirs"].as<std::vector<std::string>>();

    for (const std::string &dir : inputDirs)
    {
      std::cout << "Including additional plugin directory: ["
                << dir << "]" << std::endl;
      paths.AddPluginPaths(dir);
    }
  }

#else

  std::cout << "boost::program_options was not found when this example was\n"
            << "compiled, so we will default to:\n"
            << " -- Robot:       " << robotLib <<"\n"
            << " -- Environment: " << envLib << "\n"
            << " -- Duration:    " << duration << "\n"
            << std::endl;

#endif

  std::unordered_set<std::string> robotPlugins = loader.LoadLib(
        paths.FindSharedLibrary(robotLib));

  std::unordered_set<std::string> drivePlugins =
      loader.PluginsImplementing("gz::plugin::examples::Drive");

  RobotPluginPtr robot;
  // Get the first plugin from the robotLib library that provides a Drive
  // interface
  for (const std::string &plugin : robotPlugins)
  {
    if (drivePlugins.find(plugin) != drivePlugins.end())
    {
      robot = loader.Instantiate(plugin);
      break;
    }
  }

  if (robot.IsEmpty())
  {
    std::cerr << "The plugin library specified for the robot does not provide "
              << "a Drive interface, which is required for the robot to work.\n"
              << std::endl;
    return 2;
  }

  std::unordered_set<std::string> envPlugins = loader.LoadLib(
        paths.FindSharedLibrary(envLib));

  std::unordered_set<std::string> envInterfacePlugins =
      loader.PluginsImplementing("gz::plugin::examples::Environment");

  EnvironmentPluginPtr environment;
  // Get the first plugin from the envLib library that provides an Environment
  // interface.
  for (const std::string &plugin : envPlugins)
  {
    if (envInterfacePlugins.find(plugin) != envInterfacePlugins.end())
    {
      environment = loader.Instantiate(plugin);
      break;
    }
  }

  if (environment.IsEmpty())
  {
    std::cerr << "The plugin library specified for the environment does not "
              << "provide an Environment interface, which is required for the "
              << "environment to be generated.\n" << std::endl;
    return 3;
  }

  Simulate(environment, robot, duration, printState);
}
