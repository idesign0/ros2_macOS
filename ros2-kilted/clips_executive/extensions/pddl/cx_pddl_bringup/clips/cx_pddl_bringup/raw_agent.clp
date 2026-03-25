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

(defrule pddl-init
    (not (pddl-services-loaded))
    =>
    ; create clints for all services
    (bind ?services (create$
        add_fluents AddFluents
        add_pddl_instance AddPddlInstance
        get_fluents GetFluents
        set_goals SetGoals
        create_goal_instance CreateGoalInstance
    ))
    (bind ?index 1)
    (bind ?length (length$ ?services))
    (while (< ?index ?length)
        (bind ?service-name (nth$ ?index ?services))
        (bind ?service-type (nth$ (+ ?index 1) ?services))
        (ros-msgs-create-client
            (str-cat "/pddl_manager" "/" ?service-name)
            (str-cat "cx_pddl_interfaces/srv/" ?service-type)
        )
        (bind ?index (+ ?index 2))
    )
    (assert (pddl-services-loaded))
)

(defrule pddl-init-plan-client
    (not (pddl-planning-client-created))
    (pddl-services-loaded)
    =>
    (cx-pddl-interfaces-plan-temporal-create-client (str-cat "/pddl_manager" "/temp_plan"))
    (assert (pddl-planning-client-created))
)

; ---------------- SETUP INSTANCE ------------------

(defrule pddl-add-instance
	(ros-msgs-client (service ?service&:(eq ?service (str-cat "/pddl_manager" "/add_pddl_instance"))) (type ?type))
	(not (pddl-loaded))
    (pddl-planning-client-created)
	(time ?) ; poll until pddl manager is actually started
	=>
	(bind ?new-req (ros-msgs-create-request ?type))
	(ros-msgs-set-field ?new-req "name" "test") ;instance of name test
	(bind ?share-dir (ament-index-get-package-share-directory "cx_pddl_bringup"))
	(ros-msgs-set-field ?new-req "directory" (str-cat ?share-dir "/pddl"))
	(ros-msgs-set-field ?new-req "domain_file" "domain.pddl")
	(ros-msgs-set-field ?new-req "problem_file" "problem.pddl")
	(bind ?id (ros-msgs-async-send-request ?new-req ?service))
	(if ?id then
	  (assert (pddl-loaded))
	)
	(ros-msgs-destroy-message ?new-req)
)

