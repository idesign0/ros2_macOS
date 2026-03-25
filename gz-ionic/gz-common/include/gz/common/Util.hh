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
#ifndef GZ_COMMON_UTIL_HH_
#define GZ_COMMON_UTIL_HH_

#include <cassert>
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gz/common/Export.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/SystemPaths.hh>
#include <gz/common/URI.hh>

/////////////////////////////////////////////////
// Defines

#ifdef _WIN32
# define GZ_HOMEDIR "USERPROFILE"
#else
# define GZ_HOMEDIR "HOME"
#endif

/// \brief Seconds in one nano second.
#define GZ_NANO_TO_SEC 1e-9

/// \brief Nano seconds in one second.
#define GZ_SEC_TO_NANO 1000000000

/// \brief Nano seconds in one millisecond.
#define GZ_MS_TO_NANO 1000000

/// \brief Nano seconds in one microsecond.
#define GZ_US_TO_NANO 1000

/// \brief Speed of light.
#define GZ_SPEED_OF_LIGHT = 299792458.0

/// \brief Sleep for the specifed number of seconds
#define GZ_SLEEP_S(_s) (std::this_thread::sleep_for(\
                         std::chrono::seconds(_s)))

/// \brief Sleep for the specifed number of microseconds
#define GZ_SLEEP_US(_us) (std::this_thread::sleep_for(\
                           std::chrono::microseconds(_us)))

/// \brief Sleep for the specifed number of milliseconds
#define GZ_SLEEP_MS(_ms) (std::this_thread::sleep_for(\
                           std::chrono::milliseconds(_ms)))

/// \brief Sleep for the specifed number of nanoseconds
#define GZ_SLEEP_NS(_ns) (std::this_thread::sleep_for(\
                           std::chrono::nanoseconds(_ns)))

/// \brief Get the system time.
#define GZ_SYSTEM_TIME() (std::chrono::system_clock::now())

/// \brief Get the system time in seconds since epoch.
#define GZ_SYSTEM_TIME_S() (std::chrono::duration_cast<std::chrono::seconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in microseconds since epoch.
#define GZ_SYSTEM_TIME_US() (\
    std::chrono::duration_cast<std::chrono::microseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in milliseconds since epoch.
#define GZ_SYSTEM_TIME_MS() (\
    std::chrono::duration_cast<std::chrono::milliseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief Get the system time in nanoseconds since epoch.
#define GZ_SYSTEM_TIME_NS() (\
    std::chrono::duration_cast<std::chrono::nanoseconds>(\
      std::chrono::system_clock::now().time_since_epoch()).count())

/// \brief This macro defines the standard way of launching an exception
/// inside Gazebo.
#define GZ_ASSERT(_expr, _msg) assert((_msg, _expr))

/// \brief Forward declarations for the common classes
namespace gz
{
  namespace common
  {
    /// \brief Enumeration of the transform types
    enum NodeTransformType {TRANSLATE, ROTATE, SCALE, MATRIX};

    /// \brief A runtime error.
    typedef std::runtime_error exception;

    /// \brief Please use systemTimeIso() as this function will be deprecated
    /// in Gazebo Common 4.
    /// Get the wall time as an ISO string: YYYY-MM-DDTHH:MM:SS.NS
    /// \return The current wall time as an ISO string.
    /// \sa systemTimeIso();
    std::string GZ_COMMON_VISIBLE systemTimeISO();

    /// \brief Get the wall time as an ISO string: YYYY-MM-DDTHH:MM:SS.NS
    /// \return The current wall time as an ISO string.
    std::string GZ_COMMON_VISIBLE systemTimeIso();

    /// \brief Converts a time point to an ISO string: YYYY-MM-DDTHH:MM:SS.NS
    /// \param[in] _time A time point, such as one created by
    /// GZ_SYSTEM_TIME().
    /// \return An ISO string that represents the given _time.
    std::string GZ_COMMON_VISIBLE timeToIso(
        const std::chrono::time_point<std::chrono::system_clock> &_time);

