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
 *  communicator.cpp - protobuf network communication for CLIPS
 *
 *  Created: Tue Apr 16 13:51:14 2013
 *  Copyright  2013  Tim Niemueller [www.niemueller.de]
 *             2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *             2024  Tarik Viehmann <viehmann@kbsg.rwth-aachen.de>
 ****************************************************************************/

#include "cx_protobuf_plugin/communicator.hpp"

#include <filesystem>
#include <string>

#include "cx_utils/format.hpp"
#include "google/protobuf/descriptor.h"
#include "protobuf_comm/client.h"
#include "protobuf_comm/peer.h"
#include "protobuf_comm/server.h"

using boost::placeholders::_1;
using boost::placeholders::_2;
using boost::placeholders::_3;
using boost::placeholders::_4;
namespace pb = google::protobuf;

namespace protobuf_clips
{

/** @class ClipsProtobufCommunicator <protobuf_clips/communicator.h>
 * CLIPS protobuf integration class.
 * This class adds functionality related to protobuf to a given CLIPS
 * environment. It supports the creation of communication channels
 * through protobuf_comm. An instance maintains its own message register
 * shared among server, peer, and clients.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param env CLIPS environment to which to provide the protobuf functionality
 * @param env_mutex mutex to lock when operating on the CLIPS environment.
 */
ClipsProtobufCommunicator::ClipsProtobufCommunicator(
  clips::Environment * env, std::mutex & env_mutex, rclcpp_lifecycle::LifecycleNode::WeakPtr parent)
: clips_(env), clips_mutex_(env_mutex), next_client_id_(0), parent_(parent)
{
  message_register_ = std::make_unique<protobuf_comm::MessageRegister>();
  setup_clips();
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger("ClipsProtobufCommunicator"));
}

/** Constructor.
 * @param env CLIPS environment to which to provide the protobuf functionality
 * @param env_mutex mutex to lock when operating on the CLIPS environment.
 * @param proto_path proto path passed to a newly instantiated message register
 */
ClipsProtobufCommunicator::ClipsProtobufCommunicator(
  clips::Environment * env, std::mutex & env_mutex, std::vector<std::string> & proto_path,
  rclcpp_lifecycle::LifecycleNode::WeakPtr parent)
: clips_(env), clips_mutex_(env_mutex), next_client_id_(0), parent_(parent)
{
  message_register_ = std::make_unique<protobuf_comm::MessageRegister>(proto_path);
  setup_clips();
}

/** Destructor. */
ClipsProtobufCommunicator::~ClipsProtobufCommunicator()
{
  {
    for (auto f : functions_) {
      clips::RemoveUDF(clips_, f.c_str());
    }
    functions_.clear();
  }
  clients_.clear();
  peers_.clear();
}

