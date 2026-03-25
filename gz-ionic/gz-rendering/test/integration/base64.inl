/*
base64.cpp and base64.h

base64 encoding and decoding with C++.

Version: 1.01.00

Copyright (C) 2004-2017 René Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include "base64.hh"

#include <iostream>

static const std::string kBase64Chars =  // NOLINT
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

static inline bool is_base64(char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

void Base64Encode(const void *_bytesToEncode, size_t _len,
                  std::string &_outBase64)
{
  uint8_t const *bytesToEncode =
    reinterpret_cast<uint8_t const *>(_bytesToEncode);

  int i = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (_len--)
  {
    char_array_3[i++] = *(bytesToEncode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = static_cast<uint8_t>(((char_array_3[0] & 0x03) << 4) +
                                             ((char_array_3[1] & 0xf0) >> 4));
      char_array_4[2] = static_cast<uint8_t>(((char_array_3[1] & 0x0f) << 2) +
                                             ((char_array_3[2] & 0xc0) >> 6));
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
      {
        _outBase64.push_back(kBase64Chars[char_array_4[i]]);
      }
      i = 0;
    }
  }

  if (i)
  {
    int j = 0;
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = static_cast<uint8_t>(((char_array_3[0] & 0x03) << 4) +
                                           ((char_array_3[1] & 0xf0) >> 4));
    char_array_4[2] = static_cast<uint8_t>(((char_array_3[1] & 0x0f) << 2) +
                                           ((char_array_3[2] & 0xc0) >> 6));

    for (j = 0; (j < i + 1); j++)
    {
      _outBase64.push_back(kBase64Chars[char_array_4[j]]);
    }

    while ((i++ < 3))
    {
      _outBase64.push_back('=');
    }
  }
}

std::vector<uint8_t> Base64Decode(const std::string &_s)
{
  int in_len = static_cast<int>(_s.size());
  int i = 0;
  size_t in_ = 0u;
  uint8_t char_array_4[4], char_array_3[3];
  std::vector<uint8_t> ret;

  while (in_len-- && (_s[in_] != '=') && is_base64(_s[in_]))
  {
    char_array_4[i++] = static_cast<uint8_t>(_s[in_]);
    in_++;
    if (i == 4)
    {
      for (i = 0; i < 4; i++)
      {
        char_array_4[i] = static_cast<uint8_t>(
          kBase64Chars.find(static_cast<char>(char_array_4[i])));
      }

      char_array_3[0] = static_cast<uint8_t>((char_array_4[0] << 2) +
                                             ((char_array_4[1] & 0x30) >> 4));
      char_array_3[1] = static_cast<uint8_t>(((char_array_4[1] & 0xf) << 4) +
                                             ((char_array_4[2] & 0x3c) >> 2));
      char_array_3[2] =
        static_cast<uint8_t>(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

      for (i = 0; (i < 3); i++)
      {
        ret.push_back(char_array_3[i]);
      }
      i = 0;
    }
  }

  if (i)
  {
    int j = 0;
    for (j = 0; j < i; j++)
    {
      char_array_4[j] = static_cast<uint8_t>(static_cast<char>(
        kBase64Chars.find(static_cast<char>(char_array_4[j]))));
    }

    char_array_3[0] = static_cast<uint8_t>((char_array_4[0] << 2) +
                                           ((char_array_4[1] & 0x30) >> 4));
    char_array_3[1] = static_cast<uint8_t>(((char_array_4[1] & 0xf) << 4) +
                                           ((char_array_4[2] & 0x3c) >> 2));

    for (j = 0; (j < i - 1); j++)
    {
      ret.push_back(char_array_3[j]);
    }
  }

  return ret;
}
