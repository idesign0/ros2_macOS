/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include <cstdlib>
#include <iostream>
#include <filesystem>

#include "get_install_prefix_test_shared.h"
#include "get_install_prefix_test_static.h"
#include <get_install_prefix_test_cmake_variables.h>

std::string toCanonical(const std::string input_path)
{
    return std::filesystem::weakly_canonical(std::filesystem::path(input_path)).string();
}

int main()
{
    // Test nominal behaviour

    std::string sharedInstallPrefix = gz::cmake::test::sharedlib::getInstallPrefix();
    std::string staticInstallPrefix = gz::cmake::test::staticlib::getInstallPrefix();

    std::cerr << "get-install-prefix test:" << std::endl;
    std::cerr << "sharedInstallPrefix: " << sharedInstallPrefix << std::endl;
    std::cerr << "CMAKE_BINARY_DIR: " << CMAKE_BINARY_DIR << std::endl;
    std::cerr << "staticInstallPrefix: " << staticInstallPrefix << std::endl;
    std::cerr << "CMAKE_INSTALL_PREFIX: " << CMAKE_INSTALL_PREFIX << std::endl;

    if (toCanonical(sharedInstallPrefix) != toCanonical(CMAKE_BINARY_DIR))
    {
        std::cerr << "getInstallPrefixShared returned unexpected value, test is failing." << std::endl;
        return EXIT_FAILURE;
    }

    if (toCanonical(staticInstallPrefix) != toCanonical(CMAKE_INSTALL_PREFIX))
    {
        std::cerr << "getInstallPrefixStatic returned unexpected value, test is failing." << std::endl;
        return EXIT_FAILURE;
    }

    // Test behaviour after setting the environment variable to modify the return values (only on Unix so we can use setenv)
#ifndef _WIN32
    std::string overrideValue = "test_override_value";
    int overwrite = 1;
    setenv("GET_INSTALL_PREFIX_TEST_INSTALL_PREFIX" , overrideValue.c_str(), overwrite);
    std::string sharedInstallPrefixWithOverride = gz::cmake::test::sharedlib::getInstallPrefix();
    std::string staticInstallPrefixWithOverride = gz::cmake::test::staticlib::getInstallPrefix();

    std::cerr << "overrideValue: " << overrideValue << std::endl;
    std::cerr << "sharedInstallPrefixWithOverride: " << sharedInstallPrefixWithOverride << std::endl;
    std::cerr << "staticInstallPrefixWithOverride: " << staticInstallPrefixWithOverride << std::endl;

    if (overrideValue != sharedInstallPrefixWithOverride)
    {
        std::cerr << "getInstallPrefixShared with env variable override returned unexpected value, test is failing." << std::endl;
        return EXIT_FAILURE;
    }

    if (overrideValue != sharedInstallPrefixWithOverride)
    {
        std::cerr << "getInstallPrefixShared with env variable override returned unexpected value, test is failing." << std::endl;
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}
