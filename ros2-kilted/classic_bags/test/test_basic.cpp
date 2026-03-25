/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2023, Metro Robots
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Metro Robots nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: David V. Lu!! */

#include <gtest/gtest.h>
#include <filesystem>
#include <classic_bags/bag.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>

class BagTest : public ::testing::Test
{
public:
  BagTest()
  {
    temporary_dir_path_ = std::filesystem::temp_directory_path() / "cpp_tmp_test_dir_";
    std::filesystem::create_directory(temporary_dir_path_);
  }

  ~BagTest() override
  {
    std::filesystem::remove_all(temporary_dir_path_);
  }

  void SetUp() override
  {
    bag_path_ = temporary_dir_path_ / ::testing::UnitTest::GetInstance()->current_test_info()->name();
  }

  void WriteSimpleBag(const std::string& storage_id = "")
  {
    classic_bags::Bag out_bag(std::string(bag_path_), classic_bags::Write, storage_id);
    std_msgs::msg::String s;
    s.data = "muppets";
    std_msgs::msg::Int32 i;
    i.data = 42;
    out_bag.write("/chatter", s);
    out_bag.write("/numbers", i);
    out_bag.close();
  }

  void CheckSimpleBag(const std::string& storage_id = "")
  {
    classic_bags::Bag bag = classic_bags::Bag(std::string(bag_path_), classic_bags::Read, storage_id);
    ASSERT_TRUE(bag.hasNext());
    auto msg0 = bag.readNext();
    ASSERT_TRUE(bag.hasNext());
    auto msg1 = bag.readNext();
    ASSERT_FALSE(bag.hasNext());

    EXPECT_EQ(msg0->getTopic(), "/chatter");
    EXPECT_EQ(msg1->getTopic(), "/numbers");

    std_msgs::msg::String::SharedPtr s = msg0->instantiate<std_msgs::msg::String>();
    std_msgs::msg::Int32::SharedPtr i = msg1->instantiate<std_msgs::msg::Int32>();

    ASSERT_TRUE(s != nullptr);
    ASSERT_TRUE(i != nullptr);

    EXPECT_EQ(s->data, "muppets");
    EXPECT_EQ(i->data, 42);

    s = msg1->instantiate<std_msgs::msg::String>();
    i = msg0->instantiate<std_msgs::msg::Int32>();

    ASSERT_TRUE(s == nullptr);
    // TODO: We are able to cast the string as an int for unknown reasons
    // ASSERT_TRUE(i == nullptr);
  }

  std::filesystem::path temporary_dir_path_;
  std::filesystem::path bag_path_;
};

TEST_F(BagTest, test_default)
{
  WriteSimpleBag();
  CheckSimpleBag();
}

TEST_F(BagTest, test_mcap)
{
  WriteSimpleBag("mcap");
  CheckSimpleBag("mcap");
}

TEST_F(BagTest, test_sqlite3)
{
  WriteSimpleBag("sqlite3");
  CheckSimpleBag("sqlite3");
}

TEST_F(BagTest, test_mcap_default)
{
  WriteSimpleBag("mcap");
  CheckSimpleBag();
}

TEST_F(BagTest, test_sqlite_default)
{
  WriteSimpleBag("sqlite3");
  CheckSimpleBag();
}

TEST_F(BagTest, test_sqlite_mcap)
{
  WriteSimpleBag("sqlite3");
  EXPECT_ANY_THROW(CheckSimpleBag("mcap"));
}

TEST_F(BagTest, test_mcap_sqlite3)
{
  WriteSimpleBag("mcap");
  EXPECT_ANY_THROW(CheckSimpleBag("sqlite3"));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
