/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "Manager.hh"

#include <csignal> // NOLINT(*)
#include <fcntl.h>
#ifndef _WIN32
  #include <semaphore.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <unistd.h>
#else
  #include <process.h>
  /* Needed for std::min */
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <Processthreadsapi.h>
#endif
#include <signal.h>
#include <tinyxml2.h>

#include <condition_variable>
#include <ctime>
#include <limits>
#include <list>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/SignalHandler.hh>
#include <gz/common/SystemPaths.hh>
#include <gz/math/Rand.hh>
#include <gz/plugin/Loader.hh>

#include "gz/launch/config.hh"
#include "gz/launch/InstallationDirectories.hh"
#include "gz/launch/Plugin.hh"

#include "vendor/backward.hpp"

using namespace gz::launch;
using namespace std::chrono_literals;

#ifdef _WIN32
// Returns the last Win32 error, in string format. Returns an empty string if
// there is no error.
std::string GetLastErrorAsString()
{
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if(errorMessageID == 0) {
    // No error message has been recorded
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  // Ask Win32 to give us the string version of that message ID.
  // The parameters we pass in, tell Win32 to create the buffer that holds the
  // message for us (because we don't yet know how long the message string.
  // will be).
  size_t size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorMessageID,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&messageBuffer,
    0,
    NULL);

  // Copy the error message into a std::string.
  std::string message(messageBuffer, size);

  // Free the Win32's string's buffer.
  LocalFree(messageBuffer);

  return message;
}
#endif

/// \brief A class to encapsulate an executable (program) to run.
class Executable
{
  /// \brief Default constructor
  public: Executable() = default;

  /// \brief Constructs a executable object with a name, process id, command,
  /// and other options. This class is used primarily for internal
  /// book keeping.
  /// \param[in] _name Name of the executable. This is used for internal
  /// book keeping, it is not (necessarily) the name of the executable that
  /// will be run.
  /// \param[in] _pid The pid in which the executable is run.
  /// \param[in] _cmd The command and arguments that specify how to run the
  /// executable.
  /// \param[in] _autoRestart True if the executable should restart when it
  /// is is killed.
  /// \param[in] _envs Environment variables to set.
#ifndef _WIN32
  public: Executable(const std::string &_name, const pid_t _pid,
            const std::vector<std::string> &_cmd, bool _autoRestart,
            const std::list<std::string> &_envs)
          : name(_name), pid(_pid), command(_cmd), autoRestart(_autoRestart),
            envs(_envs)
          {}
#else
  public: Executable(const std::string &_name, const HANDLE _pi,
            const std::vector<std::string> &_cmd, bool _autoRestart,
            const std::list<std::string> &_envs)
          : name(_name), pi(_pi), command(_cmd), autoRestart(_autoRestart),
            envs(_envs)
          {}
#endif
  /// \brief Name of the executable
  public: std::string name = "";

  /// \brief Process id in which the executable is run.
#ifndef _WIN32
  public: pid_t pid = -1;
#else
  public: HANDLE pi;
#endif

  /// \brief The command to run.
  public: std::vector<std::string> command;

  /// \brief True will cause the command to restart on kill
  public: bool autoRestart = false;

  /// \brief Environment variables.
  public: std::list<std::string> envs;
};

/// \brief Private data variables for the Gazebo class.
class gz::launch::ManagerPrivate
{
  /// \brief Constructor.
  public: ManagerPrivate();

  /// \brief Destructor.
  public: ~ManagerPrivate();

  /// \brief Parse a configuration string.
  /// \param[in] _string XML configuration string.
  /// \return True on success.
  public: bool ParseConfig(const std::string &_string);

  /// \brief Fork a new process for a command specified by _exec. This
  /// function will create a new Executable and add it to the executables
  /// list.
  /// \param[in] _exec An executable definition to run.
  /// \return True on success.
  public: bool RunExecutable(const Executable &_exec);

  /// \brief Fork a new process for a command specific by _cmd.
  /// \param[in] _name A unique name given to the command. This name is
  /// used for book keeping and control of the process.
  /// \param[in] _cmd A vector of strings where the first string is
  /// expected to be the name of the executable to run, and subsequent
  /// strings are arguments.
  /// \param[in] _autoRestart True if the executable should auto restart on
  /// death.
  /// \param[in] _envs Environment variables to set.
  /// \return True on success.
  public: bool RunExecutable(const std::string &_name,
    const std::vector<std::string> &_cmd,
    const bool _autoRestart,
    const std::list<std::string> &_envs);

