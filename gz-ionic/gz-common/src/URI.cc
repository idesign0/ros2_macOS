/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <algorithm>
#include <cstring>
#include <list>
#include <map>
#include <regex>
#include <string>

#include "gz/common/Console.hh"
#include "gz/common/URI.hh"
#include "gz/common/Util.hh"

using namespace gz;
using namespace common;

static const char kSchemeDelim[] = ":";
static const char kAuthDelim[] = "//";

/// \brief URIAuthority private data.
class common::URIAuthority::Implementation
{
  /// \brief The user information.
  public: std::string userInfo;

  /// \brief The host.
  public: std::string host;

  /// \brief The port.
  public: std::optional<int> port;

  /// \brief Set this to true if an empty host is valid. This should only be
  /// set to true if the corresponding URIScheme is "file".
  public: bool emptyHostValid = false;

  /// \brief True if the host was set to empty.
  public: bool hasEmptyHost = false;
};

/// \brief URIPath private data.
class common::URIPath::Implementation
{
  /// \brief A helper method to determine if the given string represents
  ///        an absolute path starting segment or not.
  public: bool IsStringAbsolute(const std::string &_path)
  {
    return (_path.length() > 0 && _path[0] == '/') ||
           (_path.length() > 1 && _path[1] == ':');
  }

  /// \brief The parts of the path.
  public: std::list<std::string> path;

  /// \brief Whether the path is absolute (starts with slash) or not.
  public: bool isAbsolute = false;

  /// \brief True if there is a trailing slash in the path.
  public: bool trailingSlash = false;
};

/// \brief URIQuery private data.
class common::URIQuery::Implementation
{
  /// \brief The key/value tuples that compose the query.
  public: std::vector<std::pair<std::string, std::string>> values;
};

/// \brief URIFragment private data.
class common::URIFragment::Implementation
{
  /// \brief The value of the fragment.
  public: std::string value;
};

/// \brief URI private data.
class common::URI::Implementation
{
  /// \brief The URI scheme.
  public: std::string scheme;

  /// \brief Authority component.
  public: std::optional<URIAuthority> authority;

  /// \brief Path component.
  public: URIPath path;

  /// \brief Query component.
  public: URIQuery query;

  /// \brief Fragment component.
  public: URIFragment fragment;
};

//////////////////////////////////////////////////
URIAuthority::URIAuthority()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
URIAuthority::URIAuthority(const std::string &_str)
  : URIAuthority()
{
  if (!this->Parse(_str))
  {
    gzwarn << "Unable to parse URIAuthority [" << _str << "]. Ignoring."
           << std::endl;
  }
}

//////////////////////////////////////////////////
void URIAuthority::Clear()
{
  this->dataPtr->userInfo.clear();
  this->dataPtr->host.clear();
  this->dataPtr->port.reset();
  this->dataPtr->emptyHostValid = false;
  this->dataPtr->hasEmptyHost = false;
}

//////////////////////////////////////////////////
std::string URIAuthority::UserInfo() const
{
  return this->dataPtr->userInfo;
}

//////////////////////////////////////////////////
void URIAuthority::SetUserInfo(const std::string &_userInfo)
{
  this->dataPtr->userInfo = _userInfo;
}

//////////////////////////////////////////////////
std::string URIAuthority::Host() const
{
  return this->dataPtr->host;
}

//////////////////////////////////////////////////
void URIAuthority::SetHost(const std::string &_host)
{
  this->dataPtr->host = _host;
}

//////////////////////////////////////////////////
std::optional<int> URIAuthority::Port() const
{
  return this->dataPtr->port;
}

//////////////////////////////////////////////////
void URIAuthority::SetPort(int _port)
{
  this->dataPtr->port.emplace(_port);
}

//////////////////////////////////////////////////
bool URIAuthority::operator==(const URIAuthority &_auth) const
{
  return this->dataPtr->userInfo == _auth.UserInfo() &&
         this->dataPtr->host == _auth.Host() &&
         this->dataPtr->port == _auth.Port() &&
         this->dataPtr->emptyHostValid == _auth.EmptyHostValid();
}

