//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//
//  Copyright (C) 2004-2017 René Nyffenegger
//

#pragma once

#include <string>
#include <vector>

/// \brief Takes a binary input and outputs a string in base64
/// \param[in] _bytesToEncode binary data to encode
/// \param[in] _len length in bytes of _bytesToEncode
/// \param[out] _outBase64 String with base64-encoded of input
void Base64Encode(const void *_bytesToEncode, size_t _len,
                  std::string &_outBase64);

/// \brief Takes a base64-encoded string and turns it back into binary
/// \param _s base64-encoded string
/// \return Decoded binary data
std::vector<uint8_t> Base64Decode(const std::string &_s);
