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

(defrule pddl-clear-goals-request
  (declare (salience ?*PRIORITY-PDDL-CLEAR-GOALS*))
  (pddl-clear-goals (instance ?instance) (goal ?goal) (state PENDING))
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/clear_goals"))) (type ?type))
  (not (pddl-service-request-meta (service ?s)))
  (time ?any-time) ; used to continuously attempt to request the service until success
  =>
  (bind ?new-req (ros-msgs-create-request ?type))
  (ros-msgs-set-field ?new-req "pddl_instance" ?instance)
  (ros-msgs-set-field ?new-req "goal_instance" ?goal)
  (bind ?id (ros-msgs-async-send-request ?new-req ?s))
  (if ?id then
    (assert (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?instance)))
    (modify ?pi-f (busy-with CLEAR-GOALS))
   else
    (printout error "Sending of request failed, is the service " ?s " running?" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
)

(defrule pddl-clear-goals-response-received
" Get response, read it and delete."
  ?clear-goals-f <- (pddl-clear-goals (instance ?instance) (goal ?goal) (state PENDING))
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with CLEAR-GOALS))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/clear_goals"))) (type ?type))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?instance))
=>
  (modify ?pi-f (busy-with FALSE))
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (modify ?clear-goals-f (state DONE))
   else
    (modify ?clear-goals-f (state ERROR) (error ?error))
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-meta)
)
