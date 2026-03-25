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

(deffunction pddl-apply-op (?op ?value ?new-value)
  (if (eq ?op "=") then (return ?new-value))
  (if (eq ?op "+") then (return (+ ?value ?new-value)))
  (if (eq ?op "-") then (return (- ?value ?new-value)))
  (printout error "Operator " ?op " does not match expected [=,+,-]" crlf)
  (return ?new-value)
)

(defrule pddl-action-get-effect-request
  (declare (salience ?*PRIORITY-PDDL-APPLY-EFFECT*))
  (pddl-manager (node ?node))
  (pddl-action (id ?action-id) (name ?name) (params $?params) (instance ?instance))
  ?req-effect-f <- (pddl-action-get-effect (action ?action-id) (state PENDING))
  (not (pddl-action-get-effect (state START-EFFECT-APPLIED)))
  ?pi-f <- (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/get_action_effects"))) (type ?type))
  (not (pddl-service-request-meta (service ?s) (meta ?action-id)))
  =>
  (bind ?new-req (ros-msgs-create-request ?type))
  (bind ?action-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Action"))
  (ros-msgs-set-field ?action-msg "pddl_instance" ?instance)
  (ros-msgs-set-field ?action-msg "name" ?name)
  (ros-msgs-set-field ?action-msg "args" ?params)
  (ros-msgs-set-field ?new-req "action" ?action-msg)
  (bind ?id (ros-msgs-async-send-request ?new-req ?s))
  (if ?id then
    (assert (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id)))
    (modify ?req-effect-f (state WAITING))
    (modify ?pi-f (busy-with ACTION-EFFECTS))
   else
    (printout error "Sending of request failed, is the service " ?s " running?" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
  (ros-msgs-destroy-message ?action-msg)
)

(defrule pddl-action-apply-end-effect-start
  (declare (salience ?*PRIORITY-PDDL-APPLY-EFFECT*))
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with FALSE))
  (pddl-action-get-effect (action ?id) (effect-type ALL) (state START-EFFECT-APPLIED) (apply TRUE))
  (pddl-action (id ?id) (instance ?instance))
  =>
  (modify ?pi-f (busy-with ACTION-EFFECTS))
)

(defrule pddl-action-apply-effect-process-response
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with ACTION-EFFECTS))
  (pddl-action (id ?action-id) (instance ?instance))
  ?get-effect-f <- (pddl-action-get-effect (effect-type ?eff-type) (action ?action-id)
    (state ?state&:(member$ ?state (create$ WAITING START-EFFECT-APPLIED))) (apply TRUE))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/get_action_effects"))) (type ?type))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id))
  =>
  (modify ?pi-f (busy-with FALSE))
  (bind ?target-time-point FALSE)
  (bind ?next-state DONE)
  (if (and (member$ ?eff-type (create$ ALL START)) (eq ?state WAITING)) then
    (bind ?target-time-point START)
    (if (eq ?eff-type ALL) then
      (bind ?next-state START-EFFECT-APPLIED)
     else
      (bind ?nex-state DONE)
    )
   else
    (if (member$ ?eff-type (create$ ALL END)) then
      (bind ?target-time-point END)
    )
  )
  (modify ?get-effect-f (state ?next-state))
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (bind ?fun-effs (ros-msgs-get-field ?ptr "function_effects"))
    (foreach ?fun ?fun-effs
      (bind ?function-msg (ros-msgs-get-field ?fun "function"))
      (bind ?time-point (sym-cat (ros-msgs-get-field ?fun "time_point")))
      (if (eq ?time-point ?target-time-point) then
        (bind ?op (ros-msgs-get-field ?fun "operator_type"))
        (bind ?instance (sym-cat (ros-msgs-get-field ?function-msg "pddl_instance")))
        (bind ?name (sym-cat (ros-msgs-get-field ?function-msg "name")))
        (bind ?args (ros-msgs-get-field ?function-msg "args"))
        (bind ?arg-syms (create$))
        (foreach ?arg ?args
          (bind ?arg-syms (create$ ?arg-syms (sym-cat ?arg)))
        )
        (bind ?value (ros-msgs-get-field ?fun "value"))
        (if (not (do-for-fact ((?pf pddl-numeric-fluent)) (and (eq ?name ?pf:name) (eq ?pf:params ?arg-syms))
          (assert (pddl-numeric-fluent-change (name ?name) (params ?arg-syms)
            (value (pddl-apply-op ?op ?pf:value ?value)) (state PENDING) (instance ?instance)))
        )) then
          (printout error "pddl-fluent-change from action effect unknown, init to 0" crlf)
          (assert (pddl-numeric-fluent-change (instance ?instance) (name ?name) (params ?arg-syms) (value 0.0)))
          (assert (pddl-numeric-fluent-change (name ?name) (params ?arg-syms)
            (value (pddl-apply-op ?op 0.0 ?value)) (state PENDING) (instance ?instance)))
        ; TODO: pending numeric effects need the operator
        ; TODO: they also dont support values dependent on other functions
        )
      )
      (ros-msgs-destroy-message ?function-msg)
      (ros-msgs-destroy-message ?fun)
    )
    (bind ?fluent-effs (ros-msgs-get-field ?ptr "fluent_effects"))
    (foreach ?fluent ?fluent-effs
      (bind ?fluent-msg (ros-msgs-get-field ?fluent "fluent"))
      (bind ?time-point (sym-cat (ros-msgs-get-field ?fluent "time_point")))
      (if (eq ?time-point ?target-time-point) then
        (bind ?instance (sym-cat (ros-msgs-get-field ?fluent-msg "pddl_instance")))
        (bind ?name (sym-cat (ros-msgs-get-field ?fluent-msg "name")))
        (bind ?args (ros-msgs-get-field ?fluent-msg "args"))
        (bind ?arg-syms (create$))
        (foreach ?arg ?args
          (bind ?arg-syms (create$ ?arg-syms (sym-cat ?arg)))
        )
        (bind ?time-point (sym-cat (ros-msgs-get-field ?fluent "time_point")))
        (bind ?value (ros-msgs-get-field ?fluent "value"))
        (assert (pddl-fluent-change (name ?name) (params ?arg-syms)
          (delete (not ?value)) (state PENDING) (instance ?instance)))
      ); endif target-time-point matches effect
      (ros-msgs-destroy-message ?fluent-msg)
      (ros-msgs-destroy-message ?fluent)
    )
   else
    (printout error "Failed to retrieve effect for action \"" ?action-id "\":" ?error crlf)
  )
  (if (eq ?next-state DONE) then
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
    (retract ?req-meta)
  )
)

