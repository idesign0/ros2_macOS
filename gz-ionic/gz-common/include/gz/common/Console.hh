/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_CONSOLE_HH_
#define GZ_COMMON_CONSOLE_HH_

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include <gz/common/Export.hh>
#include <gz/common/Util.hh>
#include <gz/utils/log/Logger.hh>
#include <gz/utils/SuppressWarning.hh>

namespace gz
{
  namespace common
  {
    /// \brief Helper class for providing gzlog macros.
    class GZ_COMMON_VISIBLE LogMessage
    {
      /// \brief Constructor.
      /// \param[in] _file Filename.
      /// \param[in] _line Line number.
      /// \param[in] _logLevel Log level.
      public: LogMessage(const char *_file,
                         int _line,
                         spdlog::level::level_enum _logLevel);

      /// \brief Destructor.
      public: ~LogMessage();

      /// \brief Get access to the underlying stream.
      /// \return The underlying stream.
      public: std::ostream &stream();

      /// \brief Log level.
      private: spdlog::level::level_enum severity;

      /// \brief Source file location information.
      private: spdlog::source_loc sourceLocation;

      /// \brief Underlying stream.
      private: std::ostringstream ss;
    };

    /// \brief Output a critical message.
    #define gzcrit (gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::critical).stream())

    /// \brief Output an error message.
    #define gzerr gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::err).stream()

    /// \brief Output a warning message.
    #define gzwarn gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::warn).stream()

    /// \brief Output a message to a log file.
    #define gzlog gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::trace).stream()

    /// \brief Output a message.
    #define gzmsg gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::info).stream()

    /// \brief Output a debug message.
    #define gzdbg gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::debug).stream()

    /// \brief Output a trace message.
    #define gztrace gz::common::LogMessage( \
      __FILE__, __LINE__, spdlog::level::trace).stream()

    /// \brief Initialize log file with filename given by _dir/_file.
    /// If called twice, it will close the file currently in use and open a new
    /// log file.
    /// \param[in] _dir Name of directory in which to store the log file. Note
    /// that if _dir is not an absolute path, then _dir will
    /// be relative to your home directory.
    /// \param[in] _file Name of log file for gzlog messages.
    #define gzLogInit(_dir, _file)\
      gz::common::Console::Init(_dir, _file)

    /// \brief Close the file used for logging.
    #define gzLogClose()\
      gz::common::Console::Close()

    /// \brief Get the full path of the directory where the log files are stored
    /// \return Full path of the directory
    #define gzLogDirectory()\
      (gz::common::Console::Directory())

    /// \class Console Console.hh common/common.hh
    /// \brief Container for loggers, and global logging options
    /// (such as verbose vs. quiet output).
    class GZ_COMMON_VISIBLE Console : public gz::utils::log::Logger
    {
      /// \brief Class constructor.
      /// \param[in] _loggerName Logger name.
      public: explicit Console(const std::string &_loggerName);

      /// \brief Access the global gz console logger.
      /// \return The gz consoler logger.
      public: static Console &Root();

      /// \brief Initialize the global logger.
      /// \param[in] _directory Name of directory that holds the log file.
      /// \param[in] _filename Name of the log file to write output into.
      /// \return True when the initialization succeed or false otherwise.
      public: static bool Init(const std::string &_directory,
                               const std::string &_filename);

      /// \brief Detach fhe file sink from the global logger. After this call,
      /// console logging will keep working but no file logging.
      public: static void Close();

      /// \brief Get the full path of the directory where all the log files
      /// are stored.
      /// \return Full path of the directory.
      public: static std::string Directory();

      /// \brief Set verbosity, where
      /// 0: Critical messages,
      /// 1: Critical, error messages,
      /// 2: Critical, error and warning messages,
      /// 3: Critical, error, warning, and info messages,
      /// 4: Critical, error, warning, info, and debug messages.
      /// 5: Critical, error, warning, info, debug, and trace messages.
      /// \param[in] _level The new verbose level.
      public: static void SetVerbosity(const int _level);

      /// \brief Get the verbose level.
      /// \return The level of verbosity.
      /// \sa SetVerbosity(const int _level)
      public: static int Verbosity();

      /// \brief Add a custom prefix in front of the default prefixes.
      ///
      /// By default, the custom prefix is an empty string, so the messages
      /// start as:
      ///
      /// [Err], [Wrn], [Msg], [Dbg]
      ///
      /// If you set the prefix to "-my-", for example, they become:
      ///
      /// -my-[Err], -my-[Wrn], -my-[Msg], -my-[Dbg]
      ///
      /// \param[in] _customPrefix Prefix string.
      /// \sa std::string Prefix() const
      public: static void SetPrefix(const std::string &_customPrefix);

      /// \brief Get custom prefix. This is empty by default.
      /// \return The custom prefix.
      /// \sa void SetPrefix(const std::string &_customPrefix)
      public: static std::string Prefix();

      /// \brief True if initialized.
      public: static bool initialized;

      /// \brief The level of verbosity, the default level is 1.
      private: static int verbosity;

      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief A custom prefix. See SetPrefix().
      private: static std::string customPrefix;

      /// \brief Stores the full path of the directory where all the log files
      /// are stored.
      private: std::string logDirectory;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}
#endif
