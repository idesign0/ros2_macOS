; Copyright (c) 2025-2026 Carologistics
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

(defglobal
  ?*TURTLE-SERVICE* = "turtle1/teleport_absolute"
  ?*TURTLE-TELEPORT-TYPE* = "turtlesim/srv/TeleportAbsolute"
)

(defrule turtle-teleport-client-init
" Create publisher for ros_cx_out."
=>
  ; create the client
  (ros-msgs-create-client ?*TURTLE-SERVICE* ?*TURTLE-TELEPORT-TYPE*)
  (printout green "Opening client for " ?*TURTLE-SERVICE* crlf)
)

(defrule turtle-teleport-request-teleport-mid
" Attempt to request the service. "
  (ros-msgs-client (service ?service&:(eq ?service ?*TURTLE-SERVICE*)))
  (not (request ?any-id))
  (turtle-out-of-bounds)
  (time ?any-time) ; used to continuously attempt to request the service until success
  =>
  (bind ?new-req (ros-msgs-create-request ?*TURTLE-TELEPORT-TYPE*))
  (ros-msgs-set-field ?new-req "x" 5.5)
  (ros-msgs-set-field ?new-req "y" 5.5)
  (bind ?id (ros-msgs-async-send-request ?new-req ?service))
  (if ?id then
    (printout yellow "Request sent with id " ?id crlf)
    (assert (request ?id))
   else
    (printout red "Sending of request failed, is the service running?" crlf)
    (printout red "Start it using \"ros2 run turtlesim turtlesim_node\"" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
)
(defrule turtle-teleport-done
" Got response, delete it without reading, it is empty."
  ?msg-fact <- (ros-msgs-response (service ?service) (msg-ptr ?ptr) (request-id ?id))
  ?request-fact <- (request ?id)
  ?out-of-bounds-fact <- (turtle-out-of-bounds)
=>
  (printout yellow "Turtle teleport done (request id " ?id")" crlf)
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-fact ?request-fact ?out-of-bounds-fact)
)

(defrule turtle-teleport-client-finalize
" Delete the client on executive finalize. "
  (executive-finalize)
  (ros-msgs-client (service ?service))
=>
  (printout info "Destroying client" crlf)
  (ros-msgs-destroy-client ?service)
)