(defrule pddl-action-get-effect-process-response
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with ACTION-EFFECTS))
  (pddl-action (id ?action-id) (instance ?instance))
  ?get-effect-f <- (pddl-action-get-effect (effect-type ?eff-type) (action ?action-id)
    (state ?state&:(member$ ?state (create$ WAITING))) (apply FALSE))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/get_action_effects"))) (type ?type))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?action-id))
  =>
  (modify ?pi-f (busy-with FALSE))
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (modify ?get-effect-f (state DONE))
    (bind ?fun-effs (ros-msgs-get-field ?ptr "function_effects"))
    (foreach ?fun ?fun-effs
      (bind ?function-msg (ros-msgs-get-field ?fun "function"))
      (bind ?time-point (sym-cat (ros-msgs-get-field ?fun "time_point")))
      (if (eq ?time-point ?eff-type) then
        (bind ?op (ros-msgs-get-field ?fun "operator_type"))
        (bind ?instance (sym-cat (ros-msgs-get-field ?function-msg "pddl_instance")))
        (bind ?name (sym-cat (ros-msgs-get-field ?function-msg "name")))
        (bind ?args (ros-msgs-get-field ?function-msg "args"))
        (bind ?arg-syms (create$))
        (foreach ?arg ?args
          (bind ?arg-syms (create$ ?arg-syms (sym-cat ?arg)))
        )
        (bind ?value (ros-msgs-get-field ?fun "value"))
        (if (not (do-for-fact ((?pf pddl-fluent-change)) (and (eq ?name ?pf:name) (eq ?pf:params ?arg-syms))
          (assert (pddl-effect-numeric-fluent (name ?name) (params ?arg-syms)
            (value (pddl-apply-op ?op ?value ?pf:value)) (instance ?instance) (action ?action-id)))
        )) then
          (printout error "pddl-fluent-change from action effect unknown" crlf)
        )
      )
      (ros-msgs-destroy-message ?function-msg)
    )
    (bind ?fluent-effs (ros-msgs-get-field ?ptr "fluent_effects"))
    (foreach ?fluent ?fluent-effs
      (bind ?fluent-msg (ros-msgs-get-field ?fluent "fluent"))
      (bind ?time-point (sym-cat (ros-msgs-get-field ?fluent "time_point")))
      (if (eq ?time-point ?eff-type) then
        (bind ?instance (sym-cat (ros-msgs-get-field ?fluent-msg "pddl_instance")))
        (bind ?name (sym-cat (ros-msgs-get-field ?fluent-msg "name")))
        (bind ?args (ros-msgs-get-field ?fluent-msg "args"))
        (bind ?arg-syms (create$))
        (foreach ?arg ?args
          (bind ?arg-syms (create$ ?arg-syms (sym-cat ?arg)))
        )
        (bind ?time-point (sym-cat (ros-msgs-get-field ?fluent "time_point")))
        (bind ?value (ros-msgs-get-field ?fluent "value"))
        (if ?value then
          (assert (pddl-effect-fluent (name ?name) (params ?arg-syms)
            (action ?action-id) (instance ?instance)))
        )
      ); endif target-time-point matches effect
      (ros-msgs-destroy-message ?fluent-msg)
    )
   else
    (printout error "Failed to retrieve effect for action \"" ?action-id "\":" ?error crlf)
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-meta)
)