  /// \brief Stop all executables
  public: void ShutdownExecutables();

  /// \brief Stop running. This can be used to end a Run().
  /// \return True if running, False if the state was not running.
  public: bool Stop();

  /// \brief Load a plugin based on data contained in an XML element.
  /// \param[in] _elem Pointer to the XML element containing the plugin
  /// information.
  public: void LoadPlugin(const tinyxml2::XMLElement *_elem);

  /// \brief Handle SIG_INT and SIG_TERM signals
  /// \param[in] _sig The signal
  private: void OnSigIntTerm(int _sig);

  /// \brief Handle SIG_CHILD
  /// \param[in] _sig The signal
  private: static void OnSigChild(int _sig);

  /// \brief Start the worker thread to service stopped children.
  public: void StartWorkerThread();

  /// \brief Thread to handle restarting stopped children.
  private: void RestartLoop();

  /// \brief Parse executable configurations.
  /// \param[in] _elem XML element that contains an <executable>
  private: void ParseExecutables(const tinyxml2::XMLElement *_elem);

  /// \brief Parse <env> elements.
  /// \return List of environment variable name,value pairs.
  private: std::list<std::string> ParseEnvs(const tinyxml2::XMLElement *_elem);

  /// \brief Set environment variables.
  /// \param[in] _envs List of environment variable name,value pairs.
  private: void SetEnvs(const std::list<std::string> &_envs);

  /// \brief Parse executable wrappers. Executable wrappers allow a plugin
  /// to run in a process.
  /// \param[in] _elem  XML element that contains an <executable_wrapper>
  private: void ParseExecutableWrappers(const tinyxml2::XMLElement *_elem);

  /// \brief A list of executables that are running, or have been run.
  public: std::list<Executable> executables;

  /// \brief A list of children that were stopped to attempt restarts
#ifndef _WIN32
  public: std::queue<pid_t> stoppedChildren;
#else
  public: std::queue<HANDLE> stoppedChildren;
#endif
  /// \brief Semaphore to prevent restartThread from being a spinlock
#ifndef _WIN32
  public: sem_t *stoppedChildSem;
#else
  public: HANDLE stoppedChildSem;
  public: HANDLE pi;
#endif
  /// \brief Name of the semaphore created by stoppedChildSem.
  private: std::string stoppedChildSemName;

  /// \brief Thread containing the restart loop
  private: std::thread restartThread;

  /// \brief All the plugins
  public: std::unordered_set<launch::PluginPtr> plugins;

  /// \brief All the wrapped plugins
#ifndef _WIN32
  public: std::list<pid_t> wrappedPlugins;
#else
  public: std::list<PROCESS_INFORMATION> wrappedPlugins;
#endif
  /// \brief Mutex to protect the executables list.
  public: std::mutex executablesMutex;

  /// \brief A condition variable used by SIG_INT and Manager::Run.
  public: std::condition_variable runCondition;

  /// \brief A mutex used in conjunction with runCondition.
  public: std::mutex runMutex;

  /// \brief True if running.
  public: std::atomic<bool> running = false;

  /// \brief True indicates that this process is the master (main) process.
  public: bool master = false;

  /// \brief Our signal handler.
  public: std::unique_ptr<common::SignalHandler> sigHandler = nullptr;

  /// \brief Backward signal handler
  public: std::unique_ptr<backward::SignalHandling> backward = nullptr;

  /// \brief Top level environment variables.
  public: std::list<std::string> envs;

  /// \brief Pointer to myself. This is used in the signal handlers.
  /// A raw pointer is acceptable here since it is used only internally.
  public: static ManagerPrivate *myself;
};

// Init the static pointer.
ManagerPrivate *ManagerPrivate::myself = nullptr;

/////////////////////////////////////////////////
Manager::Manager()
  :dataPtr(new ManagerPrivate())
{
  this->dataPtr->myself = this->dataPtr.get();

  std::string homePath;
  gz::common::env(GZ_HOMEDIR, homePath);

  // Make sure to initialize logging.
  gzLogInit(gz::common::joinPaths(homePath, ".gz"), "launch.log");
  if (!this->dataPtr->sigHandler->Initialized())
    gzerr << "signal(2) failed while setting up for SIGINT" << std::endl;
}

/////////////////////////////////////////////////
Manager::~Manager()
{
  this->dataPtr->Stop();
}

