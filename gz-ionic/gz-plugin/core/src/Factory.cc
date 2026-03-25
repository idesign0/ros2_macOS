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

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <gz/plugin/Factory.hh>

namespace
{
  struct LostProductManager
  {
    /// \brief This class is designed to handle situations where users have lost
    /// control of their plugin lifecycle management, so we should assume that
    /// it's possible for plugin objects to be getting destructed and/or cleaned
    /// up across different threads at the same time.
    ///
    /// To guard against race conditions, we should use a mutex when modifying
    /// this object with a static lifetime.
    public: std::mutex mutex;

    /// \brief This is a collection of references to the factories of products
    /// that did not get properly deleted by a ProductDeleter. We will store
    /// their factory references here to ensure that their libraries remain
    /// loaded so the products can be deleted safely.
    ///
    /// If a user knows that they are losing control of their products, they can
    /// call CleanupLostProducts() to clear out this vector and clean up any
    /// potential memory leaks.
    public: std::vector<std::shared_ptr<void>> lostProducts;
  };

  /// static instance of the lost product manager that will be used to store the
  /// factory references of any lost products.
  static LostProductManager lostProductManager;
}  // namespace

namespace gz
{
  namespace plugin
  {
    namespace detail
    {
      FactoryCounter::~FactoryCounter()
      {
        if (this->factoryPluginInstancePtr)
        {
          // If the reference to the factory plugin is still a valid pointer,
          // that means this product is not being deleted by a ProductDeleter.
          // If we allow this destruction to proceed as normal, then the library
          // for this plugin object might get unloaded while the call stack is
          // still inside one of the library's symbols. Whenever this happens,
          // it seems to create a segmentation fault in the application, because
          // the call stack will be viewing memory that no longer belongs to the
          // application.
          //
          // Instead of allowing mysterious segmentation faults to occur, we
          // will hand off a copy of the factory reference to the
          // lostProductManager which will keep it alive until the user
          // explicitly calls CleanupLostProducts().
          std::unique_lock<std::mutex> lock(lostProductManager.mutex);
          lostProductManager.lostProducts.push_back(
                this->factoryPluginInstancePtr);
        }
      }
    }

    void CleanupLostProducts(const std::chrono::nanoseconds &_safetyWait)
    {
      std::unique_lock<std::mutex> lock(lostProductManager.mutex);

      // In case any products are in-between handing off their factory reference
      // and exiting their destructor, wait for a short while so that the call
      // stack can fully exit the destructor before we unload its library.
      std::this_thread::sleep_for(_safetyWait);

      lostProductManager.lostProducts.clear();
    }

    std::size_t LostProductCount()
    {
      std::unique_lock<std::mutex> lock(lostProductManager.mutex);
      return lostProductManager.lostProducts.size();
    }
  }
}
