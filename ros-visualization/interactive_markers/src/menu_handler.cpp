// Copyright (c) 2011, Willow Garage, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Author: David Gossow

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "interactive_markers/menu_handler.hpp"

// TODO(jacobperron): Remove this macro when it is available upstream
// See: https://github.com/ros2/rcutils/pull/112
#ifndef RCUTILS_ASSERT_MSG
#define RCUTILS_ASSERT_MSG(cond, ...) \
  do { \
    if (!(cond)) { \
      RCUTILS_LOG_FATAL( \
        "ASSERTION FAILED\n\tfile = %s\n\tline = %d\n\tcond = %s\n\tmessage = ", \
        __FILE__, __LINE__, #cond); \
      RCUTILS_LOG_FATAL(__VA_ARGS__); \
      RCUTILS_LOG_FATAL("\n"); \
      std::terminate(); \
    } \
  } while (false)
#endif

namespace interactive_markers
{

MenuHandler::MenuHandler()
: current_handle_(1)
{
}

MenuHandler::EntryHandle MenuHandler::insert(
  const std::string & title,
  const FeedbackCallback & feedback_cb)
{
  EntryHandle handle =
    doInsert(title, visualization_msgs::msg::MenuEntry::FEEDBACK, "", feedback_cb);
  top_level_handles_.push_back(handle);
  return handle;
}

MenuHandler::EntryHandle MenuHandler::insert(
  const std::string & title,
  const uint8_t command_type,
  const std::string & command)
{
  EntryHandle handle = doInsert(title, command_type, command, FeedbackCallback());
  top_level_handles_.push_back(handle);
  return handle;
}


MenuHandler::EntryHandle MenuHandler::insert(
  EntryHandle parent, const std::string & title,
  const FeedbackCallback & feedback_cb)
{
  std::unordered_map<EntryHandle, EntryContext>::iterator parent_context =
    entry_contexts_.find(parent);

  RCUTILS_ASSERT_MSG(
    parent_context != entry_contexts_.end(), "Parent menu entry %u not found.", parent);

  EntryHandle handle =
    doInsert(title, visualization_msgs::msg::MenuEntry::FEEDBACK, "", feedback_cb);
  parent_context->second.sub_entries.push_back(handle);
  return handle;
}


MenuHandler::EntryHandle MenuHandler::insert(
  EntryHandle parent, const std::string & title,
  const uint8_t command_type,
  const std::string & command)
{
  std::unordered_map<EntryHandle, EntryContext>::iterator parent_context =
    entry_contexts_.find(parent);

  RCUTILS_ASSERT_MSG(
    parent_context != entry_contexts_.end(), "Parent menu entry %u not found.", parent);

  EntryHandle handle = doInsert(title, command_type, command, FeedbackCallback());
  parent_context->second.sub_entries.push_back(handle);
  return handle;
}


bool MenuHandler::setVisible(EntryHandle handle, bool visible)
{
  std::unordered_map<EntryHandle, EntryContext>::iterator context =
    entry_contexts_.find(handle);

  if (context == entry_contexts_.end()) {
    return false;
  }

  context->second.visible = visible;
  return true;
}


bool MenuHandler::setCheckState(EntryHandle handle, CheckState check_state)
{
  std::unordered_map<EntryHandle, EntryContext>::iterator context =
    entry_contexts_.find(handle);

  if (context == entry_contexts_.end()) {
    return false;
  }

  context->second.check_state = check_state;
  return true;
}


bool MenuHandler::getCheckState(EntryHandle handle, CheckState & check_state) const
{
  std::unordered_map<EntryHandle, EntryContext>::const_iterator context =
    entry_contexts_.find(handle);

  if (context == entry_contexts_.end()) {
    check_state = NO_CHECKBOX;
    return false;
  }

  check_state = context->second.check_state;
  return true;
}


bool MenuHandler::apply(InteractiveMarkerServer & server, const std::string & marker_name)
{
  visualization_msgs::msg::InteractiveMarker int_marker;

  if (!server.get(marker_name, int_marker)) {
    // This marker has been deleted on the server, so forget it.
    managed_markers_.erase(marker_name);
    return false;
  }

  int_marker.menu_entries.clear();

  pushMenuEntries(top_level_handles_, int_marker.menu_entries, 0);

  server.insert(int_marker);
  server.setCallback(
    marker_name,
    std::bind(&MenuHandler::processFeedback, this, std::placeholders::_1),
    visualization_msgs::msg::InteractiveMarkerFeedback::MENU_SELECT);
  managed_markers_.insert(marker_name);
  return true;
}

bool MenuHandler::pushMenuEntries(
  std::vector<EntryHandle> & handles_in,
  std::vector<visualization_msgs::msg::MenuEntry> & entries_out,
  EntryHandle parent_handle)
{
  for (unsigned t = 0; t < handles_in.size(); t++) {
    EntryHandle handle = handles_in[t];
    std::unordered_map<EntryHandle, EntryContext>::iterator context_it =
      entry_contexts_.find(handle);

    if (context_it == entry_contexts_.end()) {
      RCUTILS_LOG_ERROR("Internal error: context handle not found! This is a bug in MenuHandler.");
      return false;
    }

    EntryContext & context = context_it->second;

    if (!context.visible) {
      continue;
    }

    entries_out.push_back(makeEntry(context, handle, parent_handle));
    if (false == pushMenuEntries(context.sub_entries, entries_out, handle)) {
      return false;
    }
  }
  return true;
}

bool MenuHandler::reApply(InteractiveMarkerServer & server)
{
  bool success = true;
  std::set<std::string>::iterator it = managed_markers_.begin();
  while (it != managed_markers_.end()) {
    // apply() may delete the entry "it" is pointing to, so
    // pre-compute the next iterator.
    std::set<std::string>::iterator next_it = it;
    next_it++;
    success = apply(server, *it) && success;
    it = next_it;
  }
  return success;
}

MenuHandler::EntryHandle MenuHandler::doInsert(
  const std::string & title,
  const uint8_t command_type,
  const std::string & command,
  const FeedbackCallback & feedback_cb)
{
  EntryHandle handle = current_handle_;
  current_handle_++;

  EntryContext context;
  context.title = title;
  context.command = command;
  context.command_type = command_type;
  context.visible = true;
  context.check_state = NO_CHECKBOX;
  context.feedback_cb = feedback_cb;

  entry_contexts_[handle] = context;
  return handle;
}

visualization_msgs::msg::MenuEntry MenuHandler::makeEntry(
  EntryContext & context,
  EntryHandle handle,
  EntryHandle parent_handle)
{
  visualization_msgs::msg::MenuEntry menu_entry;

  switch (context.check_state) {
    case NO_CHECKBOX:
      menu_entry.title = context.title;
      break;
    case CHECKED:
      menu_entry.title = "[x] " + context.title;
      break;
    case UNCHECKED:
      menu_entry.title = "[ ] " + context.title;
      break;
  }

  menu_entry.command = context.command;
  menu_entry.command_type = context.command_type;
  menu_entry.id = handle;
  menu_entry.parent_id = parent_handle;

  return menu_entry;
}


void MenuHandler::processFeedback(
  const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr & feedback)
{
  std::unordered_map<EntryHandle, EntryContext>::iterator context =
    entry_contexts_.find( (EntryHandle) feedback->menu_entry_id);

  if (context != entry_contexts_.end() && context->second.feedback_cb) {
    context->second.feedback_cb(feedback);
  }
}

bool MenuHandler::getTitle(EntryHandle handle, std::string & title) const
{
  std::unordered_map<EntryHandle, EntryContext>::const_iterator context =
    entry_contexts_.find(handle);

  if (context == entry_contexts_.end()) {
    return false;
  }

  title = context->second.title;
  return true;
}

}  // namespace interactive_markers
