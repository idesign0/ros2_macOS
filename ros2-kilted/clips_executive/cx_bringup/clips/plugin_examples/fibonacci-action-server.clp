; Copyright (c) 2024-2026 Carologistics
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

; This file showcases intefaces with ROS action servers and clients

(deftemplate fibonacci
  (slot uuid (type STRING))
  (slot order (type INTEGER))
  (slot progress (type INTEGER))
  (multislot sequence (type INTEGER))
  (slot result (type INTEGER))
  (slot canceled (type SYMBOL) (allowed-values FALSE TRUE))
  (slot last-computed (type FLOAT))
)

(deffunction example-interfaces-fibonacci-handle-goal-callback (?server ?goal ?uuid)
" Overrides default callback, printing a message before accepting. "
  (printout blue "Accepting goal, uuid: " ?uuid crlf)
  ; (return 1) ; REJECT
  (return 2) ; ACCEPT_AND_EXECUTE
  ; (return 3) ; ACCEPT_AND_DEFER
)

(deffunction example-interfaces-fibonacci-cancel-goal-callback (?server ?goal ?goal-handle)
" Overrides default callback, marking the associated fibonacci fact for cancelation. "
  (do-for-fact ((?f fibonacci)) (eq ?f:uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?goal-handle))
    (printout blue "Accepting cancelation of goal with uuid " ?f:uuid crlf)
    (modify ?f (canceled TRUE))
    (return 2) ; ACCEPT
  )
  (return 1) ; REJECT
)

(defrule fibonacci-action-server-init
" Create a simple server using the generated bindings. "
  (not (example-interfaces-fibonacci-server (name "ros_cx_fibonacci")))
  (not (executive-finalize))
=>
  (example-interfaces-fibonacci-create-server "ros_cx_fibonacci")
  (printout blue "Created server for /ros_cx_fibonacci" crlf)
)

(defrule fibonacci-goal-accepted-start-compute
" Create an initial fibonacci fact for an accepted goal. "
  (example-interfaces-fibonacci-accepted-goal (server ?server) (server-goal-handle-ptr ?ptr))
  (not (fibonacci (uuid ?uuid&:(eq ?uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?ptr)))))
  =>
  (if (not (example-interfaces-fibonacci-server-goal-handle-is-canceling ?ptr)) then
    (bind ?goal (example-interfaces-fibonacci-server-goal-handle-get-goal ?ptr))
    (bind ?order (example-interfaces-fibonacci-goal-get-field ?goal "order"))
    (bind ?uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?ptr))
    (assert (fibonacci (uuid ?uuid) (order ?order) (progress 2) (result 0) (sequence (create$ 0 1)) (last-computed (now))))
   else
    (printout error "Somehow the goal is canceling already" crlf)
  )
  ; do not destroy the server goal handle here, only do it once the goal is fully processed and finished
  ; (example-interfaces-fibonacci-server-goal-handle-destroy ?ptr)
)

(defrule fibonacci-compute-next
" Compute the next number in the sequence and send out feedback accordingly. "
  (example-interfaces-fibonacci-accepted-goal (server ?server) (server-goal-handle-ptr ?ptr))
  ?f <- (fibonacci (order ?order) (progress ?remaining&:(>= ?order ?remaining))
  (last-computed ?computed) (result ?old-res) (sequence $?seq) (uuid ?uuid) (canceled FALSE))
  (time ?now&:(> (- ?now ?computed) 1))
  (test (eq ?uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?ptr)))
  =>
  (bind ?step (+ ?remaining 1))
  (bind ?res (+ (nth$ ?remaining ?seq) (nth$ (- ?remaining 1) ?seq)))
  (printout blue "Computing partial result fibonacci(" ?remaining ") = " ?res crlf)
  (bind ?seq (create$ ?seq ?res))
  (modify ?f (progress ?step) (result (+ ?old-res ?res)) (sequence ?seq))
  (bind ?feedback (example-interfaces-fibonacci-feedback-create))
  (example-interfaces-fibonacci-feedback-set-field ?feedback "sequence" ?seq)
  (example-interfaces-fibonacci-server-goal-handle-publish-feedback ?ptr ?feedback)
  (example-interfaces-fibonacci-feedback-destroy ?feedback)
  (modify ?f (last-computed ?now))
)

(defrule fibonacci-compute-done
" Send the final goal result once the computation is finished. "
  ?ag <- (example-interfaces-fibonacci-accepted-goal (server ?server) (server-goal-handle-ptr ?ptr))
  ?f <- (fibonacci (order ?order) (progress ?remaining&:(< ?order ?remaining)) (result ?old-res) (sequence $?seq) (uuid ?uuid&:(eq ?uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?ptr))))
  =>
  (printout blue "Final fibonacci sequence: " ?seq crlf)
  (bind ?result (example-interfaces-fibonacci-result-create))
  (example-interfaces-fibonacci-result-set-field ?result "sequence" ?seq)
  (example-interfaces-fibonacci-server-goal-handle-succeed ?ptr ?result)
  (example-interfaces-fibonacci-result-destroy ?result)
  (example-interfaces-fibonacci-server-goal-handle-destroy ?ptr)
  (retract ?f)
  (retract ?ag)
)

(defrule fibonacci-compute-canceled
" Stop computation and send the partial result in case the goal is canceled. "
  ?ag <- (example-interfaces-fibonacci-accepted-goal (server ?server) (server-goal-handle-ptr ?ptr))
  ?f <- (fibonacci (sequence $?seq) (canceled TRUE) (uuid ?uuid&:(eq ?uuid (example-interfaces-fibonacci-server-goal-handle-get-goal-id ?ptr))))
  =>
  (printout blue "Canceling fibonacci sequence: " ?seq crlf)
  (bind ?result (example-interfaces-fibonacci-result-create))
  (example-interfaces-fibonacci-result-set-field ?result "sequence" ?seq)
  (example-interfaces-fibonacci-server-goal-handle-canceled ?ptr ?result)
  (example-interfaces-fibonacci-result-destroy ?result)
  (example-interfaces-fibonacci-server-goal-handle-destroy ?ptr)
  (retract ?f)
  (retract ?ag)
)

(defrule fibonacci-server-cleanup
  (executive-finalize)
  (example-interfaces-fibonacci-server (name ?server))
  =>
  (example-interfaces-fibonacci-destroy-server ?server)
)

(defrule fibonacci-goal-response-cleanup
  (executive-finalize)
  ?f <- (example-interfaces-fibonacci-goal-response (client-goal-handle-ptr ?p))
  =>
  (example-interfaces-fibonacci-client-goal-handle-destroy ?p)
  (retract ?f)
)

(defrule fibonacci-accepted-goal-cleanup
  (executive-finalize)
  ?f <- (example-interfaces-fibonacci-accepted-goal (server-goal-handle-ptr ?p))
  =>
  (example-interfaces-fibonacci-server-goal-handle-destroy ?p)
  (retract ?f)
)
