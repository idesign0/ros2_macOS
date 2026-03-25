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

(deftemplate pddl-object-change
" Interface for objects.clp
  Assert a fact of this type in order to indicate that an object needs to be
  added to/removed from a pddl instance.
  @slot instance: pddl instance to add the object to.
  @slot name: name of the object.
  @slot type: type of the object.
  @slot delete: if true, remove the object, else add it.
  Slots set automatically:
  @slot state:
   - PENDING: The object was not added yet.
   - WAITING: The object is about to be added and is waiting for confirmation.
   - ERROR: The object might not have been fetched due to an error.
   - ON-HOLD: Unused state that can be set in order to defer the object update
     to a later time (by switching it manually to PENDING).
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (slot type (type SYMBOL))
  (slot delete (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  (slot request-id (type INTEGER))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
  (slot error (type STRING))
)

(defrule pddl-object-change-request
  (declare (salience ?*PRIORITY-PDDL-OBJECTS*))
  (pddl-object-change (instance ?instance) (state PENDING))
  ?pi-f <- (pddl-instance (name ?instance) (state LOADED) (busy-with FALSE))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?add-s&:(eq ?add-s (str-cat ?node "/add_objects"))) (type ?add-type))
  (ros-msgs-client (service ?rm-s&:(eq ?rm-s (str-cat ?node "/rm_objects"))) (type ?rm-type))
  (not (pddl-service-request-meta (service ?add-s) (meta ?instance)))
  (not (pddl-service-request-meta (service ?rm-s) (meta ?instance)))
  =>
  (bind ?request-sent FALSE)
  (bind ?object-add-msgs (create$))
  (bind ?object-rm-msgs (create$))
  (delayed-do-for-all-facts ((?ppo pddl-object-change)) (and (eq ?ppo:state PENDING) (eq ?ppo:instance ?instance))
    (bind ?object-msg (ros-msgs-create-message "cx_pddl_interfaces/msg/Object"))
    (ros-msgs-set-field ?object-msg "pddl_instance" ?ppo:instance)
    (ros-msgs-set-field ?object-msg "name" ?ppo:name)
    (ros-msgs-set-field ?object-msg "type" ?ppo:type)
    (if ?ppo:delete then
      (bind ?object-rm-msgs (create$ ?object-rm-msgs ?object-msg))
     else
      (bind ?object-add-msgs (create$ ?object-add-msgs ?object-msg))
    )
    (modify ?ppo (state WAITING))
  )
  (if (> (length$ ?object-add-msgs) 0) then
    (bind ?new-req (ros-msgs-create-request ?add-type))
    (ros-msgs-set-field ?new-req "objects" ?object-add-msgs)
    (bind ?add-id (ros-msgs-async-send-request ?new-req ?add-s))
    (if ?add-id then
      (bind ?request-sent TRUE)
      (assert (pddl-service-request-meta (service ?add-s) (request-id ?add-id) (meta ?instance)))
     else
      (printout error "Sending of request failed, is the service " ?add-s " running?" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
    (foreach ?msg ?object-add-msgs
      (ros-msgs-destroy-message ?msg)
    )
  )
  (if (> (length$ ?object-rm-msgs) 0) then
    (bind ?new-req (ros-msgs-create-request ?rm-type))
    (ros-msgs-set-field ?new-req "objects" ?object-rm-msgs)
    (bind ?rm-id (ros-msgs-async-send-request ?new-req ?rm-s))
    (if ?rm-id then
      (bind ?request-sent TRUE)
      (assert (pddl-service-request-meta (service ?rm-s) (request-id ?rm-id) (meta ?instance)))
     else
      (printout error "Sending of request failed, is the service " ?rm-s " running?" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
    (foreach ?msg ?object-add-msgs
      (ros-msgs-destroy-message ?msg)
    )
  )
  (if ?request-sent then
    (modify ?pi-f (busy-with OBJECTS))
  )
)

(defrule pddl-object-change-add-process-response
" Process a response to the /add_objects service and clean up the associated pending facts afterwards.
"
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/add_objects"))))
  ?req-f <- (pddl-service-request-meta (service ?s) (meta ?instance) (request-id ?id))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  =>
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (printout debug "Successfully added objects" crlf)
    (delayed-do-for-all-facts ((?ppo pddl-object-change)) (and (eq ?ppo:state WAITING) (eq ?ppo:instance ?instance) (not ?ppo:delete))
      (retract ?ppo)
    )
   else
    (delayed-do-for-all-facts ((?ppo pddl-object-change)) (and (eq ?ppo:state WAITING) (eq ?ppo:instance ?instance) (not ?ppo:delete))
      (modify ?ppo (state ERROR) (error ?error))
    )
    (printout error "Failed to add objects \"" ?instance "\":" ?error crlf)
    ; TODO: how to deal with failed adding of objects
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-f)
)

(defrule pddl-object-change-rm-process-response
" Process a response to the /rm_objects service and clean up the associated pending facts afterwards.
"
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/rm_objects"))))
  ?req-f <- (pddl-service-request-meta (service ?s) (meta ?instance) (request-id ?id))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  =>
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (printout debug "Successfully added objects" crlf)
    (delayed-do-for-all-facts ((?ppo pddl-object-change)) (and (eq ?ppo:state WAITING) (eq ?ppo:instance ?instance) ?ppo:delete)
      (retract ?ppo)
    )
    (printout debug "Successfully removed objects" crlf)
   else
    (delayed-do-for-all-facts ((?ppo pddl-object-change)) (and (eq ?ppo:state WAITING) (eq ?ppo:instance ?instance) ?ppo:delete)
      (modify ?ppo (state ERROR) (error ?error))
    )
    (printout error "Failed to remove objects \"" ?instance "\":" ?error crlf)
    ; TODO: how to deal with failed removing of objects
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-f)
)

(defrule pddl-object-change-all-requests-done
  ?pi-f <- (pddl-instance (name ?instance) (busy-with OBJECTS))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?add-s&:(eq ?add-s (str-cat ?node "/add_objects"))) (type ?add-type))
  (ros-msgs-client (service ?rm-s&:(eq ?rm-s (str-cat ?node "/rm_objects"))) (type ?rm-type))
  (not (pddl-service-request-meta (service ?add-s) (meta ?instance)))
  (not (pddl-service-request-meta (service ?rm-s) (meta ?instance)))
  =>
  (modify ?pi-f (busy-with FALSE))
)