/////////////////////////////////////////////////
bool Manager::RunConfig(const std::string &_config)
{
  std::unique_lock<std::mutex> lock(this->dataPtr->runMutex);

  // This is the master.
  this->dataPtr->master = true;

  // Parse the configuration string, and run all the specified commands.
  if (!this->dataPtr->ParseConfig(_config))
    return false;

  // Child processes should exit
  if (!this->dataPtr->master)
    return true;

  // Get whether or not we should run (block).
  this->dataPtr->running = !this->dataPtr->executables.empty() ||
                           !this->dataPtr->plugins.empty();

  // Start thread to service child signals.
  this->dataPtr->StartWorkerThread();

  // Wait for a shutdown event, or for all the executables to quit.
  while (this->dataPtr->running && (!this->dataPtr->executables.empty() ||
                                    !this->dataPtr->plugins.empty()))
  {
    this->dataPtr->runCondition.wait(lock);
  }
  this->dataPtr->running = false;

  // Stop plugins.
  this->dataPtr->plugins.clear();

  // Stop executables.
  this->dataPtr->ShutdownExecutables();

  return true;
}

/////////////////////////////////////////////////
bool Manager::Stop()
{
  return this->dataPtr->Stop();
}

/////////////////////////////////////////////////
ManagerPrivate::ManagerPrivate()
{
  // Register a normal signal handler
  this->sigHandler.reset(new common::SignalHandler());
  this->sigHandler->AddCallback(
      std::bind(&ManagerPrivate::OnSigIntTerm, this, std::placeholders::_1));

  {
    // The semaphore we initialize in the next section needs a unique name, so
    // we need to seed a random number generator with a quality random number.
    // Especially time(0) itself is not a good seed as there may be multiple
    // processes launched at the same time.
    const auto time_seed = static_cast<size_t>(std::time(nullptr));
    const auto pid_seed = std::hash<std::thread::id>()(
        std::this_thread::get_id());
    std::seed_seq seed_value{time_seed, pid_seed};
    std::vector<std::uint32_t> seeds(1);
    seed_value.generate(seeds.begin(), seeds.end());
    math::Rand::Seed(seeds[0]);
  }
  const auto semRandomId = math::Rand::IntUniform(0,
      std::numeric_limits<int32_t>::max());

  // Initialize semaphore
  this->stoppedChildSemName = std::string("gz-launch-") +
      std::to_string(semRandomId);
#ifndef _WIN32
  this->stoppedChildSem = sem_open(this->stoppedChildSemName.c_str(), O_CREAT,
      0644, 1);
  if (this->stoppedChildSem == SEM_FAILED)
  {
    gzerr << "Error initializing semaphore " << this->stoppedChildSemName
           << ": " << strerror(errno) << std::endl;
  }
#else
  this->stoppedChildSem = CreateSemaphoreA(
    NULL, 0, LONG_MAX, this->stoppedChildSemName.c_str());
  if (this->stoppedChildSem == nullptr)
  {
    gzerr << "Error initializing semaphore " << this->stoppedChildSemName
           << ": " << GetLastErrorAsString() << std::endl;
  }
#endif

#ifndef _WIN32
  // Register a signal handler to capture child process death events.
  if (signal(SIGCHLD, ManagerPrivate::OnSigChild) == SIG_ERR)
    gzerr << "signal(2) failed while setting up for SIGCHLD" << std::endl;

  // Register backward signal handler for other signals
  std::vector<int> signals =
  {
    SIGABRT,    // Abort signal from abort(3)
    SIGBUS,     // Bus error (bad memory access)
    SIGFPE,     // Floating point exception
    SIGILL,     // Illegal Instruction
    SIGIOT,     // IOT trap. A synonym for SIGABRT
    // SIGQUIT, // Quit from keyboard
    SIGSEGV,    // Invalid memory reference
    SIGSYS,     // Bad argument to routine (SVr4)
    SIGTRAP,    // Trace/breakpoint trap
    SIGXCPU,    // CPU time limit exceeded (4.2BSD)
    SIGXFSZ,    // File size limit exceeded (4.2BSD)
  };

  this->backward = std::make_unique<backward::SignalHandling>(signals);
#else

#endif
}

