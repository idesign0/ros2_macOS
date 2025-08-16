/*
 * Copyright (c) 2011, Dirk Thomas, TU Darmstadt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TU Darmstadt nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "roscpp_plugin_provider.h"

#include "nodelet_plugin_provider.h"
#include <rqt_gui_cpp/plugin.h>

#include <qt_gui_cpp/plugin_provider.h>
#include <rclcpp/rclcpp.hpp>

#include <pluginlib/class_list_macros.hpp>

#include <stdexcept>
#include <sys/types.h>

namespace rqt_gui_cpp {

RosCppPluginProvider::RosCppPluginProvider()
  : qt_gui_cpp::CompositePluginProvider()
  , rclcpp_initialized_(false)
{
  if (rclcpp::ok())
  {
    rclcpp_initialized_ = true;
  }

  init_rclcpp();
  QList<PluginProvider*> plugin_providers;
  plugin_providers.append(new NodeletPluginProvider("rqt_gui", "rqt_gui_cpp::Plugin"));
  set_plugin_providers(plugin_providers);
}

RosCppPluginProvider::~RosCppPluginProvider()
{
  if (rclcpp::ok())
  {
    rclcpp::shutdown();
  }
}

void* RosCppPluginProvider::load(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context)
{
  qDebug("RosCppPluginProvider::load(%s)", plugin_id.toStdString().c_str());
  init_rclcpp();
  return qt_gui_cpp::CompositePluginProvider::load(plugin_id, plugin_context);
}

qt_gui_cpp::Plugin* RosCppPluginProvider::load_plugin(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context)
{
  qDebug("RosCppPluginProvider::load_plugin(%s)", plugin_id.toStdString().c_str());
  init_rclcpp();
  return qt_gui_cpp::CompositePluginProvider::load_plugin(plugin_id, plugin_context);
}

void RosCppPluginProvider::init_rclcpp()
{
  // initialize ROS node once
  if (!rclcpp_initialized_)
  {
    int argc = 0;
    char** argv = 0;

    // Initialize any global resources needed by the middleware and the client library.
    // This will also parse command line arguments one day (as of Beta 1 they are not used).
    // You must call this before using any other part of the ROS system.
    // This should be called once per process.
    rclcpp::init(argc, argv);


    // Don't do this again in this process
    rclcpp_initialized_ = true;
  }
}

}

PLUGINLIB_EXPORT_CLASS(rqt_gui_cpp::RosCppPluginProvider, qt_gui_cpp::PluginProvider)
