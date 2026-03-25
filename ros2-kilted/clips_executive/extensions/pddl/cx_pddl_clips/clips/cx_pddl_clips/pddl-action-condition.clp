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


(defrule pddl-action-condition-check-request
  (declare (salience ?*PRIORITY-PDDL-CHECK-CONDITION*))
  (pddl-action (id ?action-id) (name ?name) (instance ?instance) (params $?params))
  ?check-fact <- (pddl-action-condition (action ?action-id) (state PENDING))
  (not (pddl-action-condition (action ?action-id) (state CHECK-CONDITION)))
  ?pi-f <- (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/check_action_condition"))) (type ?type))
  (not (pddl-service-request-meta (service ?s) (meta ?action-id)))
  =>
  (bind ?new-req (ros-msgs-create-request ?type))
  (bind ?action-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Fluent"))
  (ros-msgs-set-field ?action-msg "pddl_instance" ?instance)
  (ros-msgs-set-field ?action-msg "name" ?name)
  (ros-msgs-set-field ?action-msg "args" ?params)
  (ros-msgs-set-field ?new-req "action" ?action-msg)
  (bind ?id (ros-msgs-async-send-request ?new-req ?s))
  (if ?id then
    (assert (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id)))
    (modify ?pi-f (busy-with CHECK-CONDITIONS))
   else
    (printout error "Sending of request failed, is the service " ?s " running?" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
  (ros-msgs-destroy-message ?action-msg)
  (modify ?check-fact (state CHECK-CONDITION))
)

(defrule pddl-action-condition-check-response
  ?action-fact <- (pddl-action (id ?action-id))
  ?check-fact <- (pddl-action-condition (action ?action-id) (state CHECK-CONDITION))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with CHECK-CONDITIONS))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/check_action_condition"))) (type ?type))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id))
  =>
  (modify ?pi-f (busy-with FALSE))
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (bind ?sat (ros-msgs-get-field ?ptr "sat"))
    (if ?sat then
      (modify ?check-fact (state CONDITION-SAT))
     else
      (modify ?check-fact (state CONDITION-UNSAT))
      (bind ?unsat-conds (ros-msgs-get-field ?ptr "unsatisfied_conditions"))
      (printout debug ?action-id " condition unsat: " crlf)
      (bind ?unsats (create$))
      (foreach ?cond ?unsat-conds
        (printout debug ?cond crlf)
        (bind ?unsats (insert$ ?unsats 1 ?cond))
      )
      (modify ?check-fact (unsatisfied-conditions $?unsats))
    )
   else
    (printout error "Failed to check condition \"" ?action-id "\":" ?error crlf)
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-meta)
)

; TODO: either do similar stuff for all requests or leave it
(defrule pddl-action-condition-response-no-action
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/check_action_condition"))) (type ?type))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with CHECK-CONDITIONS))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id))
  (not
    (and
      (pddl-action (id ?action-id))
      (pddl-action-condition (action ?action-id) (state CHECK-CONDITION))
    )
  )
  =>
  (modify ?pi-f (busy-with FALSE))
  (printout warn "Received condition check response without belonging action " ?action-id crlf)
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-meta)
)
