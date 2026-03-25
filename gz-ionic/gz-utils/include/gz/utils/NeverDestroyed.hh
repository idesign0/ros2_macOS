/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#ifndef GZ_UTILS_NEVERDESTROYED_HH_
#define GZ_UTILS_NEVERDESTROYED_HH_

#include <new>
#include <type_traits>
#include <utility>

namespace gz
{
namespace utils
{
/// Originally copied from https://github.com/RobotLocomotion/drake/blob/v0.36.0/common/never_destroyed.h
/// Originally licensed BSD 3-Clause (https://github.com/RobotLocomotion/drake/blob/v0.36.0/LICENSE.TXT)
/// Re-licensed Apache-2.0 with permission from:
/// jwnimmer-tri (https://github.com/gazebosim/gz-utils/pull/31#issuecomment-989173512)
///
/// Wraps an underlying type T such that its storage is a direct member field
/// of this object (i.e., without any indirection into the heap), but *unlike*
/// most member fields T's destructor is never invoked.
///
/// This is especially useful for function-local static variables that are not
/// trivially destructable.  We shouldn't call their destructor at program exit
/// because of the "indeterminate order of ... destruction" as mentioned in
/// cppguide's
/// <a href="https://google.github.io/styleguide/cppguide.html#Static_and_Global_Variables">Static
/// and Global Variables</a> section, but other solutions to this problem place
///  the objects on the heap through an indirection.
///
/// Compared with other approaches, this mechanism more clearly describes the
/// intent to readers, avoids "possible leak" warnings from memory-checking
/// tools, and is probably slightly faster.
///
/// Example uses:
///
/// The singleton pattern:
/// @code
/// class Singleton
/// {
///  public:
///   Singleton(const Singleton&) = delete;
///   void operator=(const Singleton&) = delete;
///   Singleton(Singleton&&) = delete;
///   void operator=(Singleton&&) = delete;
///
///   static Singleton& getInstance()
///   {
///     static gz::utils::NeverDestroyed<Singleton> instance;
///     return instance.access();
///   }
///  private:
///   friend gz::utils::NeverDestroyed<Singleton>;
///   Singleton() = default;
/// };
/// @endcode
///
/// A lookup table, created on demand the first time its needed, and then
/// reused thereafter:
/// @code
/// enum class Foo { kBar, kBaz };
/// Foo ParseFoo(const std::string& foo_string)
/// {
///   using Dict = std::unordered_map<std::string, Foo>;
///   static const gz::utils::NeverDestroyed<Dict> string_to_enum
///   {
///     std::initializer_list<Dict::value_type>
///     {
///       {"bar", Foo::kBar},
///       {"baz", Foo::kBaz},
///     }
///   };
///   return string_to_enum.access().at(foo_string);
/// }
/// @endcode
///
/// In cases where computing the static data is more complicated than an
/// initializer_list, you can use a temporary lambda to populate the value:
/// @code
/// const std::vector<double>& GetConstantMagicNumbers()
/// {
///   static const gz::utils::NeverDestroyed<std::vector<double>> result
///   {
///   []()
///   {
///     std::vector<double> prototype;
///     std::mt19937 random_generator;
///     for (int i = 0; i < 10; ++i)
///     {
///       double new_value = random_generator();
///       prototype.push_back(new_value);
///     }
///     return prototype;
///   }()
///   };
///   return result.access();
/// }
/// @endcode
///
/// Note in particular the `()` after the lambda. That causes it to be invoked.
//
// The above examples are repeated in the unit test; keep them in sync.
template <typename T>
class NeverDestroyed
{
  /// Passes the constructor arguments along to T using perfect forwarding.
  public: template <typename... Args>
          explicit NeverDestroyed(Args &&... args)
  {
    // Uses "placement new" to construct a `T` in `storage_`.
    new (&this->storage) T(std::forward<Args>(args)...);
  }

  /// Does nothing.  Guaranteed!
  public: ~NeverDestroyed() = default;

  /// \brief Deleted copy constructor
  public: NeverDestroyed(const NeverDestroyed &) = delete;

  /// \brief Deleted move constructor
  public: NeverDestroyed(NeverDestroyed &&) = delete;

  /// \brief Deleted copy assignment constructor
  public: NeverDestroyed &operator=(const NeverDestroyed &) = delete;

  /// \brief Deleted move assignment constructor
  public: NeverDestroyed &operator=(NeverDestroyed &&) noexcept = delete;

  /// Returns the underlying T reference.
  public: T &Access()
  {
    return *reinterpret_cast<T *>(&this->storage);
  }

  public: const T &Access() const
  {
    return *reinterpret_cast<const T *>(&this->storage);
  }

  private: typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
};
}  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS_NEVERDESTROYED_HH_
