/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef GZ_FUEL_TOOLS_RESTCLIENT_HH_
#define GZ_FUEL_TOOLS_RESTCLIENT_HH_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gz/fuel_tools/Export.hh"
#include "gz/fuel_tools/HttpMethod.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::string
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Stores a response to a RESTful request
  struct GZ_FUEL_TOOLS_VISIBLE RestResponse
  {
    /// \brief The returned status code. E.g.: 200
    // cppcheck-suppress unusedStructMember
    public: int statusCode = 0;

    /// \brief The data received.
    public: std::string data = "";

    /// \brief Map of headers where the key is the header type.
    /// For example, a raw header of the form "Content-Type: json" would
    /// use "Content-Type" as a key and "json" as the key's data.
    public: std::map<std::string, std::string> headers;
  };

  /// \brief A helper class for making REST requests.
  class GZ_FUEL_TOOLS_VISIBLE Rest
  {
    /// \brief Default constructor.
    public: Rest() = default;

    /// \brief Trigger a REST request.
    /// \param[in] _method The HTTP method. Use all uppercase letters.
    ///            E.g.: "GET"
    /// \param[in] _url The url to request.
    ///            E.g.: "http://localhost:8000/"
    /// \param[in] _version The protocol version.
    ///            E.g.: "1.0"
    /// \param[in] _path The path to request.
    ///            E.g.: "collection"
    /// \param[in] _queryStrings All the query strings to be requested.
    ///            E.g.: {"sort=name", "sortdir=1"}
    /// \param[in] _headers All the headers to be included in the HTTP request
    ///            E.g.: {"Accept: application/json"}
    /// \param[in] _data Data to be included in the HTTP request.
    /// \param[in] _form Multi-part / form data to be used with
    /// Method::POST_FORM. Files can be transferred by using a value of
    /// "@<local_file_path>;<upload_filename>". For example, to transfer the
    /// local file "/tmp/test.txt" and upload the file as "changed.txt"
    /// in a form with the field name "file", then insert
    /// the following key-value pair into the _form parameter:
    /// `{"file", "@/tmp/test.txt;changed.txt"}`. The ";<upload_filename>"
    /// portion is optional. If this part is absent, then the basename of
    /// provided "<local_file_path>" is used. For example,
    /// "@/tmp/text.txt" will be uploaded as "text.txt".
    public: virtual RestResponse Request(const HttpMethod _method,
        const std::string &_url,
        const std::string &_version,
        const std::string &_path,
        const std::vector<std::string> &_queryStrings,
        const std::vector<std::string> &_headers,
        const std::string &_data,
        const std::multimap<std::string, std::string> &_form =
        std::multimap<std::string, std::string>()) const;

    /// \brief Set the user agent name.
    /// \param[in] _agent User agent name.
    public: void SetUserAgent(const std::string &_agent);

    /// \brief Get the user agent name.
    /// \return Name of the user agent.
    public: const std::string &UserAgent() const;

    /// \brief The user agent name.
    private: std::string userAgent;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_RESTCLIENT_HH_
