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
#include <iostream>
#include <regex>
#include <string>

#if defined(__GNUC__) || defined(__clang__)
// This header is used for name demangling on GCC and Clang
#include <cxxabi.h>
#endif

#include <gz/plugin/utility.hh>

namespace gz
{
  namespace plugin
  {
    /////////////////////////////////////////////////
    std::string DemangleSymbol(const std::string &_symbol)
    {
    #if defined(__GNUC__) || defined(__clang__)
      int status;
      char *demangled_cstr = abi::__cxa_demangle(
            _symbol.c_str(), nullptr, nullptr, &status);

      if (0 != status)
      {
        // LCOV_EXCL_START
        std::cerr << "[Demangle] Failed to demangle the symbol name ["
                  << _symbol << "]. Error code: " << status << "\n";
        assert(false);
        return _symbol;
        // LCOV_EXCL_STOP
      }

      const std::string demangled(demangled_cstr);
      free(demangled_cstr);

      return demangled;
    #elif _MSC_VER

      assert(_symbol.substr(0, 6) == "class ");

      // Visual Studio's typeid(~).name() does not mangle the name, except that
      // it prefixes the normal name of the class with the character sequence
      // "class ". So to get the "demangled" name, all we have to do is remove
      // "class " from each place where it appears.
      const std::regex classRegex("class ");
      return std::regex_replace(_symbol, classRegex, "");
    #else
      // If we don't know the compiler, then we can't perform name demangling.
      // The tests will probably fail in this situation, and the class names
      // will probably look gross to users. Plugin name aliasing can be used
      // to make plugins robust to this situation.
      return _symbol;
    #endif
    }
  }
}
