
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

; ---------------- SETUP INSTANCE ------------------

(defrule cx-pddl-clips-agent-pddl-init
=>
  (assert (pddl-manager (node "/pddl_manager")))
)

(defrule cx-pddl-clips-agent-pddl-add-instance
" Setup PDDL instance with an active goal to plan for "
  (pddl-manager (ros-comm-init TRUE))
=>
  (bind ?share-dir (ament-index-get-package-share-directory "cx_pddl_bringup"))
  (assert
    (pddl-instance
      (name test)
      (domain "domain.pddl")
      (problem "problem.pddl")
      (directory (str-cat ?share-dir "/pddl"))
    )
    (pddl-get-fluents (instance test))
    (pddl-create-goal-instance (instance test) (goal active-goal))
    (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params a b))
    (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params b c))
    (pddl-set-goals (instance test) (goal active-goal))
    (pddl-plan (id test-plan) (instance test) (goal active-goal) (plan-type TEMPORAL))
  )
)


(defrule cx-pddl-clips-agent-select-action
" Start executing the first action of the resulting plan "
  ?plan <- (pddl-plan (id ?plan-id) (plan-start ?p-start))
  (not (pddl-action (state EXECUTING|SELECTED)))
  ?pa <- (pddl-action (plan ?plan-id) (planned-start-time ?t) (state IDLE))
  (not (pddl-action (plan ?plan-id) (state IDLE) (planned-start-time ?ot&:(< ?ot ?t))))
=>
  (if (= ?p-start 0.0) then (modify ?plan (plan-start (now))))
  (modify ?pa (state SELECTED))
)

(defrule cx-pddl-clips-agent-check-action
" Before executing an action check the condition to make sure it is feasible "
  (pddl-action (id ?id) (state SELECTED) (name ?name) (params $?params))
  (not (pddl-action-condition (action ?id)))
=>
  (assert (pddl-action-condition (instance test) (action ?id)))
)

(defrule cx-pddl-clips-agent-executable-action
" Condition is satisfied, go ahead with execution "
  (pddl-plan (id ?plan-id) (plan-start ?t))
  (pddl-action-condition (action ?action-id) (state CONDITION-SAT))
  ?pa <- (pddl-action (id ?action-id) (plan ?plan-id) (name ?name) (params $?params) (state SELECTED))
=>
  (modify ?pa (state EXECUTING) (actual-start-time (- (now) ?t)))
)

(defrule cx-pddl-clips-agent-execution-done
" After the duration has elapsed, the action is done "
  (time ?now)
  (pddl-plan (id ?plan-id) (plan-start ?t))
  ?pa <- (pddl-action (id ?id) (plan ?plan-id) (state EXECUTING) (planned-duration ?d) (name ?name)
    (actual-start-time ?s&:(< (+ ?s ?d ?t) ?now)))
=>
  (bind ?duration (- (now) (+ ?s ?t)))
  (printout info "Executed action " ?name " in " ?duration " seconds" crlf)
  (modify ?pa (state DONE) (actual-duration ?duration))
  (assert (pddl-action-get-effect (action ?id) (apply TRUE)))
)

(defrule cx-pddl-clips-agent-print-exec-times
" Once everything is done, print out planned vs actual times "
  (pddl-action)
  (not (pddl-action (state ~DONE)))
  (not (printed))
=>
  (printout blue "Execution done" crlf)
  (do-for-all-facts ((?pa pddl-action)) TRUE
     (printout green "action " ?pa:name " "
       ?pa:params " " ?pa:planned-start-time "|" ?pa:planned-duration
       " vs actual " ?pa:actual-start-time "|" ?pa:actual-duration crlf
     )
  )
  (assert (printed))
)
