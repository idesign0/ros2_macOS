# Copyright (C) 2023 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#################################################
# gz_find_package(<PACKAGE_NAME>
#                 [REQUIRED] [PRIVATE] [EXACT] [QUIET] [CONFIG] [BUILD_ONLY] [PKGCONFIG_IGNORE]
#                 [COMPONENTS <components_of_PACKAGE_NAME>]
#                 [OPTIONAL_COMPONENTS <components_of_PACKAGE_NAME>]
#                 [REQUIRED_BY <components_of_project>]
#                 [PRIVATE_FOR <components_of_project>]
#                 [VERSION <ver>]
#                 [EXTRA_ARGS <args>]
#                 [PRETTY <name>]
#                 [PURPOSE <"explanation for this dependency">]
#                 [PKGCONFIG <pkgconfig_name>]
#                 [PKGCONFIG_LIB <lib_name>]
#                 [PKGCONFIG_VER_COMPARISON  <  >  =  <=  >= ])
#
# This is a wrapper for the standard cmake find_package which behaves according
# to the conventions of the Gazebo library. In particular, we do not quit
# immediately when a required package is missing. Instead, we check all
# dependencies and provide an overview of what is missing at the end of the
# configuration process. Descriptions of the function arguments are as follows:
#
# <PACKAGE_NAME>: The name of the package as it would normally be passed to
#                 find_package(~). Note if your package corresponds to a
#                 find-module named FindABC.cmake, then <PACKAGE_NAME> must be
#                 ABC, with the case matching. If the find-module is named
#                 FindAbc.cmake, then <PACKAGE_NAME> must be Abc. This will not
#                 necessarily match the library's actual name, nor will it
#                 necessarily match the name used by pkgconfig, so there are
#                 additional arguments (i.e. PRETTY, PKGCONFIG) to specify
#                 alternative names for this package that can be used depending
#                 on the context.
#
# [REQUIRED]: Optional. If provided, macro will trigger a Gazebo build_error
#             when the package cannot be found. If not provided, this macro will
#             trigger a Gazebo build_warning when the package is not found.
#             To specify that something is required by some set of components
#             (rather than the core library), use REQUIRED_BY.
#
# [PRIVATE]: Optional. Use this to indicate that consumers of the project do not
#            need to link against the package, but it must be present on the
#            system, because our project must link against it.
#
# [EXACT]: Optional. This will pass on the EXACT option to find_package(~) and
#          also add it to the call to find_dependency(~) in the
#          <project>-config.cmake file.
#
# [QUIET]: Optional. If provided, it will be passed forward to cmake's
#          find_package(~) command. This macro will still print its normal
#          output, except there will be no warning if the package is missing,
#          unless REQUIRED or REQUIRED_BY is specified.
#
# [CONFIG]: Optional. If provided, it will be passed forward to cmake's
#          find_package(~) command. This will trigger Config mode search rather than
#          Module mode.
#
# [BUILD_ONLY]: Optional. Use this to indicate that the project only needs this
#               package while building, and it does not need to be available to
#               the consumer of this project at all. Normally this should only
#               apply to (1) a header-only library whose headers are included
#               exclusively in the source files and not included in any public
#               (i.e. installed) project headers, or to (2) a static library
#               dependency.
#
# [PKGCONFIG_IGNORE]: Discouraged. If this option is provided, this package will
#                     not be added to the project's pkgconfig file in any way.
#                     This should only be used in very rare circumstances. Note
#                     that BUILD_ONLY will also prevent a pkgconfig entry from
#                     being produced.
#
# [COMPONENTS]: Optional. If provided, the list that follows it will be passed
#               to find_package(~) to indicate which components of PACKAGE_NAME
#               are considered to be dependencies of either this project
#               (specified by REQUIRED) or this project's components (specified
#               by REQUIRED_BY). This is effectively the same as the
#               find_package( ... COMPONENTS <components>) argument.
#
# [REQUIRED_BY]: Optional. If provided, the list that follows it must indicate
#                which library components require the dependency. Note that if
#                REQUIRED is specified, then REQUIRED_BY does NOT need to be
#                specified for any components which depend on the core library,
#                because their dependence on this package will effectively be
#                inherited from the core library. This will trigger a build
#                warning to tell the user which component requires this
#                dependency.
#
# [PRIVATE_FOR]: Optional. If provided, the list that follows it must indicate
#                which library components depend on this package privately (i.e.
#                the package should not be included in its list of interface
#                libraries). This is only relevant for components that follow
#                the REQUIRED_BY command. Note that the PRIVATE argument does
#                not apply to components specified by REQUIRED_BY. This argument
#                MUST be given for components whose private dependencies have
#                been specified with REQUIRED_BY.
#
# [VERSION]: Optional. Follow this argument with the major[.minor[.patch[.tweak]]]
#            version that you need for this package.
#
# [EXTRA_ARGS]: Optional. Additional args to pass forward to find_package(~)
#
# [PRETTY]: Optional. If provided, the string that follows will replace
#           <PACKAGE_NAME> when printing messages, warnings, or errors to the
#           terminal.
#
# [PURPOSE]: Optional. If provided, the string that follows will be appended to
#            the build_warning or build_error that this function produces when
#            the package could not be found.
#
#  ==========================================================================
#  The following arguments pertain to the automatic generation of your
#  project's pkgconfig file. Ideally, this information should be provided
#  automatically by gz-cmake through the cmake find-module that is written
#  for your dependency. However, if your package gets distributed with its own
#  cmake config-file or find-module, then it might not automatically set this
#  information. Therefore, we provide the ability to set it through your call to
#  gz_find_package(~). These arguments can also be used to overwrite the
#  pkg-config entries that get generated by the gz-cmake find-module for the
#  package. Do not hesitate to ask for help if you need to use these arguments.
#
# [PKGCONFIG]: Optional. If provided, the string that follows will be used to
#              specify a "required package" for pkgconfig. Note that the option
#              PKGCONFIG_LIB has higher precedence than this option.
#
# [PKGCONFIG_LIB]: Optional. Use this to indicate that the package should be
#                  considered a "library" by pkgconfig. This is used for
#                  libraries which do not come with *.pc metadata, such as
#                  system libraries, libm, libdl, or librt. Generally you should
#                  leave this out, because most packages will be considered
#                  "modules" by pkgconfig. The string which follows this
#                  argument will be used as the library name, and the string
#                  that follows a PKGCONFIG argument will be ignored, so the
#                  PKGCONFIG argument can be left out when using this argument.
#
# [PKGCONFIG_VER_COMPARISON]: Optional. If provided, pkgconfig will be told how
#                             the available version of this package must compare
#                             to the specified version. Acceptable values are
#                             =, <, >, <=, >=. Default will be =. If no version
#                             is provided using VERSION, then this will be left
#                             out, whether or not it is provided.
#
macro(gz_find_package PACKAGE_NAME_)
  set(PACKAGE_NAME ${PACKAGE_NAME_})  # Allow for variable rebinds

  # Define the expected arguments
  set(options REQUIRED PRIVATE EXACT QUIET CONFIG BUILD_ONLY PKGCONFIG_IGNORE)
    set(oneValueArgs VERSION PRETTY PURPOSE PKGCONFIG PKGCONFIG_LIB PKGCONFIG_VER_COMPARISON)
    set(multiValueArgs REQUIRED_BY PRIVATE_FOR COMPONENTS OPTIONAL_COMPONENTS EXTRA_ARGS)

  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_find_package "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #------------------------------------
  # Construct the arguments to pass to find_package
  set(${PACKAGE_NAME}_find_package_args ${PACKAGE_NAME})

  if(gz_find_package_VERSION)
    list(APPEND ${PACKAGE_NAME}_find_package_args ${gz_find_package_VERSION})
  endif()

  if(gz_find_package_QUIET)
    list(APPEND ${PACKAGE_NAME}_find_package_args QUIET)
  endif()

  if(gz_find_package_EXACT)
    list(APPEND ${PACKAGE_NAME}_find_package_args EXACT)
  endif()

  if(gz_find_package_CONFIG)
    list(APPEND ${PACKAGE_NAME}_find_package_args CONFIG)
  endif()

  if(gz_find_package_COMPONENTS)
    list(APPEND ${PACKAGE_NAME}_find_package_args COMPONENTS ${gz_find_package_COMPONENTS})
  endif()

  if(gz_find_package_OPTIONAL_COMPONENTS)
    list(APPEND ${PACKAGE_NAME}_find_package_args OPTIONAL_COMPONENTS ${gz_find_package_OPTIONAL_COMPONENTS})
  endif()

  if(gz_find_package_EXTRA_ARGS)
    list(APPEND ${PACKAGE_NAME}_find_package_args ${gz_find_package_EXTRA_ARGS})
  endif()


  #------------------------------------
  # Call find_package with the provided arguments
  find_package(${${PACKAGE_NAME}_find_package_args})


  #------------------------------------
  # Figure out which name to print
  if(gz_find_package_PRETTY)
    set(${PACKAGE_NAME}_pretty ${gz_find_package_PRETTY})
  else()
    set(${PACKAGE_NAME}_pretty ${PACKAGE_NAME})
  endif()


  if(${PACKAGE_NAME}_FOUND)

    message(STATUS "Looking for ${${PACKAGE_NAME}_pretty} - found\n")

  else()

    message(STATUS "Looking for ${${PACKAGE_NAME}_pretty} - not found\n")

    #------------------------------------
    # Construct the warning/error message to produce
    set(${PACKAGE_NAME}_msg "Missing dependency [${${PACKAGE_NAME}_pretty}]")

    if(gz_find_package_COMPONENTS)
      _gz_list_to_string(comp_str gz_find_package_COMPONENTS DELIM ", ")
      set(${PACKAGE_NAME}_msg "${${PACKAGE_NAME}_msg} (Components: ${comp_str})")
    endif()

    if(DEFINED gz_find_package_PURPOSE)
      set(${PACKAGE_NAME}_msg "${${PACKAGE_NAME}_msg} - ${gz_find_package_PURPOSE}")
    endif()

    #------------------------------------
    # If the package is unavailable, tell the user.
    if(gz_find_package_REQUIRED)

      # If it was required by the project, we will create an error.
      gz_build_error(${${PACKAGE_NAME}_msg})

    elseif(gz_find_package_REQUIRED_BY)

      foreach(component ${gz_find_package_REQUIRED_BY})

        if(NOT SKIP_${component})
          # Otherwise, if it was only required by some of the components, create
          # a warning about which components will not be available, unless the
          # user explicitly requested that it be skipped
          gz_build_warning("Skipping component [${component}]: ${${PACKAGE_NAME}_msg}.\n    ^~~~~ Set SKIP_${component}=true in cmake to suppress this warning.\n ")

          # Create a variable to indicate that we need to skip the component
          set(INTERNAL_SKIP_${component} true)

          # Track the missing dependencies
          gz_string_append(${component}_MISSING_DEPS "${${PACKAGE_NAME}_pretty}" DELIM ", ")
        endif()

      endforeach()

    else()
      if(NOT gz_find_package_QUIET)
        gz_build_warning(${${PACKAGE_NAME}_msg})
      endif()
    endif()

  endif()


  #------------------------------------
  # Add this package to the list of dependencies that will be inserted into the
  # find-config file, unless the invoker specifies that it should not be added.
  # Also, add this package or library as an entry to the pkgconfig file that we
  # will produce for our project.
  if( ${PACKAGE_NAME}_FOUND
      AND (gz_find_package_REQUIRED OR gz_find_package_REQUIRED_BY)
      AND NOT gz_find_package_BUILD_ONLY)

    # Set up the arguments we want to pass to the find_dependency invokation for
    # our Gazebo project. We always need to pass the name of the dependency.
    #
    # NOTE: We escape the dollar signs because we want those variable
    #       evaluations to be a part of the string that we produce. It is going
    #       to be put into a *-config.cmake file. Those variables determine
    #       whether the find_package(~) call will be REQUIRED and/or QUIET.
    #
    # TODO: When we migrate to cmake-3.9+, this can be removed because calling
    #       find_dependency(~) will automatically forward these properties.
    set(${PACKAGE_NAME}_dependency_args "${PACKAGE_NAME}")

    # If a version is provided here, we should pass that as well.
    if(gz_find_package_VERSION)
      gz_string_append(${PACKAGE_NAME}_dependency_args ${gz_find_package_VERSION})
    endif()

    # If we have specified the exact version, we should provide that as well.
    if(gz_find_package_EXACT)
      gz_string_append(${PACKAGE_NAME}_dependency_args EXACT)
    endif()

    # If we have specified to use CONFIG mode, we should provide that as well.
    if(gz_find_package_CONFIG)
      gz_string_append(${PACKAGE_NAME}_dependency_args CONFIG)
    endif()

    # NOTE (MXG): 7 seems to be the number of escapes required to get
    # "${gz_package_required}" and "${gz_package_quiet}" to show up correctly
    # as strings in the final config-file outputs. It is unclear to me why the
    # escapes get collapsed exactly three times, so it is possible that any
    # changes to this script could cause a different number of escapes to be
    # necessary. Please use caution when modifying this script.
    gz_string_append(${PACKAGE_NAME}_dependency_args "\\\\\\\${gz_package_quiet} \\\\\\\${gz_package_required}")

    # If we have specified components of the dependency, mention those.
    if(gz_find_package_COMPONENTS)
      gz_string_append(${PACKAGE_NAME}_dependency_args "COMPONENTS ${gz_find_package_COMPONENTS}")
    endif()

    # If there are any additional arguments for the find_package(~) command,
    # forward them along.
    if(gz_find_package_EXTRA_ARGS)
      gz_string_append(${PACKAGE_NAME}_dependency_args "${gz_find_package_EXTRA_ARGS}")
    endif()

    # TODO: When we migrate to cmake-3.9+ bring back find_dependency(~) because
    #       at that point it will be able to support COMPONENTS and EXTRA_ARGS