/////////////////////////////////////////////////
ManagerPrivate::~ManagerPrivate()
{
  if (this->master)
  {
#ifndef _WIN32
    if (sem_close(this->stoppedChildSem) == -1)
    {
      gzerr << "Failed to close semaphore " << this->stoppedChildSemName
             << ": " << strerror(errno) << std::endl;
    }

    if (sem_unlink(this->stoppedChildSemName.c_str()) == -1)
    {
      gzerr << "Failed to unlink semaphore " << this->stoppedChildSemName
             << ": " << strerror(errno) << std::endl;
    }
#else
    int retVal = CloseHandle(this->stoppedChildSem) ? 0 : -1;
    if (retVal == -1)
    {
      gzerr << "Failed to close semaphore: " << strerror(errno)
             << std::endl;
    }
#endif
  }
}

/////////////////////////////////////////////////
bool ManagerPrivate::Stop()
{
  if (this->runMutex.try_lock())
  {
    if (this->running)
    {
      this->running = false;
    }
    this->runMutex.unlock();
    this->runCondition.notify_all();

    // Signal the restart thread to stop
#ifndef _WIN32
    sem_post(this->stoppedChildSem);
#else
    // Wait until child process exits.
    int retVal = ReleaseSemaphore(this->stoppedChildSem, 1, nullptr);
    if (retVal != 0)
    {
      gzerr << "Error Releasing Semaphore "
             << GetLastErrorAsString() << std::endl;
    }
#endif
    if (this->restartThread.joinable())
      this->restartThread.join();
  }
  return this->running;
}

/////////////////////////////////////////////////
void ManagerPrivate::OnSigIntTerm(int _sig)
{
  gzdbg << "OnSigIntTerm Received signal[" << _sig  << "]\n";
  this->Stop();
}

/////////////////////////////////////////////////
void ManagerPrivate::OnSigChild(int _sig)
{
  // Retreive the stopped child's PID, append, and signal the consumer.
#ifndef _WIN32
  // This is a signal handler, so be careful not to do any operations
  // that you are not allowed to do.
  // Ref: http://man7.org/linux/man-pages/man7/signal-safety.7.html
  (void) _sig;  // Commenting _sig above confuses codecheck
  pid_t p;
  int status;

  if ((p = waitpid(-1, &status, WNOHANG)) != -1)
  {
    myself->stoppedChildren.push(p);
    sem_post(myself->stoppedChildSem);
  }
#endif
}

/////////////////////////////////////////////////
void ManagerPrivate::StartWorkerThread()
{
  // Spawn thread to monitor restarting children.
  this->restartThread = std::thread(&ManagerPrivate::RestartLoop, this);
}

/////////////////////////////////////////////////
void ManagerPrivate::RestartLoop()
{
#ifndef _WIN32
  sigset_t chldmask;

  if ((sigemptyset(&chldmask) == -1) || (sigaddset(&chldmask, SIGCHLD) == -1))
  {
    gzerr << "Failed to initialize signal mask: "
      << strerror(errno) << std::endl;
    return;
  }

  while (this->running)
  {
    // Wait for the signal handler to signal that a child has exited.
    int s = sem_wait(this->stoppedChildSem);
    if (s == -1)
    {
      if (errno != EINTR)
        gzwarn << "sem_wait error: "
          << strerror(errno) << std::endl;
      continue;
    }

    // Block SIGCHLD while consuming queue.
    if (sigprocmask(SIG_BLOCK, &chldmask, NULL) == -1)
    {
      gzerr << "Failed to setup block for SIGCHLD: "
        << strerror(errno) << std::endl;
      continue;
    }

    // Consume stopped children from the queue.
    while (!this->stoppedChildren.empty())
    {
      std::lock_guard<std::mutex> mutex(this->executablesMutex);

      // Executable to restart
      Executable restartExec;

      pid_t p = this->stoppedChildren.front();
      this->stoppedChildren.pop();

      // Find the executable
      for (std::list<Executable>::iterator iter = this->executables.begin();
           iter != this->executables.end(); ++iter)
      {
        if (iter->pid == p)
        {
          gzdbg << "Death of process[" << p << "] with name["
                 << iter->name << "].\n";

          // Restart if autoRestart is enabled
          if (iter->autoRestart)
            restartExec = *iter;

          this->executables.erase(iter);
          break;
        }
      }

      if (!restartExec.name.empty() && !restartExec.command.empty())
      {
        gzdbg << "Restarting process with name[" << restartExec.name << "]\n";
        this->RunExecutable(restartExec);
      }

      this->runCondition.notify_all();
    }

    // Unblock SIGCHLD
    if (sigprocmask(SIG_UNBLOCK, &chldmask, NULL) == -1)
    {
      gzerr << "Failed to unblock SIGCHLD: " << strerror(errno) << std::endl;
      continue;
    }
  }
#else
  // Create a vector of monitor threads that wait for each process to stop.
  std::vector<std::thread> monitors;
  for (const Executable &exec : this->executables)
    monitors.push_back(std::thread([&] {
      WaitForSingleObject(exec.pi, INFINITE);
      myself->stoppedChildren.push(exec.pi);
    }));

  while (this->running)
  {
    // Wait for the signal handler to signal that a child has exited.
    int s = WaitForSingleObject(this->stoppedChildSem, INFINITE);
    if (!this->running)
      break;

    // Consume stopped children from the queue.
    while (!this->stoppedChildren.empty())
    {
      std::lock_guard<std::mutex> mutex(this->executablesMutex);

      // Executable to restart
      Executable restartExec;

      HANDLE p = this->stoppedChildren.front();
      this->stoppedChildren.pop();

      // Find the executable
      for (std::list<Executable>::iterator iter = this->executables.begin();
           iter != this->executables.end(); ++iter)
      {
        if (iter->pi == p)
        {
          gzdbg << "Death of process[" << p << "] with name ["
                 << iter->name << "].\n";

          // Restart if autoRestart is enabled
          if (iter->autoRestart)
            restartExec = *iter;

          this->executables.erase(iter);
          break;
        }
      }

      if (!restartExec.name.empty() && !restartExec.command.empty())
      {
        gzdbg << "Restarting process with name[" << restartExec.name << "]\n";
        this->RunExecutable(restartExec);
      }

      this->runCondition.notify_all();
    }
  }
#endif
}

