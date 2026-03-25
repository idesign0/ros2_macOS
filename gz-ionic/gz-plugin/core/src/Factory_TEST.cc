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

#include <string>
#include <vector>

#include <gz/plugin/Factory.hh>

// Dev Note (MXG): These tests are just here to make sure that the template and
// inheritance schemes for Factories are working as expected. If the
// implementation of Factory changes, then these tests may also change. The
// important Factory tests that should remain stable are in
// test/integration/Factory_TEST.

struct SomeBase
{
  virtual ~SomeBase() = default;
};

struct SomeDerived : public SomeBase
{
  SomeDerived()
  {
    // Do nothing
  }

  SomeDerived(double, int)
  {
    // Do nothing
  }

  explicit SomeDerived(const std::vector<double>&)
  {
    // Do nothing
  }
};

using NoArgFactory = gz::plugin::Factory<SomeBase>;
using NoArgProducer = NoArgFactory::Producing<SomeDerived>;

using DoubleIntFactory = gz::plugin::Factory<SomeBase, double, int>;
using DoubleIntProducer = DoubleIntFactory::Producing<SomeDerived>;

using VectorFactory = gz::plugin::Factory<
    SomeBase, const std::vector<double>&>;
using VectorProducer = VectorFactory::Producing<SomeDerived>;

/////////////////////////////////////////////////
TEST(Factory, ProducerNoArgs)
{
  NoArgProducer producer;
  EXPECT_NE(nullptr, producer.Construct());
}

/////////////////////////////////////////////////
TEST(Factory, ProducerWithDoubleInt)
{
  DoubleIntProducer producer;
  EXPECT_NE(nullptr, producer.Construct(0.0, 1u));
}

/////////////////////////////////////////////////
TEST(Factory, ProducerWithVector)
{
  VectorProducer producer;
  EXPECT_NE(nullptr, producer.Construct(std::vector<double>()));
}
