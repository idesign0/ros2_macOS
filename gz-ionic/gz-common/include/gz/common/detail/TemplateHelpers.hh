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

#ifndef GZ_COMMON_DETAIL_TEMPLATEHELPERS_HH_
#define GZ_COMMON_DETAIL_TEMPLATEHELPERS_HH_


#include <type_traits>

namespace gz
{
  namespace common
  {
    namespace detail
    {
      //////////////////////////////////////////////////
      template <typename To, typename From>
      struct ConstCompatible : std::true_type
      {
      };

      //////////////////////////////////////////////////
      template <typename To, typename From>
      struct ConstCompatible<To, const From>
          : std::integral_constant<bool, std::is_const<To>::value>
      {
      };
    }
  }
}

#endif
