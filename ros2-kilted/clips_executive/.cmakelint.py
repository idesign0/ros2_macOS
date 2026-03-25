# Copyright (c) 2024-2026 Carologistics
# SPDX-License-Identifier: Apache-2.0
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

# ----------------------------
# Options affecting the linter
# ----------------------------
with section('lint'):
    # C0307 complains after indentation after formatting
    # C0301 is the 80 character limit, but we do not enforce it,
    #       if the formatter is happy, so are we.
    # R0912 and R0915 enforce the max number of statements and
    #       if-statements per scope
    disabled_codes = ['C0111', 'C0307', 'R0912', 'R0915', 'C0301', 'C0103', 'C0303']
