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

#include "gz/common/Profiler.hh" // NOLINT(*)
#include <gtest/gtest.h> // NOLINT(*)
#include "Remotery.h"

#include <gz/common/Export.hh>

/// Return the string error message
/// param[in] error the remotery error
std::string rmtErrorToString(rmtError error);

/////////////////////////////////////////////////
TEST(Profiler, ProfilerErrorCodes)
{
  EXPECT_EQ("none", rmtErrorToString(RMT_ERROR_NONE));
  EXPECT_EQ("Not an error but an internal message to calling code",
    rmtErrorToString(RMT_ERROR_RECURSIVE_SAMPLE));

  EXPECT_EQ("Malloc call within remotery failed",
    rmtErrorToString(RMT_ERROR_MALLOC_FAIL));
  EXPECT_EQ("Attempt to allocate thread local storage failed",
    rmtErrorToString(RMT_ERROR_TLS_ALLOC_FAIL));
  EXPECT_EQ("Failed to create a virtual memory mirror buffer",
    rmtErrorToString(RMT_ERROR_VIRTUAL_MEMORY_BUFFER_FAIL));
  EXPECT_EQ("Failed to create a thread for the server",
    rmtErrorToString(RMT_ERROR_CREATE_THREAD_FAIL));

  EXPECT_EQ("Network initialisation failure (e.g. on Win32, WSAStartup fails)",
    rmtErrorToString(RMT_ERROR_SOCKET_INIT_NETWORK_FAIL));
  EXPECT_EQ("Can't create a socket for connection to the remote viewer",
    rmtErrorToString(RMT_ERROR_SOCKET_CREATE_FAIL));
  EXPECT_EQ("Can't bind a socket for the server",
    rmtErrorToString(RMT_ERROR_SOCKET_BIND_FAIL));
  EXPECT_EQ("Created server socket failed to enter a listen state",
    rmtErrorToString(RMT_ERROR_SOCKET_LISTEN_FAIL));
  EXPECT_EQ("Created server socket failed to switch to a non-blocking state",
    rmtErrorToString(RMT_ERROR_SOCKET_SET_NON_BLOCKING_FAIL));
  EXPECT_EQ("Poll attempt on an invalid socket",
    rmtErrorToString(RMT_ERROR_SOCKET_INVALID_POLL));
  EXPECT_EQ("Server failed to call select on socket",
    rmtErrorToString(RMT_ERROR_SOCKET_SELECT_FAIL));
  EXPECT_EQ("Poll notified that the socket has errors",
    rmtErrorToString(RMT_ERROR_SOCKET_POLL_ERRORS));
  EXPECT_EQ("Server failed to accept connection from client",
    rmtErrorToString(RMT_ERROR_SOCKET_ACCEPT_FAIL));
  EXPECT_EQ("Timed out trying to send data",
    rmtErrorToString(RMT_ERROR_SOCKET_SEND_TIMEOUT));
  EXPECT_EQ("Unrecoverable error occured while client/server tried to send data", //NOLINT
    rmtErrorToString(RMT_ERROR_SOCKET_SEND_FAIL));
  EXPECT_EQ("No data available when attempting a receive",
    rmtErrorToString(RMT_ERROR_SOCKET_RECV_NO_DATA));
  EXPECT_EQ("Timed out trying to receive data",
    rmtErrorToString(RMT_ERROR_SOCKET_RECV_TIMEOUT));
  EXPECT_EQ("Unrecoverable error occured while client/server tried to receive data", //NOLINT
    rmtErrorToString(RMT_ERROR_SOCKET_RECV_FAILED));

  EXPECT_EQ("WebSocket server handshake failed, not HTTP GET",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_NOT_GET));
  EXPECT_EQ("WebSocket server handshake failed, can't locate WebSocket version",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_NO_VERSION));
  EXPECT_EQ("WebSocket server handshake failed, unsupported WebSocket version",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_BAD_VERSION));
  EXPECT_EQ("WebSocket server handshake failed, can't locate host",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_NO_HOST));
  EXPECT_EQ("WebSocket server handshake failed, host is not allowed to connect",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_BAD_HOST));
  EXPECT_EQ("WebSocket server handshake failed, can't locate WebSocket key",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_NO_KEY));
  EXPECT_EQ("WebSocket server handshake failed, WebSocket key is ill-formed",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_BAD_KEY));
  EXPECT_EQ("WebSocket server handshake failed, internal error, bad string code", //NOLINT
    rmtErrorToString(RMT_ERROR_WEBSOCKET_HANDSHAKE_STRING_FAIL));
  EXPECT_EQ("WebSocket server received a disconnect request and closed the socket", //NOLINT
    rmtErrorToString(RMT_ERROR_WEBSOCKET_DISCONNECTED));
  EXPECT_EQ("Couldn't parse WebSocket frame header",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_BAD_FRAME_HEADER));
  EXPECT_EQ("Partially received wide frame header size",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_BAD_FRAME_HEADER_SIZE));
  EXPECT_EQ("Partially received frame header data mask",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_BAD_FRAME_HEADER_MASK));
  EXPECT_EQ("Timeout receiving frame header",
    rmtErrorToString(RMT_ERROR_WEBSOCKET_RECEIVE_TIMEOUT));

  EXPECT_EQ("Remotery object has not been created",
    rmtErrorToString(RMT_ERROR_REMOTERY_NOT_CREATED));
  EXPECT_EQ("An attempt was made to send an incomplete profile tree to the client", //NOLINT
    rmtErrorToString(RMT_ERROR_SEND_ON_INCOMPLETE_PROFILE));

  EXPECT_EQ("This indicates that the CUDA driver is in the process of shutting down", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_DEINITIALIZED));
  EXPECT_EQ("This indicates that the CUDA driver has not been initialized with cuInit() or that initialization has failed", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_NOT_INITIALIZED));
  EXPECT_EQ("This most frequently indicates that there is no context bound to the current thread", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_INVALID_CONTEXT));
  EXPECT_EQ("This indicates that one or more of the parameters passed to the API call is not within an acceptable range of values", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_INVALID_VALUE));
  EXPECT_EQ("This indicates that a resource handle passed to the API call was not valid", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_INVALID_HANDLE));
  EXPECT_EQ("The API call failed because it was unable to allocate enough memory to perform the requested operation", //NOLINT
    rmtErrorToString(RMT_ERROR_CUDA_OUT_OF_MEMORY));
  EXPECT_EQ("This indicates that a resource handle passed to the API call was not valid", //NOLINT
    rmtErrorToString(RMT_ERROR_ERROR_NOT_READY));

  EXPECT_EQ("Failed to create query for sample",
    rmtErrorToString(RMT_ERROR_D3D11_FAILED_TO_CREATE_QUERY));

  EXPECT_EQ("Generic OpenGL error, no need to expose detail since app will need an OpenGL error callback registered", //NOLINT
    rmtErrorToString(RMT_ERROR_OPENGL_ERROR));
  EXPECT_EQ("Unknown CUDA error",
    rmtErrorToString(RMT_ERROR_CUDA_UNKNOWN));
  EXPECT_EQ("Unknown remotery error",
    rmtErrorToString(static_cast<rmtError>(1000)));
}
