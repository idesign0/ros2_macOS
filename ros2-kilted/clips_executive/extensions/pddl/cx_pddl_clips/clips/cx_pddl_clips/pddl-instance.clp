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

(defrule pddl-instance-request
  (declare (salience ?*PRIORITY-PDDL-INSTANCES*))
  (pddl-instance (state PENDING) (name ?instance) (domain ?domain) (problem ?problem) (directory ?dir))
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/add_pddl_instance"))) (type ?type))
  (not (pddl-service-request-meta (service ?s)))
  (time ?any-time) ; used to continuously attempt to request the service until success
=>
  (bind ?new-req (ros-msgs-create-request ?type))
  (ros-msgs-set-field ?new-req "name" ?instance)
  (ros-msgs-set-field ?new-req "directory" ?dir)
  (ros-msgs-set-field ?new-req "domain_file" ?domain)
  (ros-msgs-set-field ?new-req "problem_file" ?problem)
  (bind ?id (ros-msgs-async-send-request ?new-req ?s))
  (if ?id then
    (assert (pddl-service-request-meta (service ?s) (request-id ?id) (meta ?instance)))
   else
    (printout warn "Sending of request failed, is the service " ?s " running?" crlf)
  )
  (ros-msgs-destroy-message ?new-req)
)

(defrule pddl-instance-response-received
" Get response, make sure that it succeeded and delete it afterwards."
  (pddl-manager (node ?node))
  (ros-msgs-client (service ?s&:(eq ?s (str-cat ?node "/add_pddl_instance"))) (type ?type))
  ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
  ?req-meta <- (pddl-service-request-meta (service ?) (request-id ?id) (meta ?meta))
  ?instance-f <-(pddl-instance (state PENDING) (name ?meta))
=>
  (bind ?success (ros-msgs-get-field ?ptr "success"))
  (bind ?error (ros-msgs-get-field ?ptr "error"))
  (if ?success then
    (modify ?instance-f (state LOADED))
   else
    (modify ?instance-f (state ERROR) (error ?error))
    (printout error "Failed to set problem instance \"" ?meta "\":" ?error crlf)
  )
  (ros-msgs-destroy-message ?ptr)
  (retract ?msg-f)
  (retract ?req-meta)
)
