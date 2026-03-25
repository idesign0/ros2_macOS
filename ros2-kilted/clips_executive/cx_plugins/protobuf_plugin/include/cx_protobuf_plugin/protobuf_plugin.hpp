// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// Read the full text in the LICENSE.GPL file in the main directory.

/***************************************************************************
 *  protobuf_plugin.hpp
 *
 *  Created: October 13th, 2023
 *  Copyright  2023 Daniel Swoboda
 *  Copyright  2024 Tarik Viehmann <viehmann@kbsg.rwth-achen.de>
 ****************************************************************************/

#ifndef CX_PROTOBUF_PLUGIN__PROTOBUF_PLUGIN_HPP_
#define CX_PROTOBUF_PLUGIN__PROTOBUF_PLUGIN_HPP_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cx_plugin/clips_plugin.hpp"
#include "cx_protobuf_plugin/communicator.hpp"

namespace cx
{

class ProtobufPlugin : public ClipsPlugin
{
public:
  ProtobufPlugin();
  ~ProtobufPlugin();

  void initialize() override;

  bool clips_env_init(std::shared_ptr<clips::Environment> & env) override;
  bool clips_env_destroyed(std::shared_ptr<clips::Environment> & env) override;

private:
  std::vector<std::string> paths_;
  std::unordered_map<std::string, std::unique_ptr<protobuf_clips::ClipsProtobufCommunicator>>
    protobuf_communicator_;
  std::unique_ptr<rclcpp::Logger> logger_;

  std::string plugin_path_;
};

}  // namespace cx
#endif  // CX_PROTOBUF_PLUGIN__PROTOBUF_PLUGIN_HPP_
