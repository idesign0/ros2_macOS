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

(printout info "(ros-param-get-value \"environments\" (create$ not-found))" crlf)
(printout green "    " (ros-param-get-value "environments" (create$ not-found)) crlf)

(printout info "(ros-param-get-value \"cx_ros_param.log_clips_to_file\" FALSE)" crlf)
(printout green "    " (ros-param-get-value "cx_ros_param.log_clips_to_file" FALSE) crlf)

(printout info "(ros-param-get-value \"bond_heartbeat_period\" 5.5)" crlf)
(printout green "    " (ros-param-get-value "bond_heartbeat_period" 5.5) crlf)

(printout info "(ros-param-get-value \"does_not_exist\" (create$ 1 2 3)" crlf)
(printout green "    " (ros-param-get-value "does_not_exist" (create$ 1 2 3)) crlf)

(printout info "(ros-param-get-value \"unused_param\" \"\"" crlf)
(printout green "    " (ros-param-get-value "unused_param" "") crlf)
