#!/usr/bin/env bash
#
# Copyright (C) 2022 Open Source Robotics Foundation
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
# top-level entry point in gz-tools.

GZ_LOG_SUBCOMMANDS="
record
playback
"

GZ_LOG_COMPLETION_LIST="
  -h --help
  -v --verbose
"

GZ_PLAYBACK_COMPLETION_LIST="
  -h --help
  -v --verbose
  --file
  --pattern
  --remap
  --wait
  -f
"

GZ_RECORD_COMPLETION_LIST="
  -h --help
  -v --verbose
  --file
  --force
  --pattern
"

GZ_SERVICE_COMPLETION_LIST="
  -h --help
  -v --version
  -s --service
  --reqtype
  --reptype
  --timeout
  -l --list
  -i --info
  -r --req
"

GZ_TOPIC_COMPLETION_LIST="
  -h --help
  -t --topic
  -m --msgtype
  -d --duration
  -n --num
  -l --list
  -i --info
  -e --echo
  -p --pub
  -v --version
  --json-output
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

function _gz_log_playback
{
  __get_comp_from_list "$GZ_PLAYBACK_COMPLETION_LIST"
}

function _gz_log_record
{
  __get_comp_from_list "$GZ_RECORD_COMPLETION_LIST"
}

function _gz_service
{
  __get_comp_from_list "$GZ_SERVICE_COMPLETION_LIST"
}

function _gz_topic
{
  __get_comp_from_list "$GZ_TOPIC_COMPLETION_LIST"
}

# This searches the current list of typed words for one of the subcommands
# listed in GZ_LOG_SUBCOMMANDS. This should work for most cases, but may fail
# if a word that looks like a subcommand is used as an argument to a flag.
function __get_subcommand
{
  local known_subcmd
  local subcmd
  for ((i=2; $i<=$COMP_CWORD; i++)); do
    for subcmd in $GZ_LOG_SUBCOMMANDS; do
      if [[ "${COMP_WORDS[i]}" == "$subcmd" ]]; then
        known_subcmd="$subcmd"
      fi
    done
  done
  echo "$known_subcmd"
}

function _gz_log
{
  if [[ $COMP_CWORD > 2 ]]; then
    local known_subcmd=$(__get_subcommand)
    if [[ ! -z $known_subcmd ]]; then
      local subcmd_func="_gz_log_$known_subcmd"
      if [[ "$(type -t $subcmd_func)" == 'function' ]]; then
        $subcmd_func
        return
      fi
    fi
  fi

  # If a subcommand is not found, assume we're still completing the subcommands
  # or flags for `log`.
  if [[ ${COMP_WORDS[COMP_CWORD]} == -* ]]; then
    COMPREPLY=($(compgen -W "$GZ_LOG_COMPLETION_LIST" \
      -- "${COMP_WORDS[COMP_CWORD]}" ))
  else
    COMPREPLY=($(compgen -W "${GZ_LOG_SUBCOMMANDS}" -- ${cur}))
  fi
}


function _gz_service_flags
{
  for word in $GZ_SERVICE_COMPLETION_LIST; do
    echo "$word"
  done
}


function _gz_topic_flags
{
  for word in $GZ_TOPIC_COMPLETION_LIST; do
    echo "$word"
  done
}
