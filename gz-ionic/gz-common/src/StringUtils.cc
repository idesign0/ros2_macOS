/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <cstdlib>
#include "gz/common/StringUtils.hh"

namespace gz
{
  namespace common
  {
    //////////////////////////////////////////////////
    std::vector<std::string> Split(const std::string &_orig, char _delim)
    {
      std::vector<std::string> pieces;
      size_t pos1 = 0;
      size_t pos2 = _orig.find(_delim);
      while (pos2 != std::string::npos)
      {
        pieces.push_back(_orig.substr(pos1, pos2-pos1));
        pos1 = pos2+1;
        pos2 = _orig.find(_delim, pos2+1);
      }
      pieces.push_back(_orig.substr(pos1, _orig.size()-pos1));
      return pieces;
    }

    //////////////////////////////////////////////////
    std::string Join(const std::vector<std::string> &_orig,
                     const std::string &_delim)
    {
      std::string ret = "";
      for (size_t ii = 0; ii < _orig.size(); ++ii)
      {
        if (_orig[ii].empty())
          continue;

        ret += _orig[ii];
        if (ii < _orig.size() - 1)
        {
          ret += _delim;
        }
      }
      return ret;
    }
    //////////////////////////////////////////////////
    std::string Join(const std::vector<std::string> &_orig, char _delim)
    {
      return Join(_orig, std::string(1, _delim));
    }

    //////////////////////////////////////////////////
    bool StartsWith(const std::string &_s1, const std::string &_s2)
    {
      if (_s1.size() >= _s2.size())
      {
        return 0 == _s1.compare(0, _s2.size(), _s2);
      }
      return false;
    }

    /////////////////////////////////////////////////
    bool EndsWith(const std::string &_s1, const std::string &_s2)
    {
      if (_s1.size() >= _s2.size())
      {
        return 0 == _s1.compare(_s1.size() - _s2.size(), _s2.size(), _s2);
      }
      return false;
    }

    //////////////////////////////////////////////////
    std::string PluralCast(const std::string &_baseWord, const int _n)
    {
      return PluralCast(_baseWord, _baseWord+"s", _n);
    }

    //////////////////////////////////////////////////
    std::string PluralCast(const std::string &_singular,
                           const std::string &_plural,
                           const int _n)
    {
      return std::abs(_n) == 1? _singular : _plural;
    }
  }
}