(defrule pddl-add-instance-result
	(ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/add_pddl_instance"))) (type ?type))
	?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
    (pddl-loaded)
	=>
    (bind ?success (ros-msgs-get-field ?ptr "success"))
    (bind ?error (ros-msgs-get-field ?ptr "error"))
    (if ?success then
        (printout t "PDDL instance added" crlf)
    else
        (printout error "Failed to set problem instance" ?error crlf)
    )
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
    (assert (pddl-loaded-confirmed))
)

; ---------------- GET CURRENT FLUENTS ------------------

(defrule pddl-get-fluents
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/get_fluents"))) (type ?type))
    (pddl-loaded-confirmed)
    (not (pddl-fluents-requested))
    =>
    (bind ?new-req (ros-msgs-create-request ?type))
    (ros-msgs-set-field ?new-req "pddl_instance" "test")
    (bind ?id (ros-msgs-async-send-request ?new-req ?s))
    (if ?id then
        (printout t "Requested Fluents" crlf)
        (assert (pddl-fluents-requested))
    else
        (printout error "Sending of request failed, is the service " ?s " running?" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
)

(defrule pddl-get-fluents-result
" Get response, read it and delete."
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/get_fluents"))) (type ?type))
    ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
    (pddl-fluents-requested)
    =>
    (bind ?success (ros-msgs-get-field ?ptr "success"))
    (bind ?error (ros-msgs-get-field ?ptr "error"))
    (if ?success then
        (printout t "Got fluents from current instance" crlf)
        (bind ?fluents (ros-msgs-get-field ?ptr "fluents"))
        (foreach ?fluent ?fluents
        (bind ?instance (sym-cat (ros-msgs-get-field ?fluent "pddl_instance")))
        (bind ?name (sym-cat (ros-msgs-get-field ?fluent "name")))
        (bind ?args (ros-msgs-get-field ?fluent "args"))
        (printout t ?name ?args crlf)
        )
    else
        (printout error "Failed to get fluents (" "test" "):" ?error crlf)
    )
    (ros-msgs-destroy-message ?ptr)
    )


; ---------------- SET GOAL ------------------
(defrule pddl-create-goal-instance
    (pddl-fluents-requested)
    (not (pddl-goals-set))
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/create_goal_instance"))) (type ?type))
    =>
    (bind ?new-req (ros-msgs-create-request ?type))
    (ros-msgs-set-field ?new-req "pddl_instance" "test")
    (ros-msgs-set-field ?new-req "goal_instance" "active-goal")
    (bind ?id (ros-msgs-async-send-request ?new-req ?s))
    (ros-msgs-destroy-message ?new-req)
)

(defrule pddl-create-goal-instance-result
" Get response, read it and delete."
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/create_goal_instance"))) (type ?type))
    ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
    =>
    (bind ?success (ros-msgs-get-field ?ptr "success"))
    (bind ?error (ros-msgs-get-field ?ptr "error"))
    (if ?success then
        (printout t "Goals set successfully" crlf)
    else
        (printout error "Failed to create goal instance (" "active-goal" "):" ?error crlf)
    )
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
    (assert (pddl-goal-instance-set))
)


(defrule pddl-set-goal
    (pddl-fluents-requested)
    (not (pddl-goals-set))
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/set_goals"))) (type ?type))
	(pddl-goal-instance-set)
    =>
    (bind ?new-req (ros-msgs-create-request ?type))
    (bind ?fluent-goal-msgs (create$))

    (bind ?fluent-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Fluent"))
    (ros-msgs-set-field ?fluent-msg "pddl_instance" "test")
    (ros-msgs-set-field ?fluent-msg "name" "on")
    (ros-msgs-set-field ?fluent-msg "args" (create$ "a" "b"))
    (bind ?fluent-goal-msgs (create$ ?fluent-goal-msgs ?fluent-msg))
    (bind ?fluent-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Fluent"))
    (ros-msgs-set-field ?fluent-msg "pddl_instance" "test")
    (ros-msgs-set-field ?fluent-msg "name" "on")
    (ros-msgs-set-field ?fluent-msg "args" (create$ "b" "c"))
    (bind ?fluent-goal-msgs (create$ ?fluent-goal-msgs ?fluent-msg))

    (ros-msgs-set-field ?new-req "fluents" ?fluent-goal-msgs)
    (ros-msgs-set-field ?new-req "goal_instance" "active-goal")
    (bind ?id (ros-msgs-async-send-request ?new-req ?s))
    (if ?id then
        (printout t "Requested to set goals" crlf)
    else
        (printout error "Sending of request failed, is the service " ?s " running?" crlf)
    )
    (foreach ?msg ?fluent-goal-msgs
        (ros-msgs-destroy-message ?msg)
    )
    (ros-msgs-destroy-message ?new-req)
)

(defrule pddl-set-goal-result
" Get response, read it and delete."
    (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/set_goals"))) (type ?type))
    ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
    =>
    (bind ?success (ros-msgs-get-field ?ptr "success"))
    (bind ?error (ros-msgs-get-field ?ptr "error"))
    (if ?success then
        (printout t "Goals set successfully" crlf)
    else
        (printout error "Failed to set goals (" "test" "):" ?error crlf)
    )
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
    (assert (pddl-goals-set))
)
; ---------------- PLAN FOR INSTANCE ------------------



(defrule pddl-plan
    (cx-pddl-interfaces-plan-temporal-client (server ?server&:(eq ?server "/pddl_manager/temp_plan")))
    (not (planned))
    (pddl-goals-set)
    =>
    (printout green "Start planning" crlf)
    (bind ?goal (cx-pddl-interfaces-plan-temporal-goal-create))
    (cx-pddl-interfaces-plan-temporal-goal-set-field ?goal "pddl_instance" "test")
    (cx-pddl-interfaces-plan-temporal-goal-set-field ?goal "goal_instance" "active-goal")
    (cx-pddl-interfaces-plan-temporal-send-goal ?goal ?server)
    (assert (planned))
)

(defrule pddl-plan-result
    ?wr-f <- (cx-pddl-interfaces-plan-temporal-wrapped-result (server "/pddl_manager/temp_plan") (code SUCCEEDED) (result-ptr ?res-ptr))
    =>
    (bind ?plan-found (cx-pddl-interfaces-plan-temporal-result-get-field ?res-ptr "success"))
    (printout green "planning done" crlf)
    (if ?plan-found then
        (bind ?plan (cx-pddl-interfaces-plan-temporal-result-get-field ?res-ptr "actions"))
        (foreach ?action ?plan
        (bind ?name (sym-cat (cx-pddl-interfaces-timed-plan-action-get-field ?action "name")))
        (bind ?args (cx-pddl-interfaces-timed-plan-action-get-field ?action "args"))
        (bind ?ps-time (cx-pddl-interfaces-timed-plan-action-get-field ?action "start_time"))
        (bind ?p-duration (cx-pddl-interfaces-timed-plan-action-get-field ?action "duration"))
        (printout t ?ps-time "(" ?p-duration ")   " ?name ?args crlf)
        )
    else
        (printout red "plan not found!" crlf)
    )
    (cx-pddl-interfaces-plan-temporal-result-destroy ?res-ptr)
)
