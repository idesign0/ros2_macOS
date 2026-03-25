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

(defrule pddl-plan-temporal-start-plan
" Call a planner based on a pddl-plan fact."
  (declare (salience ?*PRIORITY-PDDL-PLAN*))
  (pddl-manager (node ?node))
  (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  (cx-pddl-interfaces-plan-temporal-client (server ?server&:(eq ?server (str-cat ?node "/temp_plan"))))
  ?pp <- (pddl-plan (instance ?instance) (goal ?goal) (type TEMPORAL) (state PENDING))
  (not (pddl-plan (state WAITING)))
  =>
  (bind ?goal-msg (cx-pddl-interfaces-plan-temporal-goal-create))
  (cx-pddl-interfaces-plan-temporal-goal-set-field ?goal-msg "pddl_instance" ?instance)
  (cx-pddl-interfaces-plan-temporal-goal-set-field ?goal-msg "goal_instance" ?goal)
  (cx-pddl-interfaces-plan-temporal-send-goal ?goal-msg ?server)
  (modify ?pp (state WAITING) (goal-ptr ?goal-msg))
)

(defrule pddl-plan-temporal-get-goal-response
" Process the first goal response"

  (pddl-manager (node ?node))
  (cx-pddl-interfaces-plan-temporal-client (server ?server&:(eq ?server (str-cat ?node "/temp_plan"))))
  ?response <- (cx-pddl-interfaces-plan-temporal-goal-response (server ?server) (client-goal-handle-ptr ?gh-ptr))
  ?pp <- (pddl-plan (instance ?instance) (goal ?goal) (goal-ptr ?goal-ptr) (type TEMPORAL) (state WAITING))
  =>
  (bind ?status (cx-pddl-interfaces-plan-temporal-client-goal-handle-get-status ?gh-ptr))
  ; ACCEPTED, EXECUTING,  SUCCEEDED
  (if (or (= ?status 1)  (= ?status 2) (= ?status 4)) then
    (modify ?pp (state PLANNING) (goal-handle ?gh-ptr))
    (retract ?response)
    else
    ; CANCELING, CANCELED, ABORTED
    (if (or (= ?status 3) (= ?status 5) (= ?status 6))
      then
      (modify ?pp (state FAILURE) (goal-handle ?gh-ptr))
      else
      (modify ?pp (state FAILURE) (goal-handle ?gh-ptr))
      (printout error "pddl-plan-temporal: Unexpected status "  ?status crlf)
    )
  )
  (retract ?response)
)

(defrule pddl-plan-temporal-cancel-start
" Cancel a plan action"
  (pddl-manager (node ?node))
  (cx-pddl-interfaces-plan-temporal-client (server ?server&:(eq ?server (str-cat ?node "/temp_plan"))))
  ?pp <- (pddl-plan (instance ?instance) (goal ?goal) (type TEMPORAL) (state REQUEST-CANCELING) (goal-handle ?gh-ptr))
=>
  (cx-pddl-interfaces-plan-temporal-client-cancel-goal ?server ?gh-ptr)
  (modify ?pp (state CANCELING))
)

(defrule pddl-plan-temporal-cancel-done
" Cancelation of plan action successful"
  (pddl-manager (node ?node))
  (pddl-instance (state LOADED) (name ?instance) (busy-with FALSE))
  (cx-pddl-interfaces-plan-temporal-client (server ?server&:(eq ?server (str-cat ?node "/temp_plan"))))
  ?response <- (cx-pddl-interfaces-plan-temporal-cancel-goal-response (server ?server) (num-goals 1) (return-code ERROR_NONE|ERROR_GOAL_TERMINATED))
  ?entry <- (cx-pddl-interfaces-plan-temporal-cancel-goal-entry (server ?server) (goal-id ?uuid))
  ?pp <- (pddl-plan (instance ?instance) (goal ?goal) (type TEMPORAL) (state CANCELING) (goal-handle ?gh-ptr))
  (test (eq (cx-pddl-interfaces-plan-temporal-client-goal-handle-get-goal-id ?gh-ptr) ?uuid))
=>
  (modify ?pp (state CANCELED))
  (retract ?response ?entry)
  (cx-pddl-interfaces-plan-temporal-client-goal-handle-destroy ?gh-ptr)
)

(defrule pddl-plan-temporal-plan-result
" Retrieve the resulting plan "
  (pddl-manager (node ?node))
  (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  ?pp <- (pddl-plan (instance ?instance) (id ?plan-id) (goal ?goal) (goal-ptr ?goal-ptr) (type TEMPORAL) (state PLANNING) (goal-handle ?gh-ptr))
  ?wr-f <- (cx-pddl-interfaces-plan-temporal-wrapped-result
    (server ?server&:(eq ?server (str-cat ?node "/temp_plan"))) (goal-id ?uuid) (code SUCCEEDED) (result-ptr ?res-ptr))
  (test (eq (cx-pddl-interfaces-plan-temporal-client-goal-handle-get-goal-id ?gh-ptr) ?uuid))
  =>
  (bind ?plan-found (cx-pddl-interfaces-plan-temporal-result-get-field ?res-ptr "success"))
  (bind ?id 0)
  (if ?plan-found then
    (bind ?plan (cx-pddl-interfaces-plan-temporal-result-get-field ?res-ptr "actions"))
    (foreach ?action ?plan
      (bind ?name (sym-cat (cx-pddl-interfaces-timed-plan-action-get-field ?action "name")))
      (bind ?args (cx-pddl-interfaces-timed-plan-action-get-field ?action "args"))
      (bind ?ps-time (cx-pddl-interfaces-timed-plan-action-get-field ?action "start_time"))
      (bind ?p-duration (cx-pddl-interfaces-timed-plan-action-get-field ?action "duration"))
      (assert (pddl-action
        (id ?id)
        (plan ?plan-id)
        (instance ?instance)
        (name ?name)
        (params ?args)
        (planned-start-time ?ps-time)
        (planned-duration ?p-duration))
      )
      (bind ?id (+ ?id 1))
    )
  else
    (printout red "plan not found!" crlf)
  )
  (retract ?wr-f)
  (cx-pddl-interfaces-plan-temporal-result-destroy ?res-ptr)
  (cx-pddl-interfaces-plan-temporal-goal-destroy ?goal-ptr)
  (cx-pddl-interfaces-plan-temporal-client-goal-handle-destroy ?gh-ptr)
)