    /// \brief Get the log path
    /// \return the log path
    std::string GZ_COMMON_VISIBLE logPath();

    /// \brief add path suffix to common::SystemPaths
    /// \param[in] _suffix The suffix to add.
    void GZ_COMMON_VISIBLE addSearchPathSuffix(
        const std::string &_suffix);

    /// \brief search for file in common::SystemPaths
    /// \param[in] _file Name of the file to find.
    /// \return The path containing the file.
    std::string GZ_COMMON_VISIBLE findFile(const std::string &_file);

    /// \brief search for file in common::SystemPaths
    /// \param[in] _file Name of the file to find.
    /// \param[in] _searchLocalPath True to search in the current working
    /// directory.
    /// \return The path containing the file.
    std::string GZ_COMMON_VISIBLE findFile(const std::string &_file,
                         bool _searchLocalPath);

    /// \brief search for a file in common::SystemPaths
    /// \param[in] _file the file name to look for.
    /// \return The path containing the file.
    std::string GZ_COMMON_VISIBLE findFilePath(const std::string &_file);

    /// \brief Add a callback to use when findFile() can't find a file that is
    /// a valid URI.
    /// The callback should return a full local path to the requested file, or
    /// and empty string if the file was not found in the callback.
    /// Callbacks will be called in the order they were added until a path is
    /// found.
    /// \param[in] _cb The callback function, which takes a file path or URI
    /// and returns the full local path.
    void GZ_COMMON_VISIBLE addFindFileURICallback(
        std::function<std::string(const URI &)> _cb);

    /// \brief Get a pointer to the global system paths that is used by all
    /// the findFile functions.
    /// The returned instance has global shared state for a given process.
    /// Care should be taken when manipulating global system paths
    /// Caller should not assume ownership of the pointer.
    /// \return A mutable reference to the system paths object.
    common::SystemPaths GZ_COMMON_VISIBLE *systemPaths();

    /// \brief Compute the SHA1 hash of an array of bytes.
    /// \param[in] _buffer Input sequence. The permitted data types for this
    /// function are std::string and any STL container.
    /// \return The string representation (40 character) of the SHA1 hash.
    template<typename T>
    std::string sha1(const T &_buffer);

    /// \brief Compute the SHA1 hash of an array of bytes. Use std::string
    /// sha1(const T &_buffer) instead of this function
    /// \param[in] _buffer Input sequence. The permitted data types for this
    /// function are std::string and any STL container.
    /// \return The string representation (40 character) of the SHA1 hash.
    /// \sa sha1(const T &_buffer)
    std::string GZ_COMMON_VISIBLE sha1(
        void const *_buffer, std::size_t _byteCount);

    /// \brief fnv1a algorithm for 64-bit platforms.
    /// \param[in] _key The input string.
    /// \return A 64-bit unsigned hash value.
    /// \ref https://notes.underscorediscovery.com/constexpr-fnv1a/
    constexpr uint64_t GZ_COMMON_VISIBLE hash64(std::string_view _key)
    {
      const char *data = _key.data();
      const auto len = _key.size();
      const uint64_t prime = 0x100000001b3;
      uint64_t hash = 0xcbf29ce484222325;

      for (auto i = 0u; i < len; ++i)
      {
        uint8_t value = static_cast<uint8_t>(data[i]);
        hash = hash ^ value;
        hash *= prime;
      }

      return hash;
    }

    /// \brief Find the environment variable '_name' and return its value.
    ///
    /// \TODO(mjcarroll): Deprecate and remove in tick-tock.
    ///
    /// \param[in] _name Name of the environment variable.
    /// \param[out] _value Value if the variable was found.
    /// \return True if the variable was found or false otherwise.
    bool GZ_COMMON_VISIBLE env(
        const std::string &_name, std::string &_value);

    /// \brief Find the environment variable '_name' and return its value.
    /// \param[in] _name Name of the environment variable.
    /// \param[out] _value Value if the variable was found.
    /// \param[in] _allowEmpty Allow set-but-empty variables.
    ///           (Unsupported on Windows)
    /// \return True if the variable was found or false otherwise.
    bool GZ_COMMON_VISIBLE env(
        const std::string &_name, std::string &_value,
        bool _allowEmpty);

