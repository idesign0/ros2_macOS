#!/usr/bin/env bash
#
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
#

# bash tab-completion

# This is a per-library function definition, used in conjunction with the
# top-level entry point in ign-tools.

GZ_FUEL_SUBCOMMANDS="
delete
download
edit
list
meta
upload
"

# TODO: In Fortress+, for each of the completion lists, remove --force-version
# and --versions. Add --help-all. Update ../gz_TEST.cc accordingly.
GZ_FUEL_COMPLETION_LIST="
  -c --config
  -h --help
  -v --verbose
  --force-version
  --versions
"

GZ_DELETE_COMPLETION_LIST="
  --header
  -c --config
  -h --help
  -u --url
  --force-version
  --versions
"

GZ_DOWNLOAD_COMPLETION_LIST="
  --header
  -c --config
  -h --help
  -j --jobs
  -t --type
  -u --url
  --force-version
  --versions
"

GZ_EDIT_COMPLETION_LIST="
  --header
  -b --public
  -c --config
  -h -help
  -m --model
  -p --private
  -u --url
  --force-version
  --versions
"

GZ_LIST_COMPLETION_LIST="
  -c --config
  -h --help
  -o --owner
  -r --raw
  -t --type
  -u --url
  --force-version
  --versions
"

GZ_META_COMPLETION_LIST="
  --config2pbtxt
  --pbtxt2config
  -c --config
  -h --help
  --force-version
  --versions
"

GZ_UPLOAD_COMPLETION_LIST="
  --header
  -c --config
  -h --help
  -m --model
  -p --private
  -u --url
  --force-version
  --versions
"

function __get_comp_from_list {
  if [[ ${COMP_WORDS[COMP_CWORD]} == -* ]]; then
    # Specify options (-*) word list for this subcommand
    COMPREPLY=($(compgen -W "$@" \
      -- "${COMP_WORDS[COMP_CWORD]}" ))
    return
  else
    # Just use bash default auto-complete, because we never have two
    # subcommands in the same line. If that is ever needed, change here to
    # detect subsequent subcommands
    COMPREPLY=($(compgen -o default -- "${COMP_WORDS[COMP_CWORD]}"))
    return
  fi
}

function _gz_fuel_delete
{
  __get_comp_from_list "$GZ_DELETE_COMPLETION_LIST"
}

function _gz_fuel_download
{
  __get_comp_from_list "$GZ_DOWNLOAD_COMPLETION_LIST"
}

function _gz_fuel_edit
{
  __get_comp_from_list "$GZ_EDIT_COMPLETION_LIST"
}

function _gz_fuel_list
{
  __get_comp_from_list "$GZ_LIST_COMPLETION_LIST"
}

function _gz_fuel_meta
{
  __get_comp_from_list "$GZ_META_COMPLETION_LIST"
}

function _gz_fuel_upload
{
  __get_comp_from_list "$GZ_UPLOAD_COMPLETION_LIST"
}

# This searches the current list of typed words for one of the subcommands
# listed in GZ_FUEL_SUBCOMMANDS. This should work for most cases, but may fail
# if a word that looks like a subocmmand is used as an argument to a flag. Eg.
# `gz fuel --config download list`. Here `download` is an argument to
# `--config`, but this function will think that it's the subcommand. Since this
# seems like a rare scenario, we accept this failure mode.
function __get_subcommand
{
  local known_subcmd
  local subcmd
  for ((i=2; $i<=$COMP_CWORD; i++)); do
    for subcmd in $GZ_FUEL_SUBCOMMANDS; do
      if [[ "${COMP_WORDS[i]}" == "$subcmd" ]]; then
        known_subcmd="$subcmd"
      fi
    done
  done
  echo "$known_subcmd"
}

function _gz_fuel
{
  if [[ $COMP_CWORD > 2 ]]; then
    local known_subcmd=$(__get_subcommand)
    if [[ ! -z $known_subcmd ]]; then
      local subcmd_func="_gz_fuel_$known_subcmd"
      if [[ "$(type -t $subcmd_func)" == 'function' ]]; then
        $subcmd_func
        return
      fi
    fi
  fi

  # If a subcommand is not found, assume we're still completing the subcommands
  # or flags for `fuel`.
  if [[ ${COMP_WORDS[COMP_CWORD]} == -* ]]; then
    COMPREPLY=($(compgen -W "$GZ_FUEL_COMPLETION_LIST" \
      -- "${COMP_WORDS[COMP_CWORD]}" ))
  else
    COMPREPLY=($(compgen -W "${GZ_FUEL_SUBCOMMANDS}" -- ${cur}))
  fi
}
