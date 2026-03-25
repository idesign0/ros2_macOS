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

#include <gtest/gtest.h>

#include <gz/plugin/Factory.hh>
#include <gz/plugin/Loader.hh>

#include "../plugins/FactoryPlugins.hh"
#include "utils.hh"

using namespace test::util;

/////////////////////////////////////////////////
TEST(Factory, Inspect)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzFactoryPlugins_LIB);

  std::cout << pl.PrettyStr() << std::endl;

  EXPECT_EQ(2u, pl.PluginsImplementing<NameFactory>().size());
  EXPECT_EQ(2u, pl.PluginsImplementing<DoubleFactory>().size());
  EXPECT_EQ(2u, pl.PluginsImplementing<IntFactory>().size());
  EXPECT_EQ(2u, pl.PluginsImplementing<SomeObjectFactory>().size());
}

/////////////////////////////////////////////////
TEST(Factory, Construct)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzFactoryPlugins_LIB);

  auto nameFactory = pl.Factory<NameFactory>("test::util::DummyNameForward");
  ASSERT_NE(nullptr, nameFactory);
  EXPECT_EQ("John Doe", nameFactory->Construct("John Doe")->MyNameIs());

  nameFactory = pl.Factory<NameFactory>("test::util::DummyNameSayHello");
  ASSERT_NE(nullptr, nameFactory);
  EXPECT_EQ("Hello, John Doe!", nameFactory->Construct("John Doe")->MyNameIs());

  auto doubleFactory =
      pl.Factory<DoubleFactory>("test::util::DummyDoubleForward");
  ASSERT_NE(nullptr, doubleFactory);
  EXPECT_DOUBLE_EQ(5.3, doubleFactory->Construct(5.3)->MyDoubleValueIs());

  doubleFactory =
      pl.Factory<DoubleFactory>("test::util::DummyDoubleAddOneHalf");
  ASSERT_NE(nullptr, doubleFactory);
  EXPECT_DOUBLE_EQ(5.8, doubleFactory->Construct(5.3)->MyDoubleValueIs());

  auto intFactory =
      pl.Factory<IntFactory>("test::util::DummyIntForward");
  ASSERT_NE(nullptr, intFactory);
  EXPECT_EQ(68, intFactory->Construct(68)->MyIntegerValueIs());

  intFactory =
      pl.Factory<IntFactory>("test::util::DummyIntAddOne");
  ASSERT_NE(nullptr, intFactory);
  EXPECT_EQ(69, intFactory->Construct(68)->MyIntegerValueIs());

  auto someObjectFactory =
      pl.Factory<SomeObjectFactory>("test::util::SomeObjectForward");
  ASSERT_NE(nullptr, someObjectFactory);
  auto someObject = someObjectFactory->Construct(7, 6.5);
  ASSERT_NE(nullptr, someObject);
  EXPECT_EQ(7, someObject->someInt);
  EXPECT_DOUBLE_EQ(6.5, someObject->someDouble);
  EXPECT_DOUBLE_EQ(7 + 6.5, someObject->SomeOperation());

  someObjectFactory =
      pl.Factory<SomeObjectFactory>("test::util::SomeObjectAddTwo");
  ASSERT_NE(nullptr, someObjectFactory);
  someObject = someObjectFactory->Construct(7, 6.5);
  ASSERT_NE(nullptr, someObject);
  EXPECT_EQ(9, someObject->someInt);
  EXPECT_DOUBLE_EQ(8.5, someObject->someDouble);
  EXPECT_DOUBLE_EQ(9 + 8.5 + 2, someObject->SomeOperation());

  auto nullFactory =
      pl.Factory<SomeObjectFactory>("not a real factory");
  EXPECT_EQ(nullptr, nullFactory);
}

/////////////////////////////////////////////////
TEST(Factory, Alias)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzFactoryPlugins_LIB);

  std::string symbolName;
  for (const std::string &name : pl.AllPlugins())
  {
    if (name.find("gz::plugin::Factory") != std::string::npos &&
        name.find("test::util::SomeObjectAddTwo") != std::string::npos)
    {
      symbolName = name;
      break;
    }
  }

  EXPECT_FALSE(symbolName.empty());

  for (auto alias : {
       symbolName.c_str(),
       "test::util::SomeObjectAddTwo",
       "This factory has an alias",
       "and also a second alias"})
  {
    std::shared_ptr<SomeObjectFactory> factory =
        pl.Factory<SomeObjectFactory>(alias);
    EXPECT_NE(nullptr, factory) << " failed on name: " << alias;
    if (!factory)
      continue;

    auto object = factory->Construct(110, 2.25);
    ASSERT_NE(nullptr, object);
    EXPECT_EQ(112, object->someInt);
    EXPECT_DOUBLE_EQ(4.25, object->someDouble);
  }
}