//////////////////////////////////////////////////
std::string URIAuthority::Str() const
{
  if (!this->dataPtr->host.empty() ||
      (this->dataPtr->emptyHostValid && this->dataPtr->hasEmptyHost))
  {
    std::string result = kAuthDelim;
    result += this->dataPtr->userInfo.empty() ? "" :
      this->dataPtr->userInfo + "@";
    result += this->dataPtr->host;
    result += this->dataPtr->port ?
      ":" + std::to_string(*this->dataPtr->port) : "";
    return result;
  }

  return "";
}

//////////////////////////////////////////////////
bool URIAuthority::Valid() const
{
  return URIAuthority::Valid(this->Str());
}

//////////////////////////////////////////////////
bool URIAuthority::Valid(const std::string &_str, bool _emptyHostValid)
{
  auto str = trimmed(_str);
  if (str.empty())
    return true;

  // The authority must start with two forward slashes
  if (str.find(kAuthDelim) != 0)
    return false;

  auto userInfoIndex = str.find("@", 2);
  if (userInfoIndex != std::string::npos)
  {
    // Allowed character for userinformation from RFC3986:
    // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
    // pct-encoded   = "%" HEXDIG HEXDIG
    // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
    //                 / "*" / "+" / "," / ";" / "="
    const std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                                     "QWERTZUIOPASDFGHJKLYXCVBNM"
                                     "0123456789"
                                     "-._~"
                                     "%"
                                     "!$&'()*+,;=";

    std::string userInfo = str.substr(2, userInfoIndex - 2);
    if (userInfo.find_first_not_of(allowedChars, 1) != std::string::npos)
      return false;
    userInfoIndex += 1;
  }
  else
  {
    userInfoIndex = 2;
  }

  auto ipv6StartIndex = str.find("[", userInfoIndex);
  std::string host;
  // Is this an IPV6 address?
  if (ipv6StartIndex != std::string::npos)
  {
    auto ipv6EndIndex = str.find("]", ipv6StartIndex);
    // IPV6 must be surrounded by square brackets [ ].
    if (ipv6EndIndex == std::string::npos)
      return false;
    host = str.substr(ipv6StartIndex, ipv6EndIndex);
  }
  // Is a port specified?
  else if (str.find(":", userInfoIndex) != std::string::npos)
  {
    host = str.substr(userInfoIndex,
        str.find(":", userInfoIndex) - userInfoIndex);
  }
  else
    host = str.substr(userInfoIndex);

  // The host can't be empty.
  if (host.empty() && !_emptyHostValid)
    return false;

  // IP-literal  = "[" IPV6address "]
  // IPV4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
  // reg-name = *( unreserved / pct-encoded  )
  //
  // pct-encoded   =  "%" HEXDIG HEXDIG
  // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
  const std::string hostAllowedChars = "qwertzuiopasdfghjklyxcvbnm"
                                   "QWERTZUIOPASDFGHJKLYXCVBNM"
                                   "0123456789"
                                   "%"
                                   "-._"
                                   "[] :";
  if (host.find_first_not_of(hostAllowedChars, 1) != std::string::npos)
    return false;

  auto portIndex = str.find(":", userInfoIndex + host.size());
  if (portIndex != std::string::npos)
  {
    std::string portStr = str.substr(portIndex+1);
    const std::string allowedChars = "0123456789";
    if (portStr.find_first_not_of(allowedChars, 1) != std::string::npos)
      return false;
  }

  return true;
}

//////////////////////////////////////////////////
bool URIAuthority::EmptyHostValid() const
{
  return this->dataPtr->emptyHostValid;
}

//////////////////////////////////////////////////
void URIAuthority::SetEmptyHostValid(bool _valid)
{
  this->dataPtr->emptyHostValid = _valid;
}

