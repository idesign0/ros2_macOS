/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <memory>

#include <gz/utils/cli/CLI.hpp>
#include <gz/utils/cli/GzFormatter.hpp>

/////////////////////////////////////////////////
struct TestOptions
{
  bool fooFlag;
  int barFlag;

  int intOption;
  float floatOption;
  std::string stringOption;
  std::vector<std::string> multistringOption;

  std::string needsOption;
  std::string neededByOption;

  std::string requiredOption;

  int excludesOptionA;
  int excludesOptionB;

  float defaultValueOption {6.28f};
};

/////////////////////////////////////////////////
std::shared_ptr<TestOptions> addFlags(CLI::App &_app)
{
  auto opt = std::make_shared<TestOptions>();
  // Example of adding basic flags
  _app.add_flag("-f,--foo", opt->fooFlag, "Just a flag");
  _app.add_flag("-b,--bar", opt->barFlag, "Another flag");

  // Example of adding basic options
  _app.add_option("--int", opt->intOption, "Option that takes integer");
  _app.add_option("--float", opt->floatOption, "Option that takes float");
  _app.add_option("--string", opt->stringOption,
      "Option that takes string.\n  But also more description\n Another line");

  auto multistringOpt = _app.add_option("--stringV",
      opt->multistringOption, "Takes several strings");
  multistringOpt->expected(2, 5);
  multistringOpt->delimiter(',');

  // Example of adding required option
  _app.add_option("--required",
      opt->requiredOption,
      "This is a required option.")->required();

  // Example of adding dependent options
  auto neededByOpt = _app.add_option("--needed-by",
      opt->neededByOption,
      "This is an option another option needs.");
  auto needsOpt = _app.add_option("--needs",
      opt->needsOption,
      "This is an option that needs another option.");
  needsOpt->needs(neededByOpt);

  // Example of adding mutually-exclusive options
  auto excludesA = _app.add_option("--excludesA", opt->excludesOptionA,
      "Only A or B can be used.");
  auto excludesB = _app.add_option("--excludesB", opt->excludesOptionB,
      "Only A or B can be used.");
  excludesA->excludes(excludesB);
  excludesB->excludes(excludesA);

  _app.add_option("--default-value", opt->defaultValueOption,
      "Option with default value")->capture_default_str();

  return opt;
}

/////////////////////////////////////////////////
TEST(cli, flags)
{
  CLI::App app("Test app");

  auto opt = addFlags(app);

  std::vector<std::string> argv = {"--foo", "--bar", "-b", "--required=1"};

  app.callback([opt](){
    EXPECT_TRUE(opt->fooFlag);
    EXPECT_EQ(2, opt->barFlag);
  });

  EXPECT_NO_THROW(app.parse(argv));
}

/////////////////////////////////////////////////
TEST(cli, options)
{
  CLI::App app("Test app");

  auto opt = addFlags(app);

  std::vector<std::string> argv = {
    "--required=1",
    "--int=50",
    "--float=3.14",
    "--string=baz",
    "--stringV=foo,bar,baz",
  };

  app.callback([opt](){
      EXPECT_EQ(50, opt->intOption);
      EXPECT_EQ("baz", opt->stringOption);
      EXPECT_FLOAT_EQ(3.14f, opt->floatOption);
      EXPECT_EQ(3u, opt->multistringOption.size()) << opt->multistringOption[0];
  });

  app.formatter(std::make_shared<GzFormatter>(&app));
  EXPECT_NO_THROW(app.help());
  EXPECT_NO_THROW(app.parse(argv));
}

/////////////////////////////////////////////////
TEST(cli, help_text)
{
  CLI::App app("Test app");

  auto opt = addFlags(app);
  app.callback([opt](){});

  app.formatter(std::make_shared<GzFormatter>(&app));
  std::cout << app.help();
}

/////////////////////////////////////////////////
TEST(cli, config_text)
{
  CLI::App app("Test app");

  auto opt = addFlags(app);
  app.callback([opt](){});

  std::vector<std::string> argv = {
    "--int=50",
    "--float=3.14",
    "--string=bing",
    "--needs=foo",
    "--needed-by=bar",
    "--required=baz"
  };

  EXPECT_NO_THROW(app.parse(argv));

  std::cout << app.config_to_str(true, true);
}

