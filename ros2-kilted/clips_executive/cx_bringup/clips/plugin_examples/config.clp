; Copyright (c) 2024-2026 Carologistics
; SPDX-License-Identifier: Apache-2.0
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

(defrule load-bringup-config
  (not (confval))
  =>
  (bind ?share-dir (ament-index-get-package-share-directory "cx_bringup"))
  (bind ?file (str-cat ?share-dir "/params/plugin_examples/config.yaml"))
  (printout green "Loading yaml file: " ?file crlf)
  (config-load ?file "/")
  (delayed-do-for-all-facts ((?cv confval))
    (ppfact ?cv blue)
  )
)