//////////////////////////////////////////////////
bool URIAuthority::Parse(const std::string &_str, bool _emptyHostValid)
{
  if (!this->Valid(_str, _emptyHostValid))
    return false;

  this->Clear();

  this->dataPtr->emptyHostValid = _emptyHostValid;

  if (_str.empty() || _str == kAuthDelim)
  {
    this->dataPtr->hasEmptyHost = true;
    return true;
  }

  auto userInfoIndex = _str.find("@");
  if (userInfoIndex != std::string::npos)
  {
    this->dataPtr->userInfo = _str.substr(2, userInfoIndex - 2);
    userInfoIndex += 1;
  }
  else
  {
    userInfoIndex = 2;
  }

  auto ipv6StartIndex = _str.find("[", userInfoIndex);
  // Is this an IPV6 address?
  if (ipv6StartIndex != std::string::npos)
  {
    auto ipv6EndIndex = _str.find("]", ipv6StartIndex);
    this->dataPtr->host = _str.substr(ipv6StartIndex, ipv6EndIndex);
  }
  // Is a port specified?
  else if (_str.find(":", userInfoIndex) != std::string::npos)
  {
    this->dataPtr->host = _str.substr(userInfoIndex,
        _str.find(":", userInfoIndex) - userInfoIndex);
  }
  else
    this->dataPtr->host = _str.substr(userInfoIndex);

  auto portIndex = _str.find(":", userInfoIndex + this->dataPtr->host.size());
  if (portIndex != std::string::npos)
  {
    std::string portStr = _str.substr(portIndex + 1);
    try
    {
      this->dataPtr->port.emplace(std::stoi(portStr));
    }
    catch (...)
    {
      // Do nothing. The port number was invalid.
    }
  }

  return true;
}

