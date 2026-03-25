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

#include <curl/curl.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <gz/common/Console.hh>

#include "gz/fuel_tools/RestClient.hh"

namespace gz::fuel_tools
{

// List of known file extensions and associated mime type.
static const std::map<std::string, std::string> kContentTypes =
{
  {".config", "text/xml"},
  {".bvh",    "text/plain"},
  {".dae",    "text/xml"},
  {".erb",    "text/plain"},
  {".obj",    "text/plain"},
  {".gif",    "image/gif"},
  {".glsl",   "text/plain"},
  {".htm",    "text/html"},
  {".html",   "text/html"},
  {".jpg",    "image/jpeg"},
  {".jpeg",   "image/jpeg"},
  {".json",   "text/json"},
  {".material", "text/plain"},
  {".md",    "text/plain"},
  {".mtl",   "text/plain"},
  {".pbtxt", "text/json"},
  {".pdf",   "application/pdf"},
  {".png",   "image/png"},
  {".sdf",   "text/xml"},
  {".stl",   "text/plain"},
  {".svg",   "image/svg+xml"},
  {".tga",   "image/tga"},
  {".txt",   "text/plain"},
  {".urdf",  "text/xml"},
  {".xml",   "text/xml"},
};

//////////////////////////////////////////////////
std::string RestJoinUrl(const std::string &_base,
    const std::string &_more)
{
  if (_base.empty())
    return _more;
  else if (_more.empty())
    return _base;

  if ((_base[_base.size()-1] == '/' && _more[0] != '/')
      || (_base[_base.size()-1] != '/' && _more[0] == '/'))
  {
    // Only one of them has a slash
    return _base + _more;
  }
  else if (_base[_base.size()-1] != '/' && _more[0] != '/')
  {
    // Neither have a slash
    return  _base + '/' + _more;
  }
  else
  {
    // Both have a slash
    return _base + _more.substr(1, _more.size() - 1);
  }
}

/////////////////////////////////////////////////
size_t RestHeaderCallback(char *_ptr, size_t _size, size_t _nmemb, void *_userp)
{
  std::map<std::string, std::string> *map =
    static_cast<std::map<std::string, std::string> *>(_userp);

  _size *= _nmemb;

  if (map)
  {
    std::string header(_ptr);
    auto colonPos = header.find(":");

    // Only store header information of the form
    //     <type>: <data>
    if (colonPos != std::string::npos)
    {
      map->insert(std::make_pair(header.substr(0, colonPos),
                                 header.substr(colonPos+2)));
    }
  }

  return _size;
}

/////////////////////////////////////////////////
size_t RestWriteMemoryCallback(void *_buffer, size_t _size, size_t _nmemb,
    void *_userp)
{
  std::string *str = static_cast<std::string*>(_userp);
  _size *= _nmemb;

  // Append the new character data to the string
  str->append(static_cast<const char*>(_buffer), _size);
  return _size;
}

/////////////////////////////////////////////////
void AddFormPost(
    curl_mime * const multipart,
    const std::multimap<std::string, std::string> &_form)
{
  for (const auto &[key, value] : _form)
  {
    curl_mimepart *part = curl_mime_addpart(multipart);
    // follow same convention as curl cmdline tool
    // field starting with @ indicates path to file to upload
    // others are standard fields to describe the file
    if (!value.empty() && value[0] == '@')
    {
      // Default file path
      std::string path = value.substr(1);

      // Default upload filename
      std::string uploadFilename = gz::common::basename(path);

      // If the value has a semicolon, then use the string preceding the
      // semicolon as the local filesystem path and the string following
      // the semicolon as the upload filename.
      if (value.substr(1).find(";") != std::string::npos)
      {
        path = value.substr(1, value.find(";") - 1);
        uploadFilename = value.substr(value.find(";") + 1);
      }

      std::string basename = gz::common::basename(path);
      std::string contentType = "application/octet-stream";

      // Figure out the content type based on the file extension.
      std::string::size_type dotIdx = basename.rfind('.');
      if (dotIdx != std::string::npos)
      {
        std::string extension =
          gz::common::lowercase(basename.substr(dotIdx));
        if (kContentTypes.find(extension) != kContentTypes.end())
        {
          contentType = kContentTypes.at(extension);
        }
        else
        {
          gzwarn << "Unknown mime type for file[" << path
            << "]. The mime type '" << contentType << "' will be used.\n";
        }
      }

      curl_mime_name(part, key.c_str());
      curl_mime_filename(part, uploadFilename.c_str());
      curl_mime_filedata(part, path.c_str());
      curl_mime_type(part, contentType.c_str());
    }
    else
    {
      curl_mime_name(part, key.c_str());
      curl_mime_data(part, value.c_str(), CURL_ZERO_TERMINATED);
    }
  }
}

/////////////////////////////////////////////////
RestResponse Rest::Request(HttpMethod _method,
    const std::string &_url, const std::string &_version,
    const std::string &_path, const std::vector<std::string> &_queryStrings,
    const std::vector<std::string> &_headers, const std::string &_data,
    const std::multimap<std::string, std::string> &_form) const
{
  RestResponse res;

  if (_url.empty())
    return res;

  std::string url = _url;
  if (!_version.empty())
    url = RestJoinUrl(_url, _version);

  CURL *curl = curl_easy_init();
  char *encodedPath = nullptr;

  if (!_path.empty())
  {
    // First, unescape the _path since it might have %XX encodings. If this
    // step is not performed, then curl_easy_escape will encode the %XX
    // encodings resulting in an incorrect URL.
    int decodedSize;
    char *decodedPath = curl_easy_unescape(curl,
        _path.c_str(), _path.size(), &decodedSize);

    encodedPath = curl_easy_escape(curl, decodedPath, decodedSize);
    url = RestJoinUrl(url, encodedPath);
    curl_free(decodedPath);
  }

  // Process query strings.
  if (!_queryStrings.empty())
  {
    std::string fullQuery{"?"};
    for (const std::string &queryString : _queryStrings)
      // cppcheck-suppress useStlAlgorithm
      fullQuery += queryString + "&";

    fullQuery.pop_back();

    if (fullQuery != "?")
      url += fullQuery;
  }

  // Process headers.
  struct curl_slist *headers = nullptr;
  for (const std::string &header : _headers)
  {
    headers = curl_slist_append(headers, header.c_str());
    if (!headers)
    {
      gzerr << "[Rest::Request()]: Error processing header.\n  ["
                << header.c_str() << "]" << std::endl;

      // cleanup
      curl_easy_cleanup(curl);
      return res;
    }
  }

  // enable TCP keep-alive for this transfer
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  // keep-alive idle time to 120 seconds
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);

