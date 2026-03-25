#!/bin/sh

# This file can be downloaded (such as through curl) and used in CI to check
# that there are no doxygen warnings. For example, if you're using bitbucket
# pipelines then you can add the following line to your bitbucket-pipelines.yml
# file:
#   - bash <(curl -s https://github.com/gazebosim/gz-cmake/raw/main/tools/doc_check.sh)
if [ -f gz-doxygen.warn ]; then
  if [ -s gz-doxygen.warn ]; then
    echo "Error. The following warnings were found in gz-doxygen.warn."
    cat gz-doxygen.warn
    exit 1
  else
    echo "Success. No warnings found in gz-doxygen.warn."
    exit 0
  fi
else
  echo "The gz-doxygen.warn file does not exist. Skipping doc check."
fi