/////////////////////////////////////////////////
bool ManagerPrivate::ParseConfig(const std::string &_config)
{
  tinyxml2::XMLDocument xmlDoc;

  // Load the XML configuration file into TinyXML
  if (xmlDoc.Parse(_config.c_str()) != tinyxml2::XML_SUCCESS)
  {
    gzerr << "Unable to parse configuration. Your XML might be invalid.\n";
    return false;
  }

  // Get the root element.
  tinyxml2::XMLElement *root = xmlDoc.FirstChildElement("gz");
  if (!root)
  {
    gzerr << "Invalid config file, missing `<gz>` element\n";
    return false;
  }
  // Keep the environment variables in memory. See manpage for putenv.
  this->envs = this->ParseEnvs(root);
  this->SetEnvs(this->envs);

  // Parse and create all the <executable> elements.
  this->ParseExecutables(root);

  // Parse and create all the <executable_wrapper> elements.
  if (this->master)
    this->ParseExecutableWrappers(root);

  // Parse and create all the <plugin> elements.
  if (this->master)
  {
    // Process all the plugins.
    tinyxml2::XMLElement *pluginElem = root->FirstChildElement("plugin");
    while (pluginElem)
    {
      this->LoadPlugin(pluginElem);
      pluginElem = pluginElem->NextSiblingElement("plugin");
    }
  }

  return true;
}

/////////////////////////////////////////////////
bool ManagerPrivate::RunExecutable(const Executable &_exec)
{
  return this->RunExecutable(_exec.name, _exec.command, _exec.autoRestart,
      _exec.envs);
}

