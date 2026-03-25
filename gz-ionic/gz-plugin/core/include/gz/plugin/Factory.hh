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

#ifndef GZ_PLUGIN_FACTORY_HH_
#define GZ_PLUGIN_FACTORY_HH_

#include <chrono>
#include <functional>
#include <memory>
#include <tuple>

#include <gz/plugin/EnablePluginFromThis.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief This class provides a unary operator for safely deleting pointers
    /// to plugin factory products with the type `Interface`. If it gets passed
    /// an `Interface` pointer that is not pointing to a factory plugin product,
    /// then this just performs a normal delete.
    template <typename Interface> class ProductDeleter;

    /// \brief ProductPtr is a derivative of std::unique_ptr that can safely
    /// manage the products that come out of a plugin factory. It is strongly
    /// recommended that factory products use a ProductPtr to manage the
    /// lifecycle of a factory product.
    ///
    /// If you MUST release a factory product pointer, then it should at least
    /// be passed to a ProductDeleter to be deleted at the end of its lifecycle.
    /// If it is not possible to delete the product with a ProductDeleter, then
    /// you will need to periodically call CleanupLostProducts() or else you
    /// will experience memory leaks, and your factory plugin libraries will
    /// never get unloaded.
    template <typename Interface>
    using ProductPtr =
        std::unique_ptr<Interface, ProductDeleter<Interface>>;

    /// \brief The Factory class defines a plugin factory that can be used by
    /// the Loader class to produce products that implement an interface.
    ///
    /// To define the inputs and outputs of a factory, set the template
    /// arguments, for example:
    ///
    /// \code
    /// using MyFactory = Factory<InterfaceClass, InputType1, InputType2>;
    /// \endcode
    ///
    /// defines a factory that can produce a `std::unique_ptr<InterfaceClass>`
    /// given arguments of `InputType1` and `InputType2`.
    ///
    /// To register a factory, use the `GZ_ADD_FACTORY` macro, e.g.:
    ///
    /// \code
    /// GZ_ADD_FACTORY(ImplementedClass, MyFactory)
    /// \endcode
    ///
    /// where `ImplementedClass` is the name of the class that your plugin
    /// library has used to implement `InterfaceClass`.
    template <typename Interface, typename... Args>
    class Factory : public EnablePluginFromThis
    {
      public: using ProductPtrType = ProductPtr<Interface>;

      /// \brief This function is called by Loader to construct the class that
      /// implements the InterfacePtr interface.
      /// \param[in] _args
      ///   The arguments as defined by the template parameters.
      /// \return an RAII-managed reference to the interface type as defined by
      /// the template parameters.
      public: ProductPtrType Construct(Args&&... _args);

      /// \internal \brief This function gets implemented by Producing<Product>
      /// to manufacture the product instance.
      /// \param[in] _args
      ///   The arguments as defined by the template parameters
      /// \return a raw pointer to the product
      private: virtual Interface *ImplConstruct(Args&&... _args) = 0;

      /// \private This nested class is used to implement the plugin factory.
      /// It is not intended for external use.
      public: template <typename Product>
      class Producing;
    };

    /// \brief Call this function to cleanup the Factories of any Products which
    /// were not managed by a ProductPtr or deleted by a ProductDeleter (in
    /// other words, the Product was released from its ProductPtr and then its
    /// lifecycle was managed by a framework that does not know it has special
    /// deletion requirements).
    ///
    /// If you never call the .release() function on a ProductPtr, then you will
    /// never need to call this function.
    ///
    /// \warning Note that this function should be called ONLY while there are
    /// no Products that are actively destructing, or else there is a minuscule
    /// probability of causing a segmentation fault. This is never an issue in a
    /// single-threaded application.
    ///
    /// \warning If you have a multi-threaded application where you have
    /// absolutely no control over the lifecycle of the products, and you cannot
    /// reliably predict a safe window in which you know that no products are
    /// being actively deleted, then you can specify a short safety wait to the
    /// cleanup call. If any products were being deleted while this function is
    /// called, this wait can give them time to fully exit their destructors
    /// before we unload their libraries.
    ///
    /// \note For some applications, it might not be important if there are tiny
    /// memory leaks or if plugin libraries remain loaded until the application
    /// exits. For those applications, it is okay to not bother calling this
    /// function at all.
    ///
    /// \param[in] _safetyWait
    ///   For multi-threaded applications, this short waiting window gives time
    ///   for products that are currently being deleted to exit their
    ///   destructors before we unload their libraries. If you can reliably
    ///   predict a window of time in which no products are actively being
    ///   destructed (or if you have a single-threaded application), then it is
    ///   okay to set this waiting time to 0. Note that any threads which are
    ///   about to destruct a product will be blocked until this wait is over.
    void GZ_PLUGIN_VISIBLE CleanupLostProducts(
        const std::chrono::nanoseconds &_safetyWait =
            std::chrono::nanoseconds(5));

    /// \brief Get the number of lost products that have currently accumulated
    /// since the last time CleanupLostProducts() was called (or since the
    /// program began, if CleanupLostProducts() has not been called yet).
    std::size_t GZ_PLUGIN_VISIBLE LostProductCount();
  }
}

#include <gz/plugin/detail/Factory.hh>

#endif