    /// \brief Set the environment variable '_name'.
    ///
    /// Note that on Windows setting an empty string (_value=="")
    /// is the equivalent of unsetting the variable.
    ///
    /// \param[in] _name Name of the environment variable.
    /// \param[in] _value Value of the variable to be set.
    /// \return True if the variable was set or false otherwise.
    bool GZ_COMMON_VISIBLE setenv(
        const std::string &_name, const std::string &_value);

    /// \brief Unset the environment variable '_name'.
    /// \param[in] _name Name of the environment variable.
    /// \return True if the variable was unset or false otherwise.
    bool GZ_COMMON_VISIBLE unsetenv(const std::string &_name);

    /// \brief Get a UUID
    /// \return A UUID string
    std::string GZ_COMMON_VISIBLE uuid();

    /// \brief Splits a string into tokens.
    /// \param[in] _str Input string.
    /// \param[in] _delim Token delimiter.
    /// \return Vector of tokens.
    std::vector<std::string> GZ_COMMON_VISIBLE split(
        const std::string &_str, const std::string &_delim);

    /// \brief In place left trim
    /// \param[in,out] _s String to trim
    void GZ_COMMON_VISIBLE ltrim(std::string &_s);

    /// \brief In place right trim
    /// \param[in,out] _s String to trim
    void GZ_COMMON_VISIBLE rtrim(std::string &_s);

    /// \brief In place trim from both ends
    /// \param[in,out] _s String to trim
    void GZ_COMMON_VISIBLE trim(std::string &_s);

    /// \brief Copying left trim
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string GZ_COMMON_VISIBLE ltrimmed(std::string _s);

    /// \brief Copying right trim
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string GZ_COMMON_VISIBLE rtrimmed(std::string _s);

    /// \brief Copying trim from both ends
    /// \param[in] _s String to trim
    /// \return Trimmed string
    std::string GZ_COMMON_VISIBLE trimmed(std::string _s);

    /// \brief Transforms a string to its lowercase equivalent
    /// \param[in] _in String to convert to lowercase
    /// \return Lowercase equilvalent of _in.
    std::string GZ_COMMON_VISIBLE lowercase(const std::string &_in);

    /// \brief Transforms a string to its lowercase equivalent
    /// \param[in] _in String to convert to lowercase
    /// \return Lowercase equilvalent of _in.
    std::string GZ_COMMON_VISIBLE lowercase(const char *_in);

    /// \brief Replace all occurances of _key with _replacement.
    /// \param[out] _result The new string that has had _key replaced
    /// with _replacement.
    /// \param[in] _orig Original string.
    /// \param[in] _key String to replace.
    /// \param[in] _replacement The string that replaces _key.
    /// \sa  std::string replaceAll(const std::string &_orig,
    /// const std::string &_key, const std::string &_replacement)
    void GZ_COMMON_VISIBLE replaceAll(std::string &_result,
        const std::string &_orig,
        const std::string &_key,
        const std::string &_replacement);

    /// \brief Replace all occurances of _key with _replacement.
    /// \param[in] _orig Original string.
    /// \param[in] _key String to replace.
    /// \param[in] _replacement The string that replaces _key.
    /// \return The new string that has had _key replaced with _replacement.
    /// \sa void common::replaceAll(std::string &_result,
    /// const std::string &_orig, const std::string &_key,
    /// const std::string &_replacement)
    std::string GZ_COMMON_VISIBLE replaceAll(const std::string &_orig,
        const std::string &_key,
        const std::string &_replacement);
  }
}

///////////////////////////////////////////////
// Implementation of get_sha1
template<typename T>
std::string gz::common::sha1(const T &_buffer)
{
  if (_buffer.size() == 0)
    return gz::common::sha1(NULL, 0);
  else
  {
    return gz::common::sha1(
        &(_buffer[0]), _buffer.size() * sizeof(_buffer[0]));
  }
}
#endif