/** Setup CLIPS environment. */
void ClipsProtobufCommunicator::setup_clips()
{
  // std::lock_guard<std::mutex> lock(clips_mutex_);
  using namespace clips;  // NOLINT
  std::string function_name;
  function_name = "pb-register-type";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "b", 1, 1, ";sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue full_name;
      if (!clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &full_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-register-type: unexpected types, expected addr");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_register_type(full_name.lexemeValue->contents);
    },
    "clips_pb_register_type", this);

  function_name = "pb-field-names";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "m", 1, 1, ";e",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr;
      if (!clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-names: unexpected types, expected addr");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_names(msgptr.externalAddressValue->contents);
    },
    "clips_pb_field_names", this);

  function_name = "pb-field-type";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "y", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-type: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_type(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_field_type", this);

  function_name = "pb-has-field";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "b", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-has-field: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_has_field(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_has_field", this);

  function_name = "pb-field-label";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "y", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-label: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_label(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_field_label", this);

  function_name = "pb-field-value";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "*", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-value: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_value(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_field_value", this);

  function_name = "pb-field-list";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "ym", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-list: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_list(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_field_list", this);

  function_name = "pb-field-is-list";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "b", 2, 2, ";e;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-field-is-list: unexpected types, expected addr;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_field_is_list(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents);
    },
    "clips_pb_field_is_list", this);
  functions_.push_back("pb-field-is-list");

  function_name = "pb-create";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "e", 1, 1, ";sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue full_name;
      if (!clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &full_name)) {
        RCLCPP_ERROR(*instance->logger_, "pb-create: unexpected types, expected lex");
        clips::UDFThrowError(udfc);
        return;
      }
      *out = instance->clips_pb_create(full_name.lexemeValue->contents);
    },
    "clips_pb_create", this);

  function_name = "pb-destroy";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 1, 1, ";e",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr;
      if (!clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr)) {
        RCLCPP_ERROR(*instance->logger_, "pb-destroy: unexpected types, expected addr");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_destroy(msgptr.externalAddressValue->contents);
    },
    "clips_pb_destroy", this);

  function_name = "pb-set-field";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 3, 3, ";e;sy;*",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name, value;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name) ||
        !clips::UDFNthArgument(udfc, 3, ANY_TYPE_BITS, &value)) {
        RCLCPP_ERROR(*instance->logger_, "pb-set-field: unexpected types, expected addr;lex;*");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_set_field(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents, value);
    },
    "clips_pb_set_field", this);

  function_name = "pb-add-list";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 3, 3, ";e;sy;*",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr, field_name, value;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field_name) ||
        !clips::UDFNthArgument(udfc, 3, ANY_TYPE_BITS, &value)) {
        RCLCPP_ERROR(*instance->logger_, "pb-add-list: unexpected types, expected addr;lex;*");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_add_list(
        msgptr.externalAddressValue->contents, field_name.lexemeValue->contents, value);
    },
    "clips_pb_add_list", this);

  function_name = "pb-send";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 2, 2, ";l;e",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue client_id, msgptr;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &client_id) ||
        !clips::UDFNthArgument(udfc, 2, clips::EXTERNAL_ADDRESS_BIT, &msgptr)) {
        RCLCPP_ERROR(*instance->logger_, "pb-send: unexpected types, expected int;addr");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_send(
        client_id.integerValue->contents, msgptr.externalAddressValue->contents);
    },
    "clips_pb_send", this);

  function_name = "pb-tostring";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "sy", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue msgptr;
      if (!clips::UDFNthArgument(udfc, 1, clips::EXTERNAL_ADDRESS_BIT, &msgptr)) {
        RCLCPP_ERROR(*instance->logger_, "pb-tostring: unexpected types, expected addr");
        out->lexemeValue = clips::CreateSymbol(env, "INVALID-MESSAGE");
        clips::UDFThrowError(udfc);
        return;
      }
      std::string res = instance->clips_pb_tostring(msgptr.externalAddressValue->contents);
      out->lexemeValue = clips::CreateString(env, res.c_str());
    },
    "clips_pb_tostring", this);

  function_name = "pb-server-enable";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 1, 1, ";l",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue port;
      if (!clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &port)) {
        RCLCPP_ERROR(*instance->logger_, "pb-server-enable: unexpected types, expected int");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->enable_server(port.integerValue->contents);
    },
    "enable_server", this);

  function_name = "pb-server-disable";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 0, 0, "",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      instance->disable_server();
    },
    "disable_server", this);

  function_name = "pb-peer-create";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "l", 2, 2, ";s;l",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue address, port;
      if (
        !clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &address) ||
        !clips::UDFNthArgument(udfc, 2, clips::INTEGER_BIT, &port)) {
        RCLCPP_ERROR(*instance->logger_, "pb-peer-create: unexpected types, expected lex;int");
        clips::UDFThrowError(udfc);
        return;
      }
      out->integerValue = clips::CreateInteger(
        env,
        instance->clips_pb_peer_create(address.lexemeValue->contents, port.integerValue->contents));
    },
    "clips_pb_peer_create", this);

  function_name = "pb-peer-create-local";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "l", 3, 3, ";sy;l;l",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue address, send_port, recv_port;
      if (
        !clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &address) ||
        !clips::UDFNthArgument(udfc, 2, clips::INTEGER_BIT, &send_port) ||
        !clips::UDFNthArgument(udfc, 3, clips::INTEGER_BIT, &recv_port)) {
        RCLCPP_ERROR(
          *instance->logger_, "pb-peer-create-local: unexpected types, expected lex;int;int");
        clips::UDFThrowError(udfc);
        return;
      }
      out->integerValue = clips::CreateInteger(
        env, instance->clips_pb_peer_create_local(
               address.lexemeValue->contents, send_port.integerValue->contents,
               recv_port.integerValue->contents));
    },
    "clips_pb_peer_create_local", this);

  function_name = "pb-peer-create-crypto";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "l", 4, 4, ";sy;l;sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue address, port, crypto_key, cipher;
      if (
        !clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &address) ||
        !clips::UDFNthArgument(udfc, 2, clips::INTEGER_BIT, &port) ||
        !clips::UDFNthArgument(udfc, 4, LEXEME_BITS, &crypto_key) ||
        !clips::UDFNthArgument(udfc, 5, LEXEME_BITS, &cipher)) {
        RCLCPP_ERROR(
          *instance->logger_,
          "pb-peer-create-crypto: unexpected types, expected "
          "lex;int;lex,lex");
        clips::UDFThrowError(udfc);
        return;
      }
      out->integerValue = clips::CreateInteger(
        env, instance->clips_pb_peer_create_crypto(
               address.lexemeValue->contents, port.integerValue->contents,
               crypto_key.lexemeValue->contents, cipher.lexemeValue->contents));
    },
    "clips_pb_peer_create_crypto", this);

  function_name = "pb-peer-create-local-crypto";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "l", 5, 5, ";sy;l;l;sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue address, send_port, recv_port, crypto_key, cipher;
      if (
        !clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &address) ||
        !clips::UDFNthArgument(udfc, 2, clips::INTEGER_BIT, &send_port) ||
        !clips::UDFNthArgument(udfc, 3, clips::INTEGER_BIT, &recv_port) ||
        !clips::UDFNthArgument(udfc, 4, LEXEME_BITS, &crypto_key) ||
        !clips::UDFNthArgument(udfc, 5, LEXEME_BITS, &cipher)) {
        RCLCPP_ERROR(
          *instance->logger_,
          "pb-peer-create-local-crypto: unexpected types, "
          "expected lex;int;int,lex,lex");
        clips::UDFThrowError(udfc);
        return;
      }
      out->integerValue = clips::CreateInteger(
        env, instance->clips_pb_peer_create_local_crypto(
               address.lexemeValue->contents, send_port.integerValue->contents,
               recv_port.integerValue->contents, crypto_key.lexemeValue->contents,
               cipher.lexemeValue->contents));
    },
    "clips_pb_peer_create_local_crypto", this);

  function_name = "pb-peer-destroy";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 1, 1, ";l",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue peer_id;
      if (!clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &peer_id)) {
        RCLCPP_ERROR(*instance->logger_, "pb-peer-destroy: unexpected types, expected int");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_peer_destroy(peer_id.integerValue->contents);
    },
    "clips_pb_peer_destroy", this);

  function_name = "pb-peer-setup-crypto";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 3, 3, ";l;sy;sy",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue peer_id, crypto_key, cipher;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &peer_id) ||
        !clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &crypto_key) ||
        !clips::UDFNthArgument(udfc, 3, LEXEME_BITS, &cipher)) {
        RCLCPP_ERROR(
          *instance->logger_, "pb-peer-setup-crypto: unexpected types, expected int;lex;lex");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_peer_setup_crypto(
        peer_id.integerValue->contents, crypto_key.lexemeValue->contents,
        cipher.lexemeValue->contents);
    },
    "clips_pb_peer_setup_crypto", this);

  function_name = "pb-broadcast";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 2, 2, ";l;e",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue peer_id, msgptr;
      if (
        !clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &peer_id) ||
        !clips::UDFNthArgument(udfc, 2, clips::EXTERNAL_ADDRESS_BIT, &msgptr)) {
        RCLCPP_ERROR(*instance->logger_, "pb-broadcast: unexpected types, expected int;addr");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_broadcast(
        peer_id.integerValue->contents, msgptr.externalAddressValue->contents);
    },
    "clips_pb_broadcast", this);

  function_name = "pb-connect";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "l", 2, 2, ";sy;l",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue host, port;
      if (
        !clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &host) ||
        !clips::UDFNthArgument(udfc, 2, clips::INTEGER_BIT, &port)) {
        RCLCPP_ERROR(*instance->logger_, "pb-connect: unexpected types, expected lex;int");
        clips::UDFThrowError(udfc);
        return;
      }
      out->integerValue = clips::CreateInteger(
        env,
        instance->clips_pb_client_connect(host.lexemeValue->contents, port.integerValue->contents));
    },
    "clips_pb_client_connect", this);
  functions_.push_back("pb-connect");

  function_name = "pb-disconnect";
  functions_.push_back(function_name);
  clips::AddUDF(
    clips_, function_name.c_str(), "v", 1, 1, ";l",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ClipsProtobufCommunicator * instance =
        static_cast<ClipsProtobufCommunicator *>(udfc->context);
      clips::UDFValue peer_id;
      if (!clips::UDFNthArgument(udfc, 1, clips::INTEGER_BIT, &peer_id)) {
        RCLCPP_ERROR(*instance->logger_, "pb-disconnect: unexpected types, expected int");
        clips::UDFThrowError(udfc);
        return;
      }
      instance->clips_pb_disconnect(peer_id.integerValue->contents);
    },
    "clips_pb_disconnect", this);
}

