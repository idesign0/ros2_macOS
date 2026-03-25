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

#ifndef GZ_UTILS_TEST_IMPLPTR_TEST_CLASSES_HH_
#define GZ_UTILS_TEST_IMPLPTR_TEST_CLASSES_HH_

#include <gz/utils/ImplPtr.hh>
#include <gz/utils/Export.hh>

#include <functional>
#include <string>

namespace gz
{
  namespace implptr_test_classes
  {
    /// \brief A forward-declared implementation class
    class ObjectPrivate;

    /// \brief A PIMPL test class that can be copied.
    class GZ_UTILS_VISIBLE CopyableObject
    {
      /// \brief Constructor
      public: CopyableObject(int _ivalue = 0,
                             const std::string &_svalue = "");

      /// \brief Get the int value held by the pimpl
      public: int GetInt() const;

      /// \brief Set the int value held by the pimpl
      public: void SetInt(int _value);

      /// \brief Get the string value held by the pimpl
      public: const std::string &GetString() const;

      /// \brief Set the string value held by the pimpl
      public: void SetString(const std::string &_value);

      /// \brief Uses a thread to increment the internal integer value from
      /// 0 to 1.
      /// \return The final internal integer value, which should be 1.
      public: int ThreadIncrementInt();

      /// \brief Pointer to implementation
      /// This demonstrates using an implementation class that is
      /// forward-declared outside of this class.
      GZ_UTILS_IMPL_PTR_FWD(ObjectPrivate, dataPtr)
    };

    /// \brief A PIMPL test class that cannot be copied; it can only be moved.
    class GZ_UTILS_VISIBLE MovableObject
    {
      /// \brief Constructor
      public: MovableObject(int _ivalue = 0,
                            const std::string &_svalue = "");

      /// \brief Get the int value held by the pimpl
      public: int GetInt() const;

      /// \brief Set the int value held by the pimpl
      public: void SetInt(int _value);

      /// \brief Get the string value held by the pimpl
      public: const std::string &GetString() const;

      /// \brief Set the string value held by the pimpl
      public: void SetString(const std::string &_value);

      /// \brief Uses a thread to increment the internal integer value from
      /// 0 to 1.
      /// \return The final internal integer value, which should be 1.
      public: int ThreadIncrementInt();

      /// \brief Pointer to implementation
      /// This demonstrates using an implementation class that is
      /// forward-declared outside of this class.
      GZ_UTILS_UNIQUE_IMPL_PTR_FWD(ObjectPrivate, dataPtr)
    };

    /// \brief A PIMPL test class that can be copied (alternate definition)
    class GZ_UTILS_VISIBLE CopyableObjectAlt
    {
      /// \brief Constructor
      public: CopyableObjectAlt(int _ivalue = 0,
                                const std::string &_svalue = "");

      /// \brief Get the int value held by the pimpl
      public: int GetInt() const;

      /// \brief Set the int value held by the pimpl
      public: void SetInt(int _value);

      /// \brief Get the string value held by the pimpl
      public: const std::string &GetString() const;

      /// \brief Set the string value held by the pimpl
      public: void SetString(const std::string &_value);

      /// \brief Pointer to implementation
      /// This will automatically forward declare a class named
      /// "Implementation" inline
      GZ_UTILS_IMPL_PTR(dataPtr)
    };

  }  // namespace implptr_test_classes
}  // namespace gz

#endif  // GZ_UTILS_TEST_IMPLPTR_TEST_CLASSES_HH_
