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

#include "nodelet_plugin_provider.h"

#include "roscpp_plugin_provider.h"

#include <stdexcept>

namespace rqt_gui_cpp {

NodeletPluginProvider::NodeletPluginProvider(const QString& export_tag, const QString& base_class_type)
  : qt_gui_cpp::RosPluginlibPluginProvider<rqt_gui_cpp::Plugin>(export_tag, base_class_type)
  , loader_initialized_(false)
  , ros_spin_thread_(0)
{}

NodeletPluginProvider::~NodeletPluginProvider()
{
  if (ros_spin_thread_ != 0)
  {
    ros_spin_thread_->abort = true;
    ros_spin_thread_->exec_.remove_node(node_);
    ros_spin_thread_->wait();
    ros_spin_thread_->deleteLater();
    ros_spin_thread_ = 0;
  }
}

void NodeletPluginProvider::unload(void* instance)
{
  if (!instances_.contains(instance))
  {
    qCritical("rqt_gui_cpp::NodeletPluginProvider::unload() instance not found");
    return;
  }

  QString nodelet_name = instances_[instance];

  qt_gui_cpp::RosPluginlibPluginProvider<rqt_gui_cpp::Plugin>::unload(instance);
}

void NodeletPluginProvider::init_loader()
{

  if (!loader_initialized_)
  {
    loader_initialized_ = true;

    // spawn ros spin thread
    if (ros_spin_thread_ == 0)
    {
      ros_spin_thread_ = new RosSpinThread(this);
      ros_spin_thread_->start();
    }

    std::stringstream name;
    name << "rqt_gui_cpp_node_";
    name << getpid();
    // Initialize a node for execution to be shared by cpp plugins
    node_ = rclcpp::Node::make_shared(name.str().c_str());
    // Add our node to the executor for execution
    if (ros_spin_thread_)
    {
      ros_spin_thread_->exec_.add_node(node_);
    }
    else
    {
      qWarning("rqt_gui_cpp::NodeletPluginProvider.init_loader: ros_spin_thread_ not initialized");
    }
  }

}

std::shared_ptr<Plugin> NodeletPluginProvider::create_plugin(const std::string& lookup_name, qt_gui_cpp::PluginContext* plugin_context)
{
  init_loader();

  std::string nodelet_name = lookup_name + "_" + QString::number(plugin_context->serialNumber()).toStdString();
  instance_.reset();

  instance_ = qt_gui_cpp::RosPluginlibPluginProvider<rqt_gui_cpp::Plugin>::create_plugin(lookup_name);
  instance_->passInNode(node_);
  instances_[&*instance_] = nodelet_name.c_str();

  std::shared_ptr<rqt_gui_cpp::Plugin> instance = instance_;
  instance_.reset();
  return instance;
}


void NodeletPluginProvider::init_plugin(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context, qt_gui_cpp::Plugin* plugin)
{
  qDebug("rqt_gui_cpp::NodeletPluginProvider::init_plugin()");
  init_loader();

  rqt_gui_cpp::Plugin* rqt_plugin = dynamic_cast<rqt_gui_cpp::Plugin*>(plugin);
  if (!rqt_plugin)
  {
    throw std::runtime_error("plugin is not a rqt_plugin::Plugin");
  }

  qt_gui_cpp::RosPluginlibPluginProvider<rqt_gui_cpp::Plugin>::init_plugin(plugin_id, plugin_context, plugin);
}

NodeletPluginProvider::RosSpinThread::RosSpinThread(QObject* parent)
  : QThread(parent)
  , abort(false)
{}

NodeletPluginProvider::RosSpinThread::~RosSpinThread()
{}

void NodeletPluginProvider::RosSpinThread::run()
{
  while (!abort)
  {
    // Spin the executor
    exec_.spin_once();
  }
}


}