/** Enable protobuf stream server.
 * @param port TCP port to listen on for connections
 */
void ClipsProtobufCommunicator::enable_server(int port)
{
  if ((port > 0) && !server_) {
    server_ = std::make_unique<protobuf_comm::ProtobufStreamServer>(port, message_register_.get());
    server_->signal_connected().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_server_client_connected, this, _1, _2));
    server_->signal_disconnected().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_server_client_disconnected, this, _1, _2));
    server_->signal_received().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_server_client_msg, this, _1, _2, _3, _4));
    server_->signal_receive_failed().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_server_client_fail, this, _1, _2, _3, _4));
  }
}

/** Disable protobuf stream server. */
void ClipsProtobufCommunicator::disable_server()
{
  std::thread([this]() {
    std::scoped_lock lock{clips_mutex_, map_mutex_};
    server_.reset();
  }).detach();
}

/** Enable protobuf peer.
 * @param address IP address to send messages to
 * @param send_port UDP port to send messages to
 * @param recv_port UDP port to receive messages on, 0 to use the same as the @p
 * send_port
 * @param crypto_key encryption key
 * @param cipher cipher suite, see BufferEncryptor for supported types
 * @return peer identifier
 */
int64_t ClipsProtobufCommunicator::clips_pb_peer_create_local_crypto(
  std::string address, int send_port, int recv_port, std::string crypto_key, std::string cipher)
{
  if (recv_port <= 0) recv_port = send_port;

  if (send_port > 0) {
    protobuf_comm::ProtobufBroadcastPeer * peer;

    int64_t peer_id;
    {
      std::lock_guard<std::mutex> lock(map_mutex_);
      peer_id = ++next_client_id_;
      peers_[peer_id] = std::make_unique<protobuf_comm::ProtobufBroadcastPeer>(
        address, send_port, recv_port, message_register_.get(), crypto_key, cipher);
      peer = peers_[peer_id].get();
    }

    peer->signal_received().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_peer_msg, this, peer_id, _1, _2, _3, _4));
    peer->signal_recv_error().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_peer_recv_error, this, peer_id, _1, _2));
    peer->signal_send_error().connect(
      boost::bind(&ClipsProtobufCommunicator::handle_peer_send_error, this, peer_id, _1));

    return peer_id;
  } else {
    return 0;
  }
}

/** Enable protobuf peer.
 * @param address IP address to send messages to
 * @param port UDP port to send and receive messages
 * @param crypto_key encryption key
 * @param cipher cipher suite, see BufferEncryptor for supported types
 * @return peer identifier
 */
int64_t ClipsProtobufCommunicator::clips_pb_peer_create_crypto(
  std::string address, int port, std::string crypto_key, std::string cipher)
{
  return clips_pb_peer_create_local_crypto(address, port, port, crypto_key, cipher);
}

/** Enable protobuf peer.
 * @param address IP address to send messages to
 * @param port UDP port to send and receive messages
 * @return peer identifier
 */
int64_t ClipsProtobufCommunicator::clips_pb_peer_create(std::string address, int port)
{
  return clips_pb_peer_create_local_crypto(address, port, port);
}

/** Enable protobuf peer.
 * @param address IP address to send messages to
 * @param send_port UDP port to send messages to
 * @param recv_port UDP port to receive messages on, 0 to use the same as the @p
 * send_port
 * @return peer identifier
 */
int64_t ClipsProtobufCommunicator::clips_pb_peer_create_local(
  std::string address, int send_port, int recv_port)
{
  return clips_pb_peer_create_local_crypto(address, send_port, recv_port);
}

/** Disable peer.
 * @param peer_id ID of the peer to destroy
 */
void ClipsProtobufCommunicator::clips_pb_peer_destroy(int64_t peer_id)
{
  if (peers_.find(peer_id) != peers_.end()) {
    peers_.erase(peer_id);
  }
}

/** Setup crypto for peer.
 * @param peer_id ID of the peer to destroy
 * @param crypto_key encryption key
 * @param cipher cipher suite, see BufferEncryptor for supported types
 */
void ClipsProtobufCommunicator::clips_pb_peer_setup_crypto(
  int64_t peer_id, std::string crypto_key, std::string cipher)
{
  if (peers_.find(peer_id) != peers_.end()) {
    peers_[peer_id]->setup_crypto(crypto_key, cipher);
  }
}

/** Register a new message type.
 * @param full_name full name of type to register
 * @return true if the type was successfully registered, false otherwise
 */
