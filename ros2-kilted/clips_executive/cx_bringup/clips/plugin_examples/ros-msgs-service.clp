
; Copyright (c) 2026 Carologistics
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

(defrule ros-msgs-service-init
" Create a service /ros_cx_service"
  (not (ros-msgs-service (service "ros_cx_service")))
  (not (executive-finalize))
=>
  (ros-msgs-create-service "/ros_cx_service" "std_srvs/srv/SetBool")
  (printout green "Opening service /ros_cx_service" crlf)
  (printout green "Call it via 'ros2 service call /ros_cx_service std_srvs/srv/SetBool \"{data: true}\""' crlf)
)

(deffunction /ros_cx_service-service-callback (?name ?request ?response)
  (printout green "Received a request" crlf)
  (bind ?data (ros-msgs-get-field ?request "data"))
  (printout yellow ?data crlf)
  (ros-msgs-set-field ?response "success" TRUE)
  (ros-msgs-set-field ?response "message" (str-cat "Received " ?data))
)

(defrule ros-msgs-service-finalize
" Delete the service on executive finalize. "
  (executive-finalize)
  (ros-msgs-service (service ?service))
=>
  (printout info "Destroying service" crlf)
  (ros-msgs-destroy-service ?service)
)
