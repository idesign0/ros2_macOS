#!/bin/bash
# Verify that cmake_minimum_required statements have matching version numbers

export CMAKE_FILES_TO_CHECK="
  $1/CMakeLists.txt
  $1/cmake/gz-all-config.cmake.in
  $1/cmake/gz-component-config.cmake.in
  $1/cmake/gz-config.cmake.in
  $1/config/gz-cmake-config.cmake.in"
unset TEST_CMAKE_MIN_REQUIRED_FAILED
# first argument is root of gz-cmake repository
if [[ ! -d "$1"  \
   || ! -a "$1/CMakeLists.txt" \
   || ! -a "$1/cmake/gz-all-config.cmake.in" \
   || ! -a "$1/cmake/gz-component-config.cmake.in" \
   || ! -a "$1/cmake/gz-config.cmake.in" \
   || ! -a "$1/config/gz-cmake-config.cmake.in" ]]; then
  echo the first argument must be the root of the gz-cmake repository
  export TEST_CMAKE_MIN_REQUIRED_FAILED=1
else
  grep -h '^cmake_minimum_required' $CMAKE_FILES_TO_CHECK \
    | uniq -c \
    | awk '{ if ($1 != "5") { exit 1 }}' \
    || \
    export TEST_CMAKE_MIN_REQUIRED_FAILED=1
fi

if test "$2" = "--xml_output_dir"; then
  xml_output_dir=$3
  if [[ ! -a "${xml_output_dir}" ]]; then
    mkdir -p "${xml_output_dir}"
  fi
  if [[ ! -d "${xml_output_dir}" ]]; then
    echo If using --xml_output_dir, the 3rd argument must be a directory.
    exit 1
  fi
  if [[ -z ${TEST_CMAKE_MIN_REQUIRED_FAILED} ]]; then
    cat <<END > ${xml_output_dir}/cmake_minimum_required.xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="1" failures="0" disabled="0" errors="0" timestamp="$(date '+%Y-%m-%dT%H:%M:%S')" time="0" name="AllTests">
  <testsuite name="cmake_minimum_required" tests="1" failures="0" disabled="0" errors="0" time="0">
    <testcase name="make" status="run" time="0" classname="cmake_minimum_required" />
  </testsuite>
</testsuites>
END
  else
    cat <<END > ${xml_output_dir}/cmake_minimum_required.xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="1" failures="1" disabled="0" errors="0" timestamp="$(date '+%Y-%m-%dT%H:%M:%S')" time="0" name="AllTests">
  <testsuite name="cmake_minimum_required" tests="1" failures="1" disabled="0" errors="0" time="0">
    <testcase name="make" status="run" time="0" classname="cmake_minimum_required">
      <failure type="Standard" message="cmake_minimum_required version numbers do not match."><![CDATA[
END
  grep -n '^cmake_minimum_required' $CMAKE_FILES_TO_CHECK \
    >> ${xml_output_dir}/cmake_minimum_required.xml
    cat <<END >> ${xml_output_dir}/cmake_minimum_required.xml
      ]]></failure>
    </testcase>
  </testsuite>
</testsuites>
END
    exit 1
  fi
else
  echo Verify that cmake_minimum_required statements have matching version numbers
  grep -n '^cmake_minimum_required' $CMAKE_FILES_TO_CHECK \
    | sed -e 's@^@  @'
  if [[ -z ${TEST_CMAKE_MIN_REQUIRED_FAILED} ]]; then
    echo --------------------------- Passed ---------------------------
  else
    echo --------------------------- Failed ---------------------------
    exit 1
  fi
fi