/////////////////////////////////////////////////
URIPath::URIPath()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
URIPath::URIPath(const std::string &_str)
  : URIPath()
{
  if (!this->Parse(_str))
  {
    gzwarn << "Unable to parse URIPath [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
bool URIPath::IsAbsolute() const
{
  return this->dataPtr->isAbsolute;
}

/////////////////////////////////////////////////
void URIPath::SetAbsolute(const bool _absolute)
{
  if (this->dataPtr->isAbsolute && !_absolute && !this->dataPtr->path.empty() &&
      this->dataPtr->path.front().length() >= 2 &&
      this->dataPtr->path.front()[1] == ':')
  {
    gzerr << "URIPath " << this->Str() << " cannot be set to represent a "
              "relative path because it starts with a Windows drive "
              "specifier." << std::endl;
    return;
  }

  this->dataPtr->isAbsolute = _absolute;
}

/////////////////////////////////////////////////
void URIPath::SetRelative()
{
  this->SetAbsolute(false);
}

/////////////////////////////////////////////////
void URIPath::PushFront(const std::string &_part)
{
  if (_part.empty())
  {
    gzwarn << "Adding empty path segment to URI " << this->Str()
            << " has no effect." << std::endl;
    return;
  }

  auto part = _part;
  if (_part[0] == '/')
  {
    gzwarn << "Instead of pushing a string starting with slash, call "
               "SetAbsolute() instead." << std::endl;
    part = _part.substr(1);
    this->SetAbsolute();
  }
  // Windows absolute path
  else if (_part.length() >= 2 && _part[1] == ':')
  {
    this->SetAbsolute();
  }

  if (part.find('/') != std::string::npos)
  {
    // TODO(anyone): Once URI encoding is implemented,
    // all invalid characters should be
    // encoded, not just slashes.
    gzwarn << "Unencoded slashes in URI part, encoding them." << std::endl;
    part = common::replaceAll(part, "/", "%2F");
  }

  if (!part.empty())
    this->dataPtr->path.push_front(part);
}

/////////////////////////////////////////////////
void URIPath::PushBack(const std::string &_part)
{
  if (_part.empty())
  {
    gzwarn << "Adding empty path segment to URI " << this->Str()
            << " has no effect." << std::endl;
    return;
  }

  auto part = _part;
  if (this->dataPtr->path.size() == 0 && _part[0] == '/')
  {
    gzwarn << "Instead of pushing a string starting with slash, call "
               "SetAbsolute() instead." << std::endl;
    part = _part.substr(1);
    this->SetAbsolute();
  }
  // Windows absolute path
  else if (this->dataPtr->path.empty() && _part.size() >= 2 && _part[1] == ':')
  {
    this->SetAbsolute();
  }

  if (part.find('/') != std::string::npos)
  {
    // TODO(anyone): Once URI encoding is implemented,
    // all invalid characters should be
    // encoded, not just slashes.
    gzwarn << "Unencoded slashes in URI part, encoding them." << std::endl;
    part = common::replaceAll(part, "/", "%2F");
  }

  this->dataPtr->path.push_back(part);
}

/////////////////////////////////////////////////
std::string URIPath::PopFront()
{
  if (this->dataPtr->path.size() == 0)
    return std::string();

  auto result = this->dataPtr->path.front();
  this->dataPtr->path.pop_front();
  return result;
}

/////////////////////////////////////////////////
std::string URIPath::PopBack()
{
  if (this->dataPtr->path.size() == 0)
    return std::string();

  auto result = this->dataPtr->path.back();
  this->dataPtr->path.pop_back();
  return result;
}

/////////////////////////////////////////////////
const URIPath URIPath::operator/(const std::string &_part) const
{
  URIPath result = *this;
  result /= _part;
  return result;
}

/////////////////////////////////////////////////
const URIPath &URIPath::operator/=(const std::string &_part)
{
  this->PushBack(_part);
  return *this;
}

/////////////////////////////////////////////////
std::string URIPath::Str(const std::string &_delim) const
{
  std::string result(this->dataPtr->isAbsolute ? "/" : "");
  // if the first element is a windows drive specifier, do not prepend the slash
  if (!this->dataPtr->path.empty() && this->dataPtr->path.front().size() >= 2 &&
    this->dataPtr->path.front()[1] == ':')
  {
    result = "";
  }

  bool firstPart = true;
  for (auto const &part : this->dataPtr->path)
  {
    if (firstPart)
    {
      firstPart = false;
    }
    else
    {
      result += _delim;
    }
    result += part;
  }

  if (this->dataPtr->trailingSlash)
    result += "/";

  return result;
}

/////////////////////////////////////////////////
void URIPath::Clear()
{
  this->dataPtr->path.clear();
  this->dataPtr->isAbsolute = false;
  this->dataPtr->trailingSlash = false;
}

/////////////////////////////////////////////////
bool URIPath::operator==(const URIPath &_path) const
{
  return this->dataPtr->path == _path.dataPtr->path &&
         this->dataPtr->isAbsolute == _path.dataPtr->isAbsolute;
}

/////////////////////////////////////////////////
bool URIPath::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIPath::Valid(const std::string &_str)
{
  auto str = trimmed(_str);

  // All spaces is not allowed.
  if (str.empty() && !_str.empty())
    return false;

  // TODO(anyone): the space should not be
  // there, but leaving it out breaks
  // other stuff, e.g. gz-fuel-tools
  // now have URIs with unencoded spaces
  const std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                                   "QWERTZUIOPASDFGHJKLYXCVBNM"
                                   "0123456789"
                                   "/"
                                   ":@"
                                   "%"
                                   "-._~"
                                   "!$&'()*+,;="
                                   "[] ";
  if (str.find_first_not_of(allowedChars) != std::string::npos)
    return false;

  const std::string allowedCharsFirst = "qwertzuiopasdfghjklyxcvbnm"
                                        "QWERTZUIOPASDFGHJKLYXCVBNM"
                                        "0123456789"
                                        ":"
                                        "%"
                                        // "-._~"  // is in RFC, weird
                                        "+"  // "!$&'()*,;=" // is in RFC, weird
                                        "[/";
  if (str.substr(0, 1).find_first_not_of(allowedCharsFirst) !=
      std::string::npos)
    return false;

  return true;
}

/////////////////////////////////////////////////
bool URIPath::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  for (auto part : split(_str, "/"))
    this->PushBack(part);

  // the initial / is removed from _str when splitting, so we need to
  // explicitly check for it
  this->dataPtr->isAbsolute = this->dataPtr->IsStringAbsolute(_str);
  this->dataPtr->trailingSlash =
    _str.size() > 1 &&_str.back() == '/';

  return true;
}

/////////////////////////////////////////////////
URIQuery::URIQuery()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
URIQuery::URIQuery(const std::string &_str)
  : URIQuery()
{
  if (!this->Parse(_str))
  {
    gzwarn << "Unable to parse URIQuery [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
void URIQuery::Insert(const std::string &_key, const std::string &_value)
{
  this->dataPtr->values.push_back(std::make_pair(_key, _value));
}

/////////////////////////////////////////////////
std::string URIQuery::Str(const std::string &_delim) const
{
  if (this->dataPtr->values.empty())
      return "";

  std::string result = "?";
  for (const std::pair<std::string, std::string> &value : this->dataPtr->values)
  {
    if (result != "?")
      result += _delim;
    if (!value.second.empty())
      result += value.first + "=" + value.second;
    else
      result += value.first;
  }

  return result;
}

/////////////////////////////////////////////////
void URIQuery::Clear()
{
  this->dataPtr->values.clear();
}

/////////////////////////////////////////////////
bool URIQuery::operator==(const URIQuery &_query) const
{
  return this->Str() == _query.Str();
}

/////////////////////////////////////////////////
bool URIQuery::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIQuery::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  if (str.empty())
    return true;

  if (str[0] != '?')
    return false;

  // ABNF for query from RFC3986:
  // query         = *( pchar / "/" / "?" )
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
  // pct-encoded   = "%" HEXDIG HEXDIG
  // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
  //                 / "*" / "+" / "," / ";" / "="

  std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                             "QWERTZUIOPASDFGHJKLYXCVBNM"
                             "0123456789"
                             "/?"
                             ":@"
                             "%"
                             "-._~"
                             "!$&'()*+,;=";
  if (str.find_first_not_of(allowedChars, 1) != std::string::npos)
    return false;

  return true;
}

/////////////////////////////////////////////////
bool URIQuery::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  if (!_str.empty())
  {
    for (const std::string &query : split(_str.substr(1), "&"))
    {
      std::vector<std::string> values = split(query, "=");
      if (values.size() == 2)
        this->Insert(values.at(0), values.at(1));
      else
        this->Insert(query, "");
    }
  }

  return true;
}

/////////////////////////////////////////////////
URIFragment::URIFragment()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
URIFragment::URIFragment(const std::string &_str)
  : URIFragment()
{
  if (!this->Parse(_str))
  {
    gzwarn << "Unable to parse URIFragment [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
URIFragment &URIFragment::operator=(const std::string &_fragment)
{
  this->Parse(_fragment);
  return *this;
}

/////////////////////////////////////////////////
std::string URIFragment::Str() const
{
  if (this->dataPtr->value.empty())
    return "";
  return "#" + this->dataPtr->value;
}

/////////////////////////////////////////////////
void URIFragment::Clear()
{
  this->dataPtr->value.clear();
}

/////////////////////////////////////////////////
bool URIFragment::operator==(const URIFragment &_fragment) const
{
  return this->Str() == _fragment.Str();
}

/////////////////////////////////////////////////
bool URIFragment::Valid() const
{
  return URIFragment::Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIFragment::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  if (str.empty())
    return true;

  if (str[0] != '#')
    return false;

  // ABNF for fragment from RFC3986:
  // fragment      = *( pchar / "/" / "?" )
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
  // pct-encoded   = "%" HEXDIG HEXDIG
  // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
  //                 / "*" / "+" / "," / ";" / "="

  std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                             "QWERTZUIOPASDFGHJKLYXCVBNM"
                             "0123456789"
                             "/?"
                             ":@"
                             "%"
                             "-._~"
                             "!$&'()*+,;=";
  if (str.find_first_not_of(allowedChars, 1) != std::string::npos)
    return false;

  return true;
}

/////////////////////////////////////////////////
bool URIFragment::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  if (!_str.empty())
  {
    this->dataPtr->value = _str.substr(1);
  }

  return true;
}

/////////////////////////////////////////////////
URI::URI()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
URI::URI(const std::string &_str, bool _hasAuthority)
  : URI()
{
  if (_hasAuthority)
    this->dataPtr->authority.emplace(URIAuthority());

  std::string str = _str;
  if ((str.size() > 7 && 0 == str.compare(0, 7, "http://")) ||
      (str.size() > 8 && 0 == str.compare(0, 8, "https://")))
  {
    str = std::regex_replace(str, std::regex(R"(\\)"), "/");
  }

  this->Parse(_str);
}

//////////////////////////////////////////////////
std::string URI::Str() const
{
  std::string result =
    this->dataPtr->scheme.empty() ? "" : this->dataPtr->scheme + kSchemeDelim;

  if (this->dataPtr->authority)
  {
    result += this->dataPtr->authority->Str() + this->dataPtr->path.Str();
  }
  else
  {
    if (!this->dataPtr->scheme.empty())
    {
      result += kAuthDelim;
    }
    result += this->dataPtr->path.Str();
  }

  result += this->dataPtr->query.Str() +
            this->dataPtr->fragment.Str();
  return result;
}

/////////////////////////////////////////////////
std::string URI::Scheme() const
{
  return this->dataPtr->scheme;
}

/////////////////////////////////////////////////
void URI::SetScheme(const std::string &_scheme)
{
  this->dataPtr->scheme = _scheme;
}

/////////////////////////////////////////////////
void URI::SetAuthority(const URIAuthority &_authority)
{
  this->dataPtr->authority.emplace(_authority);
}

/////////////////////////////////////////////////
std::optional <URIAuthority> URI::Authority() const
{
  return this->dataPtr->authority;
}

/////////////////////////////////////////////////
URIPath &URI::Path()
{
  return this->dataPtr->path;
}

/////////////////////////////////////////////////
const URIPath &URI::Path() const
{
  return this->dataPtr->path;
}

/////////////////////////////////////////////////
URIQuery &URI::Query()
{
  return this->dataPtr->query;
}

/////////////////////////////////////////////////
const URIQuery &URI::Query() const
{
  return this->dataPtr->query;
}

/////////////////////////////////////////////////
URIFragment &URI::Fragment()
{
  return this->dataPtr->fragment;
}

/////////////////////////////////////////////////
const URIFragment &URI::Fragment() const
{
  return this->dataPtr->fragment;
}

/////////////////////////////////////////////////
void URI::Clear()
{
  this->dataPtr->scheme.clear();
  // Set to empty instead of removing value
  if (this->dataPtr->authority)
    this->dataPtr->authority = URIAuthority();
  this->dataPtr->path.Clear();
  this->dataPtr->query.Clear();
  this->dataPtr->fragment.Clear();
}

/////////////////////////////////////////////////
bool URI::operator==(const URI &_uri) const
{
  return this->dataPtr->scheme == _uri.dataPtr->scheme &&
         this->dataPtr->authority == _uri.dataPtr->authority &&
         this->dataPtr->path == _uri.dataPtr->path &&
         this->dataPtr->query == _uri.dataPtr->query &&
         this->dataPtr->fragment == _uri.dataPtr->fragment;
}

/////////////////////////////////////////////////
bool URI::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URI::Valid(const std::string &_str)
{
  auto str = trimmed(_str);

  // Validate scheme.
  size_t schemeDelimPos = str.find(kSchemeDelim);

  if ((str.empty()) ||
      (schemeDelimPos == std::string::npos) ||
      (schemeDelimPos == 0u))
  {
    return false;
  }

  std::string localScheme;
  std::string localAuthority;
  std::string localPath;
  std::string localQuery;
  std::string localFragment;

  localScheme = str.substr(0, schemeDelimPos);
  str.erase(0, schemeDelimPos + std::strlen(kSchemeDelim));

  bool emptyHostValid = false;
  if (localScheme == "file")
    emptyHostValid = true;

  bool authorityPresent = false;

  // Get the authority delimiter position, if one is present
  size_t authDelimPos = str.find(kAuthDelim);
  if (authDelimPos != std::string::npos && authDelimPos == 0)
  {
    authorityPresent = true;
    size_t authEndPos = str.find_first_of("/?#",
        authDelimPos + std::strlen(kAuthDelim));

    if (localScheme != "file" && authEndPos == authDelimPos+2)
    {
      gzerr << "A host is mandatory when using a scheme other than file\n";
      return false;
    }
    else
    {
      localAuthority = str.substr(authDelimPos, authEndPos);
      str.erase(0, authEndPos);
    }
  }

  // Get the path information
  size_t pathEndPos = str.find_first_of("?#");
  localPath = str.substr(0, pathEndPos);
  str.erase(0, pathEndPos);

  size_t queryStartPos = str.find("?");
  if (queryStartPos != std::string::npos)
  {
    size_t queryEndPos = str.find("#");
    // Get the query.
    localQuery = str.substr(0, queryEndPos);
    str.erase(0, queryEndPos);
  }

  size_t fragStartPos = str.find("#");
  if (fragStartPos != std::string::npos)
  {
    // Get the query.
    localFragment = str;
  }

  if (!emptyHostValid || authorityPresent)
  {
    if (!URIAuthority::Valid(localAuthority, emptyHostValid))
      return false;
  }

  // Validate the path and query.
  return URIPath::Valid(localPath) &&
         URIQuery::Valid(localQuery) &&
         URIFragment::Valid(localFragment);
}

/////////////////////////////////////////////////
bool URI::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  // Copy the string so that we can modify it.
  std::string str = _str;

  std::string localScheme;
  std::string localAuthority;
  std::string localPath;
  std::string localQuery;
  std::string localFragment;

  size_t schemeDelimPos = str.find(kSchemeDelim);
  localScheme = str.substr(0, schemeDelimPos);
  str.erase(0, schemeDelimPos + std::strlen(kSchemeDelim));

  bool emptyHostValid = false;
  if (localScheme == "file")
    emptyHostValid = true;

  bool authorityPresent = false;
  if (this->dataPtr->authority)
  {
    // Get the authority delimiter position, if one is present
    size_t authDelimPos = str.find(kAuthDelim);
    if (authDelimPos != std::string::npos && authDelimPos == 0)
    {
      authorityPresent = true;
      size_t authEndPos = str.find_first_of("/?#",
          authDelimPos + std::strlen(kAuthDelim));

      if (authEndPos != std::string::npos &&
          (str[authEndPos-1] == ':' && str[authEndPos] == '/'))
      {
      // This could be a windows file path of the form file://D:/path/to/file
      // In this case, the authority is not present.
        str.erase(0, std::strlen(kAuthDelim));
        authorityPresent = false;
      }
      else if (localScheme != "file" &&
               authEndPos == authDelimPos + std::strlen(kAuthDelim))
      {
        gzerr << "A host is manadatory when using a scheme other than file\n";
        return false;
      }
      else
      {
        localAuthority = str.substr(authDelimPos, authEndPos);
        str.erase(0, authEndPos);
      }
    }
  }
  else
  {
    // Relative paths: remove `//` prefix if there are exactly 2
    if (str.find("//") == 0 && str.find("///") == std::string::npos)
    {
      str = str.substr(2);
    }
  }

  // Get the path information
  size_t pathEndPos = str.find_first_of("?#");
  localPath = str.substr(0, pathEndPos);
  str.erase(0, pathEndPos);

  size_t queryStartPos = str.find("?");
  if (queryStartPos != std::string::npos)
  {
    size_t queryEndPos = str.find("#");
    // Get the query.
    localQuery = str.substr(0, queryEndPos);
    str.erase(0, queryEndPos);
  }

  size_t fragStartPos = str.find("#");
  if (fragStartPos != std::string::npos)
  {
    // Get the query.
    localFragment = str;
  }

  this->Clear();
  this->SetScheme(localScheme);

  if (this->dataPtr->authority && (!emptyHostValid || authorityPresent))
  {
    if (!this->dataPtr->authority->Parse(localAuthority, emptyHostValid))
      return false;
  }

  return this->dataPtr->path.Parse(localPath) &&
         this->dataPtr->query.Parse(localQuery) &&
         this->dataPtr->fragment.Parse(localFragment);
}