#    set(${PACKAGE_NAME}_find_dependency "find_dependency(${${PACKAGE_NAME}_dependency_args})")

    set(${PACKAGE_NAME}_find_dependency "find_package(${${PACKAGE_NAME}_dependency_args})")


    if(gz_find_package_REQUIRED)
      # If this is REQUIRED, add it to PROJECT_CMAKE_DEPENDENCIES
      gz_string_append(PROJECT_CMAKE_DEPENDENCIES "${${PACKAGE_NAME}_find_dependency}" DELIM "\n")
    endif()

    if(gz_find_package_REQUIRED_BY)

      # Identify which components are privately requiring this package
      foreach(component ${gz_find_package_PRIVATE_FOR})
        set(${component}_${PACKAGE_NAME}_PRIVATE true)
      endforeach()

      # If this is required by some components, add it to the
      # ${component}_CMAKE_DEPENDENCIES variables that are specific to those
      # componenets
      foreach(component ${gz_find_package_REQUIRED_BY})
        if(NOT ${component}_${PACKAGE_NAME}_PRIVATE)
          gz_string_append(${component}_CMAKE_DEPENDENCIES "${${PACKAGE_NAME}_find_dependency}" DELIM "\n")
        endif()
      endforeach()

    endif()

    #------------------------------------
    # Add this library or project to its relevant pkgconfig entry, unless we
    # have been explicitly instructed to ignore it.
    if(NOT gz_find_package_PKGCONFIG_IGNORE)

      # Here we will set up the pkgconfig entry for this package. Ordinarily,
      # these variables should be set by the gz-cmake custom find-module for
      # the package which should use gz_pkg_check_modules[_quiet] or
      # gz_pkg_config_library_entry. However, that will not be performed by
      # third-party dependencies that provide their own find-module or their own
      # cmake config-module. Therefore, we provide the option of specifying
      # pkgconfig information through the call to gz_find_package. This also
      # allows callers of gz_find_package(~) to overwrite the default
      # pkg-config entry that gets generated by the gz-cmake find-modules.

      # If the caller has specified the arguments PKGCONFIG_LIB or PKGCONFIG,
      # then we will overwrite these pkgconfig variables with the information
      # provided by the caller.
      if(gz_find_package_PKGCONFIG_LIB)

        # Libraries must be prepended with -l
        set(${PACKAGE_NAME}_PKGCONFIG_ENTRY "-l${gz_find_package_PKGCONFIG_LIB}")
        set(${PACKAGE_NAME}_PKGCONFIG_TYPE PKGCONFIG_LIBS)

      elseif(gz_find_package_PKGCONFIG)

        # Modules (a.k.a. packages) can just be specified by their package
        # name without any prefixes like -l
        set(${PACKAGE_NAME}_PKGCONFIG_ENTRY "${gz_find_package_PKGCONFIG}")
        set(${PACKAGE_NAME}_PKGCONFIG_TYPE PKGCONFIG_REQUIRES)

        # Add the version requirements to the entry.
        if(gz_find_package_VERSION)
          # Use equivalency by default
          set(comparison "=")

          # If the caller has specified a version comparison operator, use that
          # instead of equivalency.
          if(gz_find_package_PKGCONFIG_VER_COMPARISON)
            set(comparison ${gz_find_package_PKGCONFIG_VER_COMPARISON})
          endif()

          # Append the comparison and the version onto the pkgconfig entry
          set(${PACKAGE_NAME}_PKGCONFIG_ENTRY "${${PACKAGE_NAME}_PKGCONFIG_ENTRY} ${comparison} ${gz_find_package_VERSION}")

        endif()

      endif()

      if(NOT ${PACKAGE_NAME}_PKGCONFIG_ENTRY)

        # The find-module has not provided a default pkg-config entry for this
        # package, and the caller of gz_find_package(~) has not explicitly
        # provided pkg-config information. The caller has also not specified
        # PKGCONFIG_IGNORE. This means that the requirements of this package
        # will be unintentionally omitted from the auto-generated
        # gz-<project>.pc file. This is probably an oversight in our build
        # system scripts, so we will emit a warning about this.
        message(AUTHOR_WARNING
          " -- THIS MESSAGE IS INTENDED FOR GZ-${GZ_DESIGNATION_UPPER} AUTHORS --\n"
          "    (IF YOU SEE THIS, PLEASE REPORT IT)\n"
          "Could not find pkg-config information for ${PACKAGE_NAME}. "
          "It was not provided by the find-module for the package, nor was it "
          "explicitly passed into the call to gz_find_package(~). This is "
        "most likely an error in this project's use of gz-cmake.")

      else()

        # We have pkg-config information for this package

        if(gz_find_package_REQUIRED)

          if(gz_find_package_PRIVATE)
            # If this is a private library or module, use the _PRIVATE suffix
            set(PROJECT_${PACKAGE_NAME}_PKGCONFIG_TYPE ${${PACKAGE_NAME}_PKGCONFIG_TYPE}_PRIVATE)
          else()
            # Otherwise, use the plain type
            set(PROJECT_${PACKAGE_NAME}_PKGCONFIG_TYPE ${${PACKAGE_NAME}_PKGCONFIG_TYPE})
          endif()

          # Append the entry as a string onto the project-wide variable for
          # whichever requirement type we selected
          gz_string_append(PROJECT_${PROJECT_${PACKAGE_NAME}_PKGCONFIG_TYPE} ${${PACKAGE_NAME}_PKGCONFIG_ENTRY})

        endif()

        if(gz_find_package_REQUIRED_BY)

          # For each of the components that requires this package, append its
          # entry as a string onto the component-specific variable for whichever
          # requirement type we selected
          foreach(component ${gz_find_package_REQUIRED_BY})

            if(${component}_${PACKAGE_NAME}_PRIVATE)
              # If this is a private library or module, use the _PRIVATE suffix
              set(${component}_${PACKAGE_NAME}_PKGCONFIG_TYPE ${component}_${${PACKAGE_NAME}_PKGCONFIG_TYPE}_PRIVATE)
            else()
              # Otherwise, use the plain type
              set(${component}_${PACKAGE_NAME}_PKGCONFIG_TYPE ${component}_${${PACKAGE_NAME}_PKGCONFIG_TYPE})
            endif()

            # Append the entry as a string onto the component-specific variable
            # for whichever required type we selected
            gz_string_append(${${component}_${PACKAGE_NAME}_PKGCONFIG_TYPE} ${${PACKAGE_NAME}_PKGCONFIG_ENTRY})

          endforeach()

        endif()

      endif()

    endif()

  endif()

endmacro()
