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

(defrule pddl-fluent-change-request
  (declare (salience ?*PRIORITY-PDDL-FLUENTS*))
  (pddl-fluent-change (instance ?instance) (state PENDING))
  ?pi-f <- (pddl-instance (name ?instance) (state LOADED)(busy-with FALSE))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?add-s&:(eq ?add-s (str-cat ?node "/add_fluents"))) (type ?add-type))
  (ros-msgs-client (service ?rm-s&:(eq ?rm-s (str-cat ?node "/rm_fluents"))) (type ?rm-type))
  =>
  (bind ?request-sent FALSE)
  (bind ?fluent-add-msgs (create$))
  (bind ?fluent-rm-msgs (create$))
  (do-for-all-facts ((?ppf pddl-fluent-change)) (and (eq ?ppf:state PENDING) (eq ?ppf:instance ?instance))
    (bind ?fluent-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Fluent"))
    (ros-msgs-set-field ?fluent-msg "pddl_instance" ?ppf:instance)
    (ros-msgs-set-field ?fluent-msg "name" ?ppf:name)
    (ros-msgs-set-field ?fluent-msg "args" ?ppf:params)
    (if ?ppf:delete then
      (bind ?fluent-rm-msgs (create$ ?fluent-rm-msgs ?fluent-msg))
     else
      (bind ?fluent-add-msgs (create$ ?fluent-add-msgs ?fluent-msg))
    )
    (modify ?ppf (state WAITING))
  )
  (if (> (length$ ?fluent-add-msgs) 0) then
    (bind ?new-req (ros-msgs-create-request ?add-type))
    (ros-msgs-set-field ?new-req "fluents" ?fluent-add-msgs)
    (bind ?add-id (ros-msgs-async-send-request ?new-req ?add-s))
    (if ?add-id then
      (bind ?request-sent TRUE)
      (assert (pddl-service-request-meta (service ?add-s) (request-id ?add-id) (meta ?instance)))
     else
      (printout error "Sending of request failed, is the service " ?add-s " running?" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
    (foreach ?msg ?fluent-add-msgs
      (ros-msgs-destroy-message ?msg)
    )
  )
  (if (> (length$ ?fluent-rm-msgs) 0) then
    (bind ?new-req (ros-msgs-create-request ?rm-type))
    (ros-msgs-set-field ?new-req "fluents" ?fluent-rm-msgs)
    (bind ?rm-id (ros-msgs-async-send-request ?new-req ?rm-s))
    (if ?rm-id then
      (bind ?request-sent TRUE)
      (assert (pddl-service-request-meta (service ?rm-s) (request-id ?rm-id) (meta ?instance)))
     else
      (printout error "Sending of request failed, is the service " ?rm-s " running?" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
    (foreach ?msg ?fluent-rm-msgs
      (ros-msgs-destroy-message ?msg)
    )
  )
  (if ?request-sent then
    (modify ?pi-f (busy-with FLUENTS))
  )
)

(defrule pddl-fluent-change-add-process-response
" Process a response to the /add_fluents service by adding the respective pddl-fluent facts and clean up the associated pending facts afterwards.
"
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/add_fluents"))))
  ?req-f <- (pddl-service-request-meta (service ?s) (meta ?instance) (request-id ?id))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  =>
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (printout debug "Successfully added fluents" crlf)
    (delayed-do-for-all-facts ((?ppf pddl-fluent-change)) (and (eq ?ppf:state WAITING) (eq ?ppf:instance ?instance) (not ?ppf:delete))
      (assert (pddl-fluent (instance ?ppf:instance) (name ?ppf:name) (params ?ppf:params)))
      (retract ?ppf)
    )
   else
    (printout error "Failed to add fluents \"" ?instance "\":" ?error crlf)
    (delayed-do-for-all-facts ((?ppf pddl-fluent-change)) (and (eq ?ppf:state WAITING) (eq ?ppf:instance ?instance) (not ?ppf:delete))
      (modify ?ppf (state ERROR))
    )
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-f)
)

(defrule pddl-fluent-change-rm-process-response
" Process a response to the /rm_fluents service by removing the respective pddl-fluent facts and clean up the associated pending facts afterwards.
"
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/rm_fluents"))))
  ?req-f <- (pddl-service-request-meta (service ?s) (meta ?instance) (request-id ?id))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  =>
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (printout debug "Successfully removed fluents" crlf)
    (delayed-do-for-all-facts ((?ppf pddl-fluent-change)) (and (eq ?ppf:state WAITING) (eq ?ppf:instance ?instance) ?ppf:delete)
      (do-for-fact ((?fluent pddl-fluent)) (and (eq ?fluent:name ?ppf:name) (eq ?fluent:params ?ppf:params))
        (retract ?fluent)
      )
      (retract ?ppf)
    )
   else
    (printout error "Failed to remove fluents \"" ?instance "\":" ?error crlf)
    ; TODO: how to deal with failed removing of fluents
    (delayed-do-for-all-facts ((?ppf pddl-fluent-change)) (and (eq ?ppf:state WAITING) (eq ?ppf:instance ?instance) ?ppf:delete)
      (modify ?ppf (state ERROR) (error ?error))
    )
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-f)
)

(defrule pddl-fluent-change-all-requests-done
  (pddl-manager (node ?node))
  ?pi-f <- (pddl-instance (name ?instance) (busy-with FLUENTS))
  (ros-msgs-client (service ?add-s&:(eq ?add-s (str-cat ?node "/add_fluents"))) (type ?add-type))
  (ros-msgs-client (service ?rm-s&:(eq ?rm-s (str-cat ?node "/rm_fluents"))) (type ?rm-type))
  (not (pddl-service-request-meta (service ?add-s) (meta ?instance)))
  (not (pddl-service-request-meta (service ?rm-s) (meta ?instance)))
  =>
  (modify ?pi-f (busy-with FALSE))
)