/////////////////////////////////////////////////
bool ManagerPrivate::RunExecutable(const std::string &_name,
    const std::vector<std::string> &_cmd, bool _autoRestart,
    const std::list<std::string> &_envs)
{
  // Check for empty
  if (_cmd.empty())
  {
    gzerr << "Empty command.\n";
    return false;
  }
#ifdef _WIN32
  typedef struct MyData {
      std::vector<std::string> _cmd;
      HANDLE stoppedChildSem;
  } MYDATA, *PMYDATA;

  SetEnvs(_envs);

  PMYDATA pDataArray = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(MYDATA));
  if (pDataArray == nullptr)
  {
    gzerr << "allocation fails " << GetLastErrorAsString() << '\n';
    return false;
  }

  for (auto & cmd : _cmd){
    pDataArray->_cmd.push_back(cmd);
  }

  pDataArray->stoppedChildSem = this->stoppedChildSem;

  auto dontThreadOnMe = [](LPVOID lpParam) -> DWORD {
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;

    // Create a vector of char* in the child process
    std::vector<char*> cstrings;
    cstrings.push_back("C:\\WINDOWS\\SYSTEM32\\CMD.EXE");
    cstrings.push_back("cmd.exe ");
    cstrings.push_back("/c");
    for (const std::string &part : pDataArray->_cmd)
    {
      cstrings.push_back(const_cast<char *>(part.c_str()));
    }

    // Add the nullptr termination.
    cstrings.push_back(nullptr);

    // Run the command, replacing the current process image
    if (_spawnv(_P_WAIT , cstrings[0], &cstrings[0]) < 0)
    {
      gzerr << "Unable to run command["
        << std::accumulate(
            pDataArray->_cmd.begin(),
            pDataArray->_cmd.end(),
            std::string(""))
        << "] " << GetLastErrorAsString() << "\n";
      return -1;
    }

    if (!ReleaseSemaphore(pDataArray->stoppedChildSem, 1, nullptr))
    {
      gzerr << "Error Releasing Semaphore "
             << GetLastErrorAsString() << std::endl;
    }

    if(pDataArray != NULL)
    {
      HeapFree(GetProcessHeap(), 0, pDataArray);
      pDataArray = NULL;    // Ensure address is not reused.
    }

    return 0;
  };

  auto thread = CreateThread(
    nullptr, 0, dontThreadOnMe, pDataArray, 0, nullptr);

  if (thread == nullptr) {
    gzerr << "Error creating thread on Windows "
           << GetLastErrorAsString() << '\n';
  }
  else
  {
    std::lock_guard<std::mutex> mutex(this->executablesMutex);
    this->master = true;

    // Store the PID in the parent process.
    this->executables.push_back(Executable(
          _name, thread, _cmd, _autoRestart, _envs));
  }
#else
  // Fork a process for the command
  pid_t pid = fork();

  // If parent process...
  if (pid)
  {
    gzdbg << "Forked a process for [" << _name << "] command["
      << std::accumulate(_cmd.begin(), _cmd.end(), std::string("")) << "]\n"
      << std::flush;

    std::lock_guard<std::mutex> mutex(this->executablesMutex);
    this->master = true;
    // Store the PID in the parent process.
    this->executables.push_back(Executable(
          _name, pid, _cmd, _autoRestart, _envs));
  }
  // Else child process...
  else
  {
    // A child is not the master
    this->master = false;

    // Create a vector of char* in the child process
    std::vector<char*> cstrings;
    for (const std::string &part : _cmd)
    {
      cstrings.push_back(const_cast<char *>(part.c_str()));
    }

    // Add the nullptr termination.
    cstrings.push_back(nullptr);

    // Remove from foreground process group.
    setpgid(0, 0);

    this->SetEnvs(_envs);

    // Run the command, replacing the current process image
    if (execvp(cstrings[0], &cstrings[0]) < 0)
    {
      gzerr << "Unable to run command["
             << std::accumulate(
                _cmd.begin(),
                _cmd.end(),
                std::string("")) << "]\n";
      return false;
    }
  }
#endif
  return true;
}

/////////////////////////////////////////////////
void ManagerPrivate::ShutdownExecutables()
{
  std::lock_guard<std::mutex> mutex(this->executablesMutex);

#ifndef _WIN32
  // Remove the sigchld signal handler
  signal(SIGCHLD, nullptr);
#endif

  // Create a vector of monitor threads that wait for each process to stop.
  std::vector<std::thread> monitors;
  for (const Executable &exec : this->executables)
    monitors.push_back(std::thread([&] {
#ifndef _WIN32
      waitpid(exec.pid, nullptr, 0);
#else
      WaitForSingleObject(exec.pi, INFINITE);
      int retVal = ReleaseSemaphore(myself->stoppedChildSem, 1, nullptr);
      if (retVal != 0)
      {
        gzerr << "Error Releasing Semaphore: "
               << GetLastErrorAsString() << std::endl;
      }
#endif
    }));

#ifndef _WIN32
  for (const pid_t &wrapper : this->wrappedPlugins)
    monitors.push_back(std::thread([&] {
      waitpid(wrapper, nullptr, 0);
    }));
#else
  for (const PROCESS_INFORMATION &wrapper : this->wrappedPlugins)
    monitors.push_back(std::thread([&] {
      WaitForSingleObject(wrapper.hProcess, INFINITE);
      int retVal = ReleaseSemaphore(myself->stoppedChildSem, 1, nullptr);
      if (retVal != 0)
      {
        gzerr << "Error Releasing Semaphore: "
               << GetLastErrorAsString() << std::endl;
      }
    }));
#endif

  // Shutdown the processes
  for (const Executable &exec : this->executables)
  {
    gzdbg << "Killing the process[" << exec.name
#ifndef _WIN32
      << "] with PID[" << exec.pid << "]\n";
    kill(exec.pid, SIGINT);
#else
  << "]\n";
#endif
  }

#ifndef _WIN32
  // Shutdown the wrapped plugins
  for (const pid_t &pid : this->wrappedPlugins)
#else
  for (const PROCESS_INFORMATION &pid : this->wrappedPlugins)
#endif
  {
#ifndef _WIN32
    gzdbg << "Killing the wrapped plugin PID[" << pid << "]\n";
    kill(pid, SIGINT);
#else
    gzdbg << "Killing the wrapped plugin PID[" << pid.dwProcessId << "]\n";
    TerminateProcess(pid.hProcess, 0);
#endif
  }

  gzdbg << "Waiting for each process to end\n";

  // Wait for all the monitors to stop
  for (std::thread &m : monitors)
    m.join();
  gzdbg << "All finished\n";
}