  // interval time between keep-alive probes: 60 seconds
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, this->userAgent.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  std::string responseData;
  std::map<std::string, std::string> headerData;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestWriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestHeaderCallback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

  char errbuf[CURL_ERROR_SIZE];
  // provide a buffer to store errors in
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  // set the error buffer as empty before performing a request
  errbuf[0] = 0;

  // ToDo: Set this option to 0 only when using localhost.
  // Set the default value: do not prove that SSL certificate is authentic
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

  // Follow redirects to any URL set on the Location header.
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // Set cURL to only follow 3 redirects tops.
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

  std::ifstream ifs;
  curl_mime *multipart = curl_mime_init(curl);

  // Send the request.
  if (_method == HttpMethod::GET)
  {
    // no need to do anything
  }
  else if (_method == HttpMethod::PATCH_FORM)
  {
    AddFormPost(multipart, _form);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
  }
  else if (_method == HttpMethod::POST)
  {
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _data.c_str());
  }
  else if (_method == HttpMethod::POST_FORM)
  {
    AddFormPost(multipart, _form);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
  }
  else if (_method == HttpMethod::DELETE)
  {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  }
  else
  {
    gzerr << "Unsupported method" << std::endl;

    // Cleanup.
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
  }

  CURLcode success = curl_easy_perform(curl);
  if (success != CURLE_OK)
  {
    gzerr << "Error in REST request" << std::endl;
    size_t len = strlen(errbuf);
    fprintf(stderr, "\nlibcurl: (%d) ", success);
    if (len)
    {
      fprintf(stderr, "%s%s", errbuf,
              ((errbuf[len - 1] != '\n') ? "\n" : ""));
    }
    else
      fprintf(stderr, "%s\n", curl_easy_strerror(success));
  }

  // Update the status code.
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.statusCode);

  // Update the data.
  res.data = responseData;

  // Update the header data.
  res.headers = headerData;

  // free encoded path char*
  if (encodedPath)
    curl_free(encodedPath);

  // free the headers
  curl_slist_free_all(headers);

  // Cleaning.
  curl_mime_free(multipart);
  curl_easy_cleanup(curl);

  if (ifs.is_open())
    ifs.close();
  return res;
}

/////////////////////////////////////////////////
void Rest::SetUserAgent(const std::string &_agent)
{
  this->userAgent = _agent;
}

/////////////////////////////////////////////////
const std::string &Rest::UserAgent() const
{
  return this->userAgent;
}
}  // namespace gz::fuel_tools
