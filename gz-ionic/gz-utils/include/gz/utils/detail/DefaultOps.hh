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

#ifndef GZ_UTILS__DETAIL__DEFAULTOPS_HH_
#define GZ_UTILS__DETAIL__DEFAULTOPS_HH_

#include <type_traits>

namespace gz
{
  namespace utils
  {
    namespace detail
    {
      //////////////////////////////////////////////////
      template <class T>
      void VerifyComplete()
      {
        // If you are brought here by a compilation error while using the
        // ImplPtr class or the UniqueImplPtr class, be sure to use the
        // MakeImpl<T>() or MakeUniqueImpl<T>() function when constructing your
        // [Unique]ImplPtr instance.
        static_assert(sizeof(T) > 0,
                      "DefaultDelete cannot delete an incomplete type");
        static_assert(!std::is_void<T>::value,
                      "DefaultDelete cannot delete an incomplete type");
      }

      //////////////////////////////////////////////////
      template <class T>
      void DefaultDelete(T *_ptr) noexcept
      {
        VerifyComplete<T>();
        delete _ptr;
      }

      //////////////////////////////////////////////////
      template <class T>
      T *DefaultCopyConstruct(const T &_source)
      {
        VerifyComplete<T>();
        return new T(_source);
      }

      //////////////////////////////////////////////////
      template <class T>
      void DefaultCopyAssign(T &_dest, const T &_source)
      {
        VerifyComplete<T>();
        _dest = _source;
      }

      //////////////////////////////////////////////////
      template <class T,
                class CopyConstruct = T* (*)(const T&),
                class CopyAssign = void (*)(T&, const T&)>
      struct CopyMoveDeleteOperations
      {
        public: template <class C, class A>
        CopyMoveDeleteOperations(C &&_construct, A &&_assign);

        public: CopyConstruct construct;
        public: CopyAssign assign;
      };
    }  // namespace detail
  }  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS__DETAIL__DEFAULTOPS_HH_
