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

#ifndef GZ_PLUGIN_DETAIL_FACTORY_HH_
#define GZ_PLUGIN_DETAIL_FACTORY_HH_

#include <memory>
#include <utility>

#include <gz/utils/SuppressWarning.hh>

#include <gz/plugin/Factory.hh>

namespace gz
{
  namespace plugin
  {
    namespace detail
    {
      /// \brief This base class gets mixed in with a Product so that the
      /// Product can keep track of the factory that produced it. This allows
      /// the factory's library to remain loaded (so that the Product's symbols
      /// remain available to the application). Without this, the shared library
      /// might get unloaded while the Product is still alive, and then the
      /// application will experience a segmentation fault as soon as it tries
      /// to do anything with the Product (including deleting it).
      class GZ_PLUGIN_VISIBLE FactoryCounter
      {
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        /// \brief A reference to the factory that created this product
        private: std::shared_ptr<void> factoryPluginInstancePtr;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief A special destructor that ensures the shared library remains
        /// loaded throughout the destruction process of this product.
        public: virtual ~FactoryCounter();

        // friendship declaration
        template <typename, typename...> friend class gz::plugin::Factory;
        template <typename> friend class gz::plugin::ProductDeleter;
      };
    }

    template <typename Interface>
    class ProductDeleter
    {
      /// \brief This is a unary function for deleting product pointers. It
      /// keeps the factory reference alive while the product is being deleted,
      /// and then cleans up the factory reference immediately afterwards.
      ///
      /// This is the recommended method for deleting product pointers.
      public: void operator()(Interface *_ptr)
      {
        detail::FactoryCounter *counter =
            dynamic_cast<detail::FactoryCounter*>(_ptr);

        std::shared_ptr<void> factoryPluginInstancePtr;
        if (counter)
        {
          // Hold onto the factory instance pointer while the product completes
          // its destruction.
          //
          // At the same time, clear out the product's reference to its factory
          // so that it knows that it's being deleted by a ProductDeleter.
          // Otherwise, it will intentionally leak its factory reference to
          // avoid causing a segmentation fault in the application.
          factoryPluginInstancePtr.swap(counter->factoryPluginInstancePtr);
        }

        delete _ptr;
      }
    };

    template <typename Interface, typename... Args>
    auto Factory<Interface, Args...>::Construct(Args&&... _args)
        -> ProductPtrType
    {
      return ProductPtrType(this->ImplConstruct(std::forward<Args>(_args)...));
    }

    /// \brief Producing provides the implementation of Factory for a specific
    /// derivative of Factory's Interface type. That derivative is called
    /// Product, which must be a fully-defined class that implements Interface.
    ///
    /// The mechanism of this class ensures that as long as the Factory output
    /// object is alive, the plugin library that it depends on will remain
    /// safely loaded.
    template <typename Interface, typename... Args>
    // cppcheck-suppress syntaxError
    template <typename Product>
    class Factory<Interface, Args...>::Producing
        : public Factory<Interface, Args...>
    {
      /// \brief This class mixes the product implementation with the factory
      /// counter so that the factory remains alive and the library remains
      /// loaded throughout the lifecycle of the product.
      ///
      /// Dev note(MXG): FactoryCounter is inherited first so that its
      /// destructor gets called last. In case this product is not deleted by a
      /// ProductDeleter, having the FactoryCounter destruct last will minimize
      /// the amount of time between handing off its shared_ptr to the
      /// lostProductManager and the time that the call stack leaves the
      /// destructor of this symbol entirely. This makes it less risky to call
      /// CleanupLostProducts() in a multi-threaded application. Combined with
      /// giving CleanupLostProducts() a brief waiting period, this should allow
      /// even completely reckless multi-threaded applications to be able to
      /// cleanup its lost products safely.
      class ProductWithFactoryCounter
          : public detail::FactoryCounter,
            public Product
      {
        /// \brief Forwarding constructor
        public: ProductWithFactoryCounter(Args&&... _args)
          : Product(std::forward<Args>(_args)...)
        {
          // Do nothing
        }
      };

      // Documentation inherited
      private: Interface *ImplConstruct(Args&&... _args) override
      {
        auto *product =
            new ProductWithFactoryCounter(std::forward<Args>(_args)...);

        product->factoryPluginInstancePtr = this->PluginInstancePtrFromThis();

        return product;
      }
    };
  }
}

#endif