//////////////////////////////////////////////////
void ManagerPrivate::ParseExecutables(const tinyxml2::XMLElement *_elem)
{
  // Process all the executables.
  const tinyxml2::XMLElement *execElem = _elem->FirstChildElement("executable");

  // This "i" variable is just used for output messages.
  for (int i = 0; execElem && this->master; ++i)
  {
    bool autoRestart = false;
    bool valid = true;
    std::vector<std::string> cmdParts;

    // Get the executable's name
    std::string nameStr = execElem->Attribute("name");
    if (nameStr.empty())
    {
      valid = false;
      gzerr << "Invalid configuration file, "
        << "missing name attribute for the " << i << " <executable> element."
        << std::endl;
    }

    // Get the command
    const tinyxml2::XMLElement *cmdElem =
      execElem->FirstChildElement("command");
    if (!cmdElem)
    {
      valid = false;
      gzerr << "Invalid configuration file, "
        << "missing <command> child element "
        << " of <executable name=\"" << nameStr << "\">\n";
    }
    else
    {
      std::vector<std::string> parts =
        gz::common::split(cmdElem->GetText(), " ");
      std::move(parts.begin(), parts.end(), std::back_inserter(cmdParts));
    }

    // Get the <auto_restart> element. It is okay if the <auto_restart> element
    // is not present
    const tinyxml2::XMLElement *restartElem = execElem->FirstChildElement(
        "auto_restart");
    if (restartElem && restartElem->GetText())
    {
      std::string txt = gz::common::lowercase(restartElem->GetText());
      autoRestart = txt == "true" || txt == "1" || txt == "t";
    }

    std::list<std::string> localEnvs = this->ParseEnvs(execElem);

    if (valid)
    {
      if (!this->RunExecutable(nameStr, cmdParts, autoRestart, localEnvs))
      {
        gzerr << "Unable to run executable named[" << nameStr << "] in "
          << "configuration file.\n";
      }
    }

    execElem = execElem->NextSiblingElement("executable");
  }
}

//////////////////////////////////////////////////
std::list<std::string> ManagerPrivate::ParseEnvs(
    const tinyxml2::XMLElement *_elem)
{
  std::list<std::string> result;

  // Get the environment variables
  const tinyxml2::XMLElement *envElem = _elem->FirstChildElement("env");
  while (envElem)
  {
    const tinyxml2::XMLElement *nameElem = envElem->FirstChildElement("name");
    const tinyxml2::XMLElement *valueElem =
      envElem->FirstChildElement("value");
    if (nameElem && valueElem)
    {
      std::string name = nameElem->GetText();
      std::string value = valueElem->GetText();

      // Expand env var contents, such as $LD_LIBRARY_PATH
      if (!value.empty() && value.at(0) == '$')
      {
        gz::common::env(value.substr(1), value);
      }

      result.push_back(name + "=" + value);
    }

    envElem = envElem->NextSiblingElement("env");
  }

  return result;
}