clips::UDFValue ClipsProtobufCommunicator::clips_pb_register_type(std::string full_name)
{
  clips::UDFValue res;
  try {
    message_register_->add_message_type(full_name);
    res.lexemeValue = clips::CreateBoolean(clips_, true);
    return res;
  } catch (std::runtime_error & e) {
    RCLCPP_ERROR(
      *logger_,
      cx::format("CLIPS-Protobuf: Registering type {} failed: {}", full_name, e.what()).c_str());
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_create(std::string full_name)
{
  clips::UDFValue res;
  try {
    std::shared_ptr<google::protobuf::Message> m = message_register_->new_message_for(full_name);
    messages_[m.get()] = m;
    res.externalAddressValue = clips::CreateCExternalAddress(clips_, m.get());
    return res;
  } catch (std::runtime_error & e) {
    RCLCPP_WARN(
      *logger_,
      cx::format("CLIPS-Protobuf: Cannot create message of type {}: {}", full_name, e.what())
        .c_str());
    res.externalAddressValue = clips::CreateCExternalAddress(clips_, nullptr);
    return res;
  }
}

void ClipsProtobufCommunicator::clips_pb_destroy(void * msgptr) { messages_.erase(msgptr); }

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_names(void * msgptr)
{
  clips::UDFValue field_names;
  field_names.begin = 0;
  field_names.range = -1;
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    field_names.lexemeValue = clips::CreateBoolean(clips_, false);
    return field_names;
  }
  const pb::Descriptor * desc = msg->GetDescriptor();
  const int field_count = desc->field_count();
  std::string out_str = "";
  for (int i = 0; i < field_count; ++i) {
    out_str += " " + desc->field(i)->name();
  }
  field_names.multifieldValue = clips::StringToMultifield(clips_, out_str.c_str());
  return field_names;
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_type(
  void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    res.lexemeValue = clips::CreateSymbol(clips_, "DOES-NOT-EXIST");
    return res;
  }

  switch (field->type()) {
    case pb::FieldDescriptor::TYPE_DOUBLE:
      res.lexemeValue = clips::CreateSymbol(clips_, "DOUBLE");
      break;
    case pb::FieldDescriptor::TYPE_FLOAT:
      res.lexemeValue = clips::CreateSymbol(clips_, "FLOAT");
      break;
    case pb::FieldDescriptor::TYPE_INT64:
      res.lexemeValue = clips::CreateSymbol(clips_, "INT64");
      break;
    case pb::FieldDescriptor::TYPE_UINT64:
      res.lexemeValue = clips::CreateSymbol(clips_, "UINT64");
      break;
    case pb::FieldDescriptor::TYPE_INT32:
      res.lexemeValue = clips::CreateSymbol(clips_, "INT32");
      break;
    case pb::FieldDescriptor::TYPE_FIXED64:
      res.lexemeValue = clips::CreateSymbol(clips_, "FIXED64");
      break;
    case pb::FieldDescriptor::TYPE_FIXED32:
      res.lexemeValue = clips::CreateSymbol(clips_, "FIXED32");
      break;
    case pb::FieldDescriptor::TYPE_BOOL:
      res.lexemeValue = clips::CreateSymbol(clips_, "BOOL");
      break;
    case pb::FieldDescriptor::TYPE_STRING:
      res.lexemeValue = clips::CreateSymbol(clips_, "STRING");
      break;
    case pb::FieldDescriptor::TYPE_MESSAGE:
      res.lexemeValue = clips::CreateSymbol(clips_, "MESSAGE");
      break;
    case pb::FieldDescriptor::TYPE_BYTES:
      res.lexemeValue = clips::CreateSymbol(clips_, "BYTES");
      break;
    case pb::FieldDescriptor::TYPE_UINT32:
      res.lexemeValue = clips::CreateSymbol(clips_, "UINT32");
      break;
    case pb::FieldDescriptor::TYPE_ENUM:
      res.lexemeValue = clips::CreateSymbol(clips_, "ENUM");
      break;
    case pb::FieldDescriptor::TYPE_SFIXED32:
      res.lexemeValue = clips::CreateSymbol(clips_, "SFIXED32");
      break;
    case pb::FieldDescriptor::TYPE_SFIXED64:
      res.lexemeValue = clips::CreateSymbol(clips_, "SFIXED64");
      break;
    case pb::FieldDescriptor::TYPE_SINT32:
      res.lexemeValue = clips::CreateSymbol(clips_, "SINT32");
      break;
    case pb::FieldDescriptor::TYPE_SINT64:
      res.lexemeValue = clips::CreateSymbol(clips_, "SINT64");
      break;
    default:
      res.lexemeValue = clips::CreateSymbol(clips_, "UNKNOWN");
      break;
  }
  return res;
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_has_field(void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }

  const pb::Reflection * refl = msg->GetReflection();

  if (field->is_repeated()) {
    res.lexemeValue = clips::CreateBoolean(clips_, (refl->FieldSize(*msg, field) > 0));
  } else {
    res.lexemeValue = clips::CreateBoolean(clips_, refl->HasField(*msg, field));
  }
  return res;
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_label(
  void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    res.lexemeValue = clips::CreateSymbol(clips_, "DOES-NOT-EXIST");
    return res;
  }
  switch (field->label()) {
    case pb::FieldDescriptor::LABEL_OPTIONAL:
      res.lexemeValue = clips::CreateSymbol(clips_, "OPTIONAL");
      return res;
    case pb::FieldDescriptor::LABEL_REQUIRED:
      res.lexemeValue = clips::CreateSymbol(clips_, "REQUIRED");
      return res;
    case pb::FieldDescriptor::LABEL_REPEATED:
      res.lexemeValue = clips::CreateSymbol(clips_, "REPEATED");
      return res;
    default:
      res.lexemeValue = clips::CreateSymbol(clips_, "UNKNOWN");
      return res;
  }
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_value(
  void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    RCLCPP_WARN(
      *logger_,
      cx::format("CLIPS-Protobuf: Field {} of {} does not exist", field_name, msg->GetTypeName())
        .c_str());
    res.lexemeValue = clips::CreateSymbol(clips_, "DOES-NOT-EXIST");
    return res;
  }
  const pb::Reflection * refl = msg->GetReflection();
  if (field->type() != pb::FieldDescriptor::TYPE_MESSAGE && !refl->HasField(*msg, field)) {
    RCLCPP_WARN(
      *logger_,
      cx::format("CLIPS-Protobuf: Field {} of {} not set", field_name, msg->GetTypeName()).c_str());
    res.lexemeValue = clips::CreateSymbol(clips_, "NOT-SET");
    return res;
  }
  switch (field->type()) {
    case pb::FieldDescriptor::TYPE_DOUBLE:
      res.floatValue = clips::CreateFloat(clips_, refl->GetDouble(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_FLOAT:
      res.floatValue = clips::CreateFloat(clips_, refl->GetFloat(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_INT64:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt64(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_UINT64:
      res.integerValue = clips::CreateInteger(clips_, (int64_t)refl->GetUInt64(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_INT32:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt32(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_FIXED64:
      res.integerValue = clips::CreateInteger(clips_, (int64_t)refl->GetUInt64(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_FIXED32:
      res.integerValue = clips::CreateInteger(clips_, refl->GetUInt32(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_BOOL:
      res.lexemeValue = clips::CreateBoolean(clips_, refl->GetBool(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_BYTES:
      res.lexemeValue = clips::CreateString(clips_, "BYTES");
      return res;
    case pb::FieldDescriptor::TYPE_STRING:
      res.lexemeValue = clips::CreateString(clips_, refl->GetString(*msg, field).c_str());
      return res;
    case pb::FieldDescriptor::TYPE_MESSAGE: {
      const google::protobuf::Message & mfield = refl->GetMessage(*msg, field);
      google::protobuf::Message * mcopy = mfield.New();
      mcopy->CopyFrom(mfield);

      std::shared_ptr<google::protobuf::Message> m(mcopy);
      messages_[m.get()] = m;
      res.externalAddressValue = clips::CreateCExternalAddress(clips_, m.get());
      return res;
    }
    case pb::FieldDescriptor::TYPE_UINT32:
      res.integerValue = clips::CreateInteger(clips_, refl->GetUInt32(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_ENUM:
      res.lexemeValue = clips::CreateSymbol(clips_, refl->GetEnum(*msg, field)->name().c_str());
      return res;
    case pb::FieldDescriptor::TYPE_SFIXED32:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt32(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_SFIXED64:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt64(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_SINT32:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt32(*msg, field));
      return res;
    case pb::FieldDescriptor::TYPE_SINT64:
      res.integerValue = clips::CreateInteger(clips_, refl->GetInt64(*msg, field));
      return res;
    default:
      throw std::logic_error("Unknown protobuf field type encountered");
  }
  return res;
}

void ClipsProtobufCommunicator::clips_pb_set_field(
  void * msgptr, std::string field_name, clips::UDFValue value)
{
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    return;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    RCLCPP_WARN(
      *logger_, cx::format("CLIPS-Protobuf: Could not find field {}", field_name).c_str());
    return;
  }
  const pb::Reflection * refl = msg->GetReflection();

  try {
    switch (field->type()) {
      case pb::FieldDescriptor::TYPE_DOUBLE:
        refl->SetDouble(msg.get(), field, value.floatValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_FLOAT:
        refl->SetFloat(msg.get(), field, value.floatValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_SFIXED64:
      case pb::FieldDescriptor::TYPE_SINT64:
      case pb::FieldDescriptor::TYPE_INT64:
        refl->SetInt64(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_FIXED64:
      case pb::FieldDescriptor::TYPE_UINT64:
        refl->SetUInt64(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_SFIXED32:
      case pb::FieldDescriptor::TYPE_SINT32:
      case pb::FieldDescriptor::TYPE_INT32:
        refl->SetInt32(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_BOOL:
        refl->SetBool(msg.get(), field, std::strcmp(value.lexemeValue->contents, "TRUE") == 0);
        break;
      case pb::FieldDescriptor::TYPE_STRING:
        refl->SetString(msg.get(), field, value.lexemeValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_MESSAGE: {
        auto sub_msg = messages_[value.externalAddressValue->contents];
        if (!sub_msg) {
          messages_.erase(msgptr);
          return;
        }
        pb::Message * mut_msg = refl->MutableMessage(msg.get(), field);
        mut_msg->CopyFrom(*(sub_msg.get()));
      } break;
      case pb::FieldDescriptor::TYPE_BYTES:
        break;
      case pb::FieldDescriptor::TYPE_FIXED32:
      case pb::FieldDescriptor::TYPE_UINT32:
        refl->SetUInt32(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_ENUM: {
        const pb::EnumDescriptor * enumdesc = field->enum_type();
        const pb::EnumValueDescriptor * enumval =
          enumdesc->FindValueByName(value.lexemeValue->contents);
        if (enumval) {
          refl->SetEnum(msg.get(), field, enumval);
        } else {
          RCLCPP_WARN(
            *logger_, cx::format(
                        "CLIPS-Protobuf: {}: cannot set invalid enum "
                        "value '{}' on '{}'",
                        msg->GetTypeName(), value.lexemeValue->contents, field_name)
                        .c_str());
        }
      } break;
      default:
        throw std::logic_error("Unknown protobuf field type encountered");
    }
  } catch (std::logic_error & e) {
    RCLCPP_WARN(
      *logger_,
      cx::format(
        "CLIPS-Protobuf: Failed to set field {} of {}: {} (type {}, as "
        "string {})",
        field_name, msg->GetTypeName(), e.what(), value.lexemeValue->header.type, to_string(value))
        .c_str());
  }
}

void ClipsProtobufCommunicator::clips_pb_add_list(
  void * msgptr, std::string field_name, clips::UDFValue value)
{
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    return;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    RCLCPP_WARN(
      *logger_, cx::format("CLIPS-Protobuf: Could not find field {}", field_name).c_str());
    return;
  }
  const pb::Reflection * refl = msg->GetReflection();

  try {
    switch (field->type()) {
      case pb::FieldDescriptor::TYPE_DOUBLE:
        refl->AddDouble(msg.get(), field, value.floatValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_FLOAT:
        refl->AddFloat(msg.get(), field, value.floatValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_SFIXED64:
      case pb::FieldDescriptor::TYPE_SINT64:
      case pb::FieldDescriptor::TYPE_INT64:
        refl->AddInt64(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_FIXED64:
      case pb::FieldDescriptor::TYPE_UINT64:
        refl->AddUInt64(msg.get(), field, (int64_t)value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_SFIXED32:
      case pb::FieldDescriptor::TYPE_SINT32:
      case pb::FieldDescriptor::TYPE_INT32:
        refl->AddInt32(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_BOOL:
        refl->AddBool(msg.get(), field, std::strcmp(value.lexemeValue->contents, "TRUE") == 0);
        break;
      case pb::FieldDescriptor::TYPE_STRING:
        refl->AddString(msg.get(), field, value.lexemeValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_MESSAGE: {
        std::shared_ptr<google::protobuf::Message> mfrom =
          messages_[value.externalAddressValue->contents];
        if (!mfrom) {
          messages_.erase(value.externalAddressValue->contents);
          return;
        }
        pb::Message * new_msg = refl->AddMessage(msg.get(), field);
        new_msg->CopyFrom(*mfrom);
      } break;
      case pb::FieldDescriptor::TYPE_BYTES:
        throw std::logic_error("Unsupported type BYTES");
      case pb::FieldDescriptor::TYPE_FIXED32:
      case pb::FieldDescriptor::TYPE_UINT32:
        refl->AddUInt32(msg.get(), field, value.integerValue->contents);
        break;
      case pb::FieldDescriptor::TYPE_ENUM: {
        const pb::EnumDescriptor * enumdesc = field->enum_type();
        const pb::EnumValueDescriptor * enumval =
          enumdesc->FindValueByName(value.lexemeValue->contents);
        if (enumval) refl->AddEnum(msg.get(), field, enumval);
      } break;
      default:
        throw std::logic_error("Unknown protobuf field type encountered");
    }
  } catch (std::logic_error & e) {
    RCLCPP_WARN(
      *logger_, cx::format(
                  "CLIPS-Protobuf: Failed to add field {} of {}: {}", field_name,
                  msg->GetTypeName().c_str(), e.what())
                  .c_str());
  }
}

int64_t ClipsProtobufCommunicator::clips_pb_client_connect(std::string host, int port)
{
  if (port <= 0) return false;

  protobuf_comm::ProtobufStreamClient * client;

  int64_t client_id;
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    client_id = ++next_client_id_;
    clients_[client_id] =
      std::make_unique<protobuf_comm::ProtobufStreamClient>(message_register_.get());
    client = clients_[client_id].get();
  }

  client->signal_connected().connect(
    boost::bind(&ClipsProtobufCommunicator::handle_client_connected, this, client_id));
  client->signal_disconnected().connect(boost::bind(
    &ClipsProtobufCommunicator::handle_client_disconnected, this, client_id,
    boost::asio::placeholders::error));
  client->signal_received().connect(
    boost::bind(&ClipsProtobufCommunicator::handle_client_msg, this, client_id, _1, _2, _3));
  client->signal_receive_failed().connect(boost::bind(
    &ClipsProtobufCommunicator::handle_client_receive_fail, this, client_id, _1, _2, _3));

  client->async_connect(host.c_str(), port);
  return client_id;
}

void ClipsProtobufCommunicator::clips_pb_send(int64_t client_id, void * msgptr)
{
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    return;
  }
  try {
    std::lock_guard<std::mutex> lock(map_mutex_);

    if (server_ && server_clients_.find(client_id) != server_clients_.end()) {
      // printf("***** SENDING via SERVER\n");
      server_->send(server_clients_[client_id], msg);
      sig_server_sent_(server_clients_[client_id], msg);
    } else if (clients_.find(client_id) != clients_.end()) {
      // printf("***** SENDING via CLIENT\n");
      clients_[client_id]->send(msg);
      std::pair<std::string, uint16_t> & client_endpoint = client_endpoints_[client_id];
      sig_client_sent_(client_endpoint.first, client_endpoint.second, msg);
    } else if (peers_.find(client_id) != peers_.end()) {
      peers_[client_id]->send(msg);
      sig_peer_sent_(client_id, msg);
    } else {
      // printf("Client ID %li is unknown, cannot send message of type %s\n",
      //     client_id, (*m)->GetTypeName().c_str());
    }
  } catch (google::protobuf::FatalException & e) {
    RCLCPP_WARN(
      *logger_, cx::format(
                  "CLIPS-Profobuf: Failed to send message of type {}: {}",
                  msg->GetTypeName().c_str(), e.what())
                  .c_str());
  } catch (std::runtime_error & e) {
    RCLCPP_WARN(
      *logger_, cx::format(
                  "CLIPS-Profobuf: Failed to send message of type {}: {}",
                  msg->GetTypeName().c_str(), e.what())
                  .c_str());
  }
}

std::string ClipsProtobufCommunicator::clips_pb_tostring(void * msgptr)
{
  auto msg = messages_[msgptr];
  if (!msg) {
    RCLCPP_WARN(*logger_, "CLIPS-Protobuf: Cannot convert message to string: invalid message");
    messages_.erase(msgptr);
    return "";
  }
  std::string res = msg->DebugString();
  std::replace(res.begin(), res.end(), '\n', ' ');  // Replace '\n' with ' '
  return res;
}

void ClipsProtobufCommunicator::clips_pb_broadcast(int64_t peer_id, void * msgptr)
{
  auto msg = messages_[msgptr];
  if (!msg) {
    messages_.erase(msgptr);
    return;
  }

  std::lock_guard<std::mutex> lock(map_mutex_);
  if (peers_.find(peer_id) == peers_.end()) return;

  try {
    peers_[peer_id]->send(msg);
  } catch (google::protobuf::FatalException & e) {
    RCLCPP_WARN(
      *logger_,
      cx::format("Failed to broadcast message of type {}: {}", msg->GetTypeName(), e.what())
        .c_str());
  } catch (std::runtime_error & e) {
    RCLCPP_WARN(
      *logger_,
      cx::format("Failed to broadcast message of type {}: {}", msg->GetTypeName(), e.what())
        .c_str());
  }

  sig_peer_sent_(peer_id, msg);
}

void ClipsProtobufCommunicator::clips_pb_disconnect(int64_t client_id)
{
  RCLCPP_INFO(*logger_, cx::format("CLIPS-Protobuf: Disconnecting client {}", client_id).c_str());
  try {
    std::lock_guard<std::mutex> lock(map_mutex_);

    if (server_clients_.find(client_id) != server_clients_.end()) {
      protobuf_comm::ProtobufStreamServer::ClientID srv_client = server_clients_[client_id];
      server_->disconnect(srv_client);
      server_clients_.erase(client_id);
      rev_server_clients_.erase(srv_client);
    } else if (clients_.find(client_id) != clients_.end()) {
      clients_.erase(client_id);
    }
  } catch (std::runtime_error & e) {
    RCLCPP_WARN(
      *logger_,
      cx::format("CLIPS-Protobuf: Failed to disconnect from client {}: {}", client_id, e.what())
        .c_str());
  }
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_list(
  void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    messages_.erase(msgptr);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    res.lexemeValue = clips::CreateSymbol(clips_, "DOES-NOT-EXIST");
    return res;
  }
  res.begin = 0;
  res.range = -1;
  clips::MultifieldBuilder * mb = clips::CreateMultifieldBuilder(clips_, 10);
  if (
    field->label() == pb::FieldDescriptor::LABEL_REQUIRED ||
    field->label() == pb::FieldDescriptor::LABEL_OPTIONAL) {
    clips::UDFValue aux_val = clips_pb_field_value(msgptr, field_name);
    clips::MBAppend(mb, static_cast<clips::CLIPSValue *>(aux_val.value));
    res.multifieldValue = clips::MBCreate(mb);
    MBDispose(mb);
    return res;
  }

  const pb::Reflection * refl = msg->GetReflection();
  int field_size = refl->FieldSize(*msg, field);
  for (int i = 0; i < field_size; ++i) {
    switch (field->type()) {
      case pb::FieldDescriptor::TYPE_DOUBLE:
        clips::MBAppendFloat(mb, refl->GetRepeatedDouble(*msg, field, i));
        break;
      case pb::FieldDescriptor::TYPE_FLOAT:
        clips::MBAppendFloat(mb, refl->GetRepeatedFloat(*msg, field, i));
        break;
      case pb::FieldDescriptor::TYPE_UINT64:
      case pb::FieldDescriptor::TYPE_FIXED64:
        clips::MBAppendInteger(mb, (int64_t)refl->GetRepeatedUInt64(*msg, field, i));
        break;
      case pb::FieldDescriptor::TYPE_UINT32:
      case pb::FieldDescriptor::TYPE_FIXED32:
        clips::MBAppendInteger(mb, refl->GetRepeatedUInt32(*msg, field, i));
        break;
      case pb::FieldDescriptor::TYPE_BOOL:
        // Booleans are represented as Symbols in CLIPS
        if (refl->GetRepeatedBool(*msg, field, i)) {
          clips::MBAppendSymbol(mb, "TRUE");
        } else {
          clips::MBAppendSymbol(mb, "FALSE");
        }
        break;
      case pb::FieldDescriptor::TYPE_STRING:
        clips::MBAppendString(mb, refl->GetRepeatedString(*msg, field, i).c_str());
        break;
      case pb::FieldDescriptor::TYPE_MESSAGE: {
        const google::protobuf::Message & sub_msg = refl->GetRepeatedMessage(*msg, field, i);
        google::protobuf::Message * mcopy = sub_msg.New();
        mcopy->CopyFrom(sub_msg);
        messages_[mcopy] = std::shared_ptr<google::protobuf::Message>(mcopy);
        clips::MBAppendCLIPSExternalAddress(mb, clips::CreateCExternalAddress(clips_, mcopy));
      } break;
      case pb::FieldDescriptor::TYPE_BYTES:
        clips::MBAppendString(mb, "BYTES");
        break;
      case pb::FieldDescriptor::TYPE_ENUM:
        clips::MBAppendSymbol(mb, refl->GetRepeatedEnum(*msg, field, i)->name().c_str());
        break;
      case pb::FieldDescriptor::TYPE_SFIXED32:
      case pb::FieldDescriptor::TYPE_INT32:
      case pb::FieldDescriptor::TYPE_SINT32:
        clips::MBAppendInteger(mb, refl->GetRepeatedInt32(*msg, field, i));
        break;
      case pb::FieldDescriptor::TYPE_SFIXED64:
      case pb::FieldDescriptor::TYPE_SINT64:
      case pb::FieldDescriptor::TYPE_INT64:
        clips::MBAppendInteger(mb, refl->GetRepeatedInt64(*msg, field, i));
        break;
      default:
        throw std::logic_error("Unknown protobuf field type encountered");
    }
  }
  auto val = clips::MBCreate(mb);
  res.multifieldValue = val;
  MBDispose(mb);
  return res;
}

clips::UDFValue ClipsProtobufCommunicator::clips_pb_field_is_list(
  void * msgptr, std::string field_name)
{
  clips::UDFValue res;
  auto msg = messages_[msgptr];
  if (!msg) {
    res.lexemeValue = clips::CreateBoolean(clips_, false);
    messages_.erase(msgptr);
    return res;
  }

  const pb::Descriptor * desc = msg->GetDescriptor();
  const pb::FieldDescriptor * field = desc->FindFieldByName(field_name);
  if (!field) {
    res.lexemeValue = clips::CreateBoolean(clips_, false);
  }
  res.lexemeValue = clips::CreateBoolean(clips_, field->is_repeated());
  return res;
}

void ClipsProtobufCommunicator::clips_assert_message(
  std::pair<std::string, uint16_t> & endpoint, uint16_t comp_id, uint16_t msg_type,
  std::shared_ptr<google::protobuf::Message> & msg, ClipsProtobufCommunicator::ClientType ct,
  int64_t client_id)
{
  clips::FactBuilder * fact_builder = clips::CreateFactBuilder(clips_, "protobuf-msg");
  if (fact_builder) {
    clips::FBPutSlotString(fact_builder, "type", msg->GetTypeName().c_str());
    clips::FBPutSlotInteger(fact_builder, "comp-id", comp_id);
    clips::FBPutSlotInteger(fact_builder, "msg-type", msg_type);
    clips::FBPutSlotSymbol(fact_builder, "rcvd-via", (ct == CT_PEER) ? "BROADCAST" : "STREAM");
    auto node = parent_.lock();
    clips::FBPutSlotFloat(fact_builder, "rcvd-at", node->get_clock()->now().seconds());
    clips::FBPutSlotMultifield(
      fact_builder, "rcvd-from",
      clips::StringToMultifield(
        clips_, cx::format("{} {}", endpoint.first, endpoint.second).c_str()));
    clips::FBPutSlotSymbol(
      fact_builder, "client-type",
      ct == CT_CLIENT ? "CLIENT" : (ct == CT_SERVER ? "SERVER" : "PEER"));
    clips::FBPutSlotInteger(fact_builder, "client-id", client_id);
    messages_[msg.get()] = msg;
    clips::FBPutSlotCLIPSExternalAddress(
      fact_builder, "ptr", clips::CreateCExternalAddress(clips_, msg.get()));
    clips::Fact * new_fact = clips::FBAssert(fact_builder);

    if (!new_fact) {
      RCLCPP_WARN(*logger_, "CLIPS-Protobuf: Asserting protobuf-msg fact failed");
    }
    clips::FBDispose(fact_builder);
  } else {
    RCLCPP_WARN(*logger_, "CLIPS-Protobuf: Did not get template, did you load protobuf.clp?");
  }
}

void ClipsProtobufCommunicator::handle_server_client_connected(
  protobuf_comm::ProtobufStreamServer::ClientID client, boost::asio::ip::tcp::endpoint & endpoint)
{
  int64_t client_id = -1;
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    client_id = ++next_client_id_;
    client_endpoints_[client_id] = std::make_pair(endpoint.address().to_string(), endpoint.port());
    server_clients_[client_id] = client;
    rev_server_clients_[client] = client_id;
  }

  std::lock_guard<std::mutex> lock(clips_mutex_);
  clips::AssertString(
    clips_, cx::format(
              "(protobuf-server-client-connected {} {} {})", client_id,
              endpoint.address().to_string().c_str(), endpoint.port())
              .c_str());
}

void ClipsProtobufCommunicator::handle_server_client_disconnected(
  protobuf_comm::ProtobufStreamServer::ClientID client, const boost::system::error_code & /*error*/)
{
  int64_t client_id = -1;
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    RevServerClientMap::iterator c;
    if ((c = rev_server_clients_.find(client)) != rev_server_clients_.end()) {
      client_id = c->second;
      rev_server_clients_.erase(c);
      server_clients_.erase(client_id);
    }
  }

  if (client_id >= 0) {
    std::lock_guard<std::mutex> lock(clips_mutex_);
    clips::AssertString(
      clips_, cx::format("(protobuf-server-client-disconnected {})", client_id).c_str());
  }
}

/** Handle message that came from a client.
 * @param client client ID
 * @param component_id component the message was addressed to
 * @param msg_type type of the message
 * @param msg the message
 */
void ClipsProtobufCommunicator::handle_server_client_msg(
  protobuf_comm::ProtobufStreamServer::ClientID client, uint16_t component_id, uint16_t msg_type,
  std::shared_ptr<google::protobuf::Message> msg)
{
  std::scoped_lock lock{clips_mutex_, map_mutex_};
  RevServerClientMap::iterator c;
  if ((c = rev_server_clients_.find(client)) != rev_server_clients_.end()) {
    clips_assert_message(
      client_endpoints_[c->second], component_id, msg_type, msg, CT_SERVER, c->second);
  }
}

/** Handle server reception failure
 * @param client client ID
 * @param component_id component the message was addressed to
 * @param msg_type type of the message
 * @param msg the message string
 */
void ClipsProtobufCommunicator::handle_server_client_fail(
  protobuf_comm::ProtobufStreamServer::ClientID client, uint16_t component_id, uint16_t msg_type,
  std::string msg)
{
  std::lock_guard<std::mutex> lock(map_mutex_);
  RevServerClientMap::iterator c;
  if ((c = rev_server_clients_.find(client)) != rev_server_clients_.end()) {
    std::lock_guard<std::mutex> lock(clips_mutex_);
    clips::AssertString(
      clips_, cx::format(
                "(protobuf-server-receive-failed (comp-id {}) (msg-type {}) "
                "(rcvd-via STREAM) (client-id {}) (message \"{}\") "
                "(rcvd-from (\"{}\" {})))",
                component_id, msg_type, c->second, msg.c_str(),
                client_endpoints_[c->second].first.c_str(), client_endpoints_[c->second].second)
                .c_str());
  }
}

/** Handle message that came from a peer/robot
 * @param endpoint the endpoint from which the message was received
 * @param component_id component the message was addressed to
 * @param msg_type type of the message
 * @param msg the message
 */
void ClipsProtobufCommunicator::handle_peer_msg(
  int64_t peer_id, boost::asio::ip::udp::endpoint & endpoint, uint16_t component_id,
  uint16_t msg_type, std::shared_ptr<google::protobuf::Message> msg)
{
  std::lock_guard<std::mutex> lock(clips_mutex_);
  std::pair<std::string, uint16_t> endpp =
    std::make_pair(endpoint.address().to_string(), endpoint.port());
  clips_assert_message(endpp, component_id, msg_type, msg, CT_PEER, peer_id);
}

/** Handle error during peer message processing.
 * @param endpoint endpoint of incoming message
 * @param msg error message
 */
void ClipsProtobufCommunicator::handle_peer_recv_error(
  int64_t /*peer_id*/, boost::asio::ip::udp::endpoint & endpoint, std::string msg)
{
  RCLCPP_WARN(
    *logger_, cx::format(
                "CLIPS-Protobuf: Failed to receive peer message from {}:{}: {}",
                endpoint.address().to_string(), endpoint.port(), msg)
                .c_str());
}

/** Handle error during peer message processing.
 * @param msg error message
 */
void ClipsProtobufCommunicator::handle_peer_send_error(int64_t /*peer_id*/, std::string msg)
{
  RCLCPP_WARN(*logger_, cx::format("CLIPS-Protobuf: Failed to send peer message: {}", msg).c_str());
}

void ClipsProtobufCommunicator::handle_client_connected(int64_t client_id)
{
  std::lock_guard<std::mutex> lock(clips_mutex_);
  clips::AssertString(clips_, cx::format("(protobuf-client-connected {})", client_id).c_str());
}

void ClipsProtobufCommunicator::handle_client_disconnected(
  int64_t client_id, const boost::system::error_code & /*error*/)
{
  std::lock_guard<std::mutex> lock(clips_mutex_);
  clips::AssertString(clips_, cx::format("(protobuf-client-disconnected {})", client_id).c_str());
}

void ClipsProtobufCommunicator::handle_client_msg(
  int64_t client_id, uint16_t comp_id, uint16_t msg_type,
  std::shared_ptr<google::protobuf::Message> msg)
{
  std::lock_guard<std::mutex> lock(clips_mutex_);
  std::pair<std::string, uint16_t> endpp = std::make_pair(std::string(), 0);
  clips_assert_message(endpp, comp_id, msg_type, msg, CT_CLIENT, client_id);
}

void ClipsProtobufCommunicator::handle_client_receive_fail(
  int64_t client_id, uint16_t comp_id, uint16_t msg_type, std::string msg)
{
  std::lock_guard<std::mutex> lock(clips_mutex_);
  clips::AssertString(
    clips_, cx::format(
              "(protobuf-receive-failed (client-id {}) (rcvd-via STREAM) "
              "(comp-id {}) (msg-type {}) (message \"{}\"))",
              client_id, comp_id, msg_type, msg.c_str())
              .c_str());
}

std::string ClipsProtobufCommunicator::to_string(const clips::UDFValue & v)
{
  switch (v.lexemeValue->header.type) {
    case VOID_TYPE:
      return "Void Type";
    case FLOAT_TYPE:
      return std::to_string(v.floatValue->contents);
    case INTEGER_TYPE:
      return std::to_string(v.integerValue->contents);
    case SYMBOL_TYPE:
    case INSTANCE_NAME_TYPE:
    case STRING_TYPE:
      return v.lexemeValue->contents;
    case INSTANCE_ADDRESS_TYPE:
    case FACT_ADDRESS_TYPE:
    case EXTERNAL_ADDRESS_TYPE:
      return cx::format("{:p}", v.externalAddressValue->contents);

    case MULTIFIELD_TYPE:
      std::string res;

      for (size_t i = v.begin; i < (v.begin + v.range); i++) {
        // actually, a type check for each field would be needed...
        res += v.multifieldValue->contents[i].lexemeValue->contents;
      }
      return res;
  }
  return "Implicit unknown type";
}

}  // end namespace protobuf_clips
