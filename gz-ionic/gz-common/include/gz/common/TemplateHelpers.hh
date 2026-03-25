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


#ifndef GZ_COMMON_TEMPLATEHELPERS_HH_
#define GZ_COMMON_TEMPLATEHELPERS_HH_

#include "gz/common/detail/TemplateHelpers.hh"

namespace gz
{
  namespace common
  {
    /// \brief Contains a static constexpr field named `value` which will be
    /// true if the type `From` has a const-quality less than or equal to the
    /// type `To`.
    ///
    /// The following expressions will return true:
    ///
    /// \code
    ///     ConstCompatible<T, T>::value
    ///     ConstCompatible<const T, T>::value
    /// \endcode
    ///
    /// The following expression will return false:
    ///
    /// \code
    ///     ConstCompatible<T, const T>::value
    /// \endcode
    ///
    template <typename To, typename From>
    using ConstCompatible = detail::ConstCompatible<To, From>;
  }
}

#endif