//////////////////////////////////////////////////
void ManagerPrivate::LoadPlugin(const tinyxml2::XMLElement *_elem)
{
  // Get the plugin's name
  const char *nameStr = _elem->Attribute("name");
  std::string name = nameStr == nullptr ? "" : nameStr;
  if (name.empty())
  {
    gzerr << "Invalid config file, "
      << "missing a name attribute for a plugin." << std::endl;
    return;
  }

  // Get the plugin's filename
  const char *fileStr = _elem->Attribute("filename");
  std::string file = fileStr == nullptr ? "" : fileStr;
  if (file.empty())
  {
    gzerr << "Invalid config file, "
      << "missing filename attribute for plugin with name[" << name << "]"
      << std::endl;
    return;
  }

  if (name == "gz::launch::SimServer")
  {
#ifdef _WIN32
    _putenv_s("RMT_PORT", "1500");
#else
    setenv("RMT_PORT", "1500", 1);
#endif
  }
  else if (name == "gz::launch::SimGui")
  {
#ifdef _WIN32
    _putenv_s("RMT_PORT", "1501");
#else
    setenv("RMT_PORT", "1501", 1);
#endif
  }

  gz::common::SystemPaths systemPaths;
  systemPaths.SetPluginPathEnv("GZ_LAUNCH_PLUGIN_PATH");
  systemPaths.AddPluginPaths(gz::launch::getPluginInstallPath());

  // Add LD_LIBRARY_PATH
#ifdef __linux__
  std::string libPath;
  gz::common::env("LD_LIBRARY_PATH", libPath);
  systemPaths.AddPluginPaths(libPath);
#endif

  // Add in the gazebo plugin path for convenience
  std::string homePath;
  gz::common::env(GZ_HOMEDIR, homePath);

  systemPaths.AddPluginPaths(
    gz::common::joinPaths(homePath, ".gz", "sim", "plugins"));

  std::string pathToLib;
  if (common::exists(file))
    pathToLib = file;
  else
    pathToLib = systemPaths.FindSharedLibrary(file);

  if (pathToLib.empty())
  {
    gzerr << "Failed to find the path to library[" << file << "]. "
      << "Try adding the path to the GZ_LAUNCH_PLUGIN_PATH environment "
      << "variable.\n";
    return;
  }

  plugin::Loader loader;
  std::unordered_set<std::string> localPlugins = loader.LoadLib(pathToLib);
  if (localPlugins.empty())
  {
    gzerr << "Failed to load plugin [" << pathToLib << "] library\n";
    return;
  }

  std::unordered_set<std::string> validPlugins =
    loader.PluginsImplementing<gz::launch::Plugin>();
  if (validPlugins.count(name) == 0)
  {
    std::string availablePlugins;
    for (const std::string &vp : validPlugins)
      availablePlugins += vp + " ";

    gzerr << "Failed to find implementation with name[" << name << "] in "
      << file << ". Available implementations include: "
      << availablePlugins << std::endl;
    return;
  }

  gzdbg << "Loading plugin. Name[" << name
    << "] File[" << file << "]" << std::endl;

  PluginPtr plugin = loader.Instantiate(name);
  if (!plugin->QueryInterface<Plugin>()->Load(_elem))
    gzerr << "Unable to load " << name << std::endl;
  this->plugins.insert(plugin);
}

//////////////////////////////////////////////////
void ManagerPrivate::ParseExecutableWrappers(
  const tinyxml2::XMLElement *_elem)
{
  // Process all the executables.
  const tinyxml2::XMLElement *execElem = _elem->FirstChildElement(
      "executable_wrapper");
#ifndef _WIN32
  std::list<pid_t> pluginPids;
#endif

  // This "i" variable is just used for output messages.
  for (int i = 0; execElem && this->master; ++i)
  {
    const tinyxml2::XMLElement *pluginElem =
      execElem->FirstChildElement("plugin");
    if (pluginElem)
    {
      // Fork a process for the command
#ifndef _WIN32
      pid_t pid = fork();
      // If parent process...
      if (pid)
      {
        this->master = true;
        pluginPids.push_back(pid);
      }
      else
      {
        this->master = false;

        // Remove from foreground process group.
        setpgid(0, 0);

        this->plugins.clear();
        this->wrappedPlugins.clear();
        this->sigHandler.reset();
        this->backward.reset();
        this->executables.clear();
        this->LoadPlugin(pluginElem);
        return;
      }
#else
#endif
    }
    execElem = execElem->NextSiblingElement("executable_wrapper");
  }

#ifndef _WIN32
  if (this->master)
    this->wrappedPlugins = pluginPids;
#endif
}

//////////////////////////////////////////////////
void ManagerPrivate::SetEnvs(const std::list<std::string> &_envs)
{
  for (const std::string &env : _envs)
  {
    #ifdef _WIN32
      _putenv_s(const_cast<char*>(env.c_str()), "");
    #else
      putenv(const_cast<char*>(env.c_str()));
    #endif
  }
}
