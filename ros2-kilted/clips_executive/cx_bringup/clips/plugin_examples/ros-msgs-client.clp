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

(defrule ros-msgs-client-init
" Create publisher for ros_cx_out."
  (not (ros-msgs-client (service "ros_cx_client")))
  (not (executive-finalize))
=>
  ; create the publisher
  (ros-msgs-create-client "ros_cx_client" "std_srvs/srv/SetBool")
  (printout info "Opening client on /ros_cx_client" crlf)
)

(defrule ros-msgs-request-true
" Attempt to request the service. "
  (ros-msgs-client (service ?service))
  (not (request ?any-id))
  (time ?any-time) ; used to continuously attempt to request the service until success
  =>
  (bind ?new-req (ros-msgs-create-request "std_srvs/srv/SetBool"))
  (ros-msgs-set-field ?new-req "data" TRUE)
  (bind ?id (ros-msgs-async-send-request ?new-req ?service))
  (if ?id then
    (printout yellow "Request sent with id " ?id crlf)
    (assert (request ?id))
   else
    (printout red "Sending of request failed, is the service running?" crlf)
    (printout red "Start it using \"ros2 run cx_bringup test_service.py\"" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
)
(defrule set-bool-client-response-received
" Get response, read it and delete."
  ?msg-fact <- (ros-msgs-response (service ?service) (msg-ptr ?ptr) (request-id ?id))
  (request ?id)
=>
  (bind ?succ (ros-msgs-get-field ?ptr "success"))
  (bind ?msg (ros-msgs-get-field ?ptr "message"))
  (printout green "Received response from " ?service " with: " ?succ " (" ?msg ")" crlf)
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-fact)
)

(defrule ros-msgs-client-finalize
" Delete the client on executive finalize. "
  (executive-finalize)
  (ros-msgs-client (service ?service))
=>
  (printout info "Destroying client" crlf)
  (ros-msgs-destroy-client ?service)
)

(defrule ros-msgs-message-cleanup
" Delete the messages on executive finalize. "
  (executive-finalize)
  ?msg-f <- (ros-msgs-message (msg-ptr ?ptr))
=>
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
)
