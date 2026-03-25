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

#ifndef GZ_UTILS_CLI_GZ_FORMATTER_HPP_
#define GZ_UTILS_CLI_GZ_FORMATTER_HPP_

#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "gz/utils/cli/App.hpp"
#include "gz/utils/cli/FormatterFwd.hpp"
#include "gz/utils/cli/StringTools.hpp"
#include "gz/utils/Export.hh"

//////////////////////////////////////////////////
/// \brief CLI Formatter class that implements custom Gazebo-specific
/// formatting.
///
/// More information on custom formatters:
/// https://cliutils.github.io/CLI11/book/chapters/formatting.html
class GzFormatter: public CLI::Formatter {

//////////////////////////////////////////////////
public: explicit GzFormatter(const CLI::App *_app)
{
  // find needs/needed_by for root options
  for (const CLI::Option *appOpt: _app->get_options())
  {
    for(const CLI::Option *needsOpt : appOpt->get_needs())
    {
      this->needed_by.insert({needsOpt->get_name(), appOpt->get_name()});
      this->needs.insert({appOpt->get_name(), needsOpt->get_name()});
    }
  }

  // find needs/needed_by for subcommand (or command group) options
  auto subcommands = _app->get_subcommands([](const CLI::App*){return true;});
  for (const CLI::App *sub : subcommands)
  {
    // find needs/needed_by for root options
    for (const CLI::Option *subOpt: sub->get_options())
    {
      for(const CLI::Option *needsOpt : subOpt->get_needs())
      {
        this->needed_by.insert({needsOpt->get_name(), subOpt->get_name()});
        this->needs.insert({subOpt->get_name(), needsOpt->get_name()});
      }
    }
  }
}

//////////////////////////////////////////////////
public: std::string make_option_name(
            const CLI::Option *opt, bool is_positional) const override {
  if (is_positional)
    return opt->get_name(true, false);

  std::stringstream out;

  auto snames = opt->get_snames();
  auto lnames = opt->get_lnames();

  std::vector<std::string> sname_list;
  std::transform(snames.begin(), snames.end(), std::back_inserter(sname_list),
      [](const std::string &sname) { return "-" + sname; });

  std::vector<std::string> lname_list;
  std::transform(lnames.begin(), lnames.end(), std::back_inserter(lname_list),
      [](const std::string &lname) { return "--" + lname; });

  // If no short options, just use long
  if (sname_list.empty())
  {
    out << CLI::detail::join(lname_list);
  }
  else
  {
    out << CLI::detail::join(sname_list);
    // Put lnames in brackets to look like ruby formatting
    if (!lnames.empty())
    {
      out << " [" << CLI::detail::join(lname_list) << "]";
    }
  }

  return out.str();
}


//////////////////////////////////////////////////
public: std::string make_option_opts(const CLI::Option *opt) const override {
    std::stringstream out;

    if(opt->get_type_size() != 0) {
        if(!opt->get_type_name().empty())
            out << " " << get_label(opt->get_type_name());
        if(!opt->get_default_str().empty())
            out << "=" << opt->get_default_str();
        if(opt->get_expected_max() == CLI::detail::expected_max_vector_size)
            out << " ...";
        else if(opt->get_expected_min() > 1)
            out << " x " << opt->get_expected();
    }
    if(!opt->get_envname().empty())
        out << " (" << get_label("Env") << ":" << opt->get_envname() << ")";
    return out.str();
}


//////////////////////////////////////////////////
std::string make_option_desc(const CLI::Option *opt) const override {
  std::stringstream out;

  out << opt->get_description();

  if (opt->get_required())
  {
    out << "\nREQUIRED";
  }

  auto range = this->needs.equal_range(opt->get_name());
  std::for_each(
      range.first,
      range.second,
      [&out](const auto &opt_name)
      {
        out << "\nRequires: " << opt_name.second;
      });

  range = this->needed_by.equal_range(opt->get_name());
  std::for_each(
      range.first,
      range.second,
      [&out](const auto &opt_name)
      {
        out << "\nRequired by: " << opt_name.second;
      });

  if (!opt->get_excludes().empty()) {
    out << "\n" << get_label("Excludes") << ":";
    for(const CLI::Option *op : opt->get_excludes())
      out << " " << op->get_name();
  }

  return out.str() + '\n';
}

/// \brief Track dependent options
private: std::unordered_multimap<std::string, std::string> needs;

/// \brief Track dependent options (inverse)
private: std::unordered_multimap<std::string, std::string> needed_by;
};

#endif  // GZ_UTILS_CLI_GZ_FORMATTER_HPP_