/////////////////////////////////////////////////
TEST(Factory, LibraryManagement)
{
  const std::string &libraryPath = GzFactoryPlugins_LIB;

  // Test that a single ProductPtr will keep the library loaded and correctly
  // manage the lifecycle of the product.
  {
    SomeObjectFactory::ProductPtrType obj;

    {
      gz::plugin::Loader pl;
      pl.LoadLib(libraryPath);
      CHECK_FOR_LIBRARY(libraryPath, true);

      auto factory = pl.Factory<SomeObjectFactory>(
            "test::util::SomeObjectAddTwo");
      ASSERT_NE(nullptr, factory);

      obj = factory->Construct(1, 2.0);
      ASSERT_NE(nullptr, obj);

      // These values are based on us loading a SomeObjectAddTwo plugin
      EXPECT_EQ(3, obj->someInt);
      EXPECT_DOUBLE_EQ(4.0, obj->someDouble);
    }

    CHECK_FOR_LIBRARY(libraryPath, true);
  }

  CHECK_FOR_LIBRARY(libraryPath, false);

  // Test that we can release from a ProductPtr, and the library will still
  // remain loaded, and then it will unload correctly later, as long as we
  // manually destruct with a ProductDeleter
  {
    SomeObject *obj;

    {
      gz::plugin::Loader pl;
      pl.LoadLib(libraryPath);
      CHECK_FOR_LIBRARY(libraryPath, true);

      auto factory = pl.Factory<SomeObjectFactory>(
            "test::util::SomeObjectForward");
      ASSERT_NE(nullptr, factory);

      obj = factory->Construct(1, 2.0).release();
      ASSERT_NE(nullptr, obj);

      // These values are based on us loading a SomeObjectForward plugin
      EXPECT_EQ(1, obj->someInt);
      EXPECT_DOUBLE_EQ(2.0, obj->someDouble);
    }

    CHECK_FOR_LIBRARY(libraryPath, true);

    gz::plugin::ProductDeleter<SomeObject>()(obj);

    CHECK_FOR_LIBRARY(libraryPath, false);
  }

  CHECK_FOR_LIBRARY(libraryPath, false);

  // Test that if we release from a ProductPtr but do not delete it with a
  // ProductDeleter, then its shared library will remain loaded until we call
  // CleanupLostProducts().
  {
    std::unique_ptr<SomeObject> obj;

    {
      gz::plugin::Loader pl;
      pl.LoadLib(libraryPath);
      CHECK_FOR_LIBRARY(libraryPath, true);

      auto factory = pl.Factory<SomeObjectFactory>(
            "test::util::SomeObjectAddTwo");
      ASSERT_NE(nullptr, factory);

      obj = std::unique_ptr<SomeObject>(factory->Construct(3, 4.0).release());
      ASSERT_NE(nullptr, obj);

      // These values are based on us loading a SomeObjectAddTwo plugin
      EXPECT_EQ(5, obj->someInt);
      EXPECT_DOUBLE_EQ(6.0, obj->someDouble);
    }

    CHECK_FOR_LIBRARY(libraryPath, true);
  }

  // Now the reference count for the library has been passed into the
  // lostProductManager.
  EXPECT_EQ(1u, gz::plugin::LostProductCount());

  // As long as the reference count is in the lostProductManager, the library
  // will remain loaded.
  CHECK_FOR_LIBRARY(libraryPath, true);

  // This function will clean up all the lost products, deleting their reference
  // counts.
  gz::plugin::CleanupLostProducts();

  // Now there should be no more lost products, so this count is 0.
  EXPECT_EQ(0u, gz::plugin::LostProductCount());

  // With the reference counts deleted, the library should automatically unload.
  CHECK_FOR_LIBRARY(libraryPath, false);
}
