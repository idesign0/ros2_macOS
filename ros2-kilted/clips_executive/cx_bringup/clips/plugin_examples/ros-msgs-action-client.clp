
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

(defrule ros-msgs-action-client-init
" Create a client for fibonacci example action server."
  (not (ros-msgs-action-client (server "ros_cx_fibonacci")))
  (not (executive-finalize))
=>
  (ros-msgs-create-action-client "ros_cx_fibonacci" "example_interfaces/action/Fibonacci")
  (printout yellow "Created client for /ros_cx_fibonacci" crlf)
)

(defrule ros-msgs-action-client-send-goal
" Send a goal request for the 10th fibonacci number. "
  (ros-msgs-action-client (server ?server))
  (not (send-request))
=>
  (assert (send-request))
  (bind ?new-req (ros-msgs-create-goal-request "example_interfaces/action/Fibonacci"))
  (ros-msgs-set-field ?new-req "order" 5)
  (bind ?val (ros-msgs-get-field ?new-req "order"))
  (bind ?id (ros-msgs-async-send-goal ?new-req ?server))
  (assert (fibonacci-goal ?new-req))
  (printout yellow "Sending goal fibonacci(5)" crlf)
  ; do not destroy goal directly, it is kept alive
  ;(ros-msgs-destroy-message ?new-req)
)

(defrule ros-msgs-action-client-check-client-goal-handle
" Process the response for the requested goal."
  (ros-msgs-action-client (server ?server))
  (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?cgh-ptr))
=>
  (bind ?g-id (ros-msgs-client-goal-handle-get-goal-id ?cgh-ptr))
  (bind ?stamp (ros-msgs-client-goal-handle-get-goal-stamp ?cgh-ptr))
  (printout yellow "Response: goal id " ?g-id " at stamp " ?stamp " with status " (ros-msgs-client-goal-handle-get-status ?cgh-ptr) crlf)
)

(defrule ros-msgs-action-client-read-feedback
  (ros-msgs-action-client (server ?server))
 ?fact <- (ros-msgs-feedback (server ?server) (client-goal-handle-ptr ?cgh-ptr) (feedback-ptr ?msg-ptr))
=>
  (bind ?seq (ros-msgs-get-field ?msg-ptr "sequence"))
  (printout yellow "partial sequence: " ?seq crlf)
  (ros-msgs-destroy-message ?msg-ptr)
  (if (= (length$ ?seq) 7) then
     (assert (cancel-goal))
  )
  (retract ?fact)
)

(defrule ros-msgs-action-client-cleanup-after-wrapped-result
  ?response-f <- (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?cgh-ptr))
  ?result-f <- (ros-msgs-wrapped-result (server ?server) (goal-id ?id) (code ?code) (result-ptr ?res-ptr))
  (test (eq ?id (ros-msgs-client-goal-handle-get-goal-id ?cgh-ptr)))
  ?req-f <- (fibonacci-goal ?req-ptr)
=>
  (printout yellow "Action result: " ?code crlf)
  (if (eq ?code SUCCEEDED)
    then
    (bind ?seq (ros-msgs-get-field ?res-ptr "sequence"))
    (printout yellow "Final fibonacci sequence: " ?seq crlf)
  )
  (if (eq ?code CANCELED)
    then
    (bind ?seq (ros-msgs-get-field ?res-ptr "sequence"))
    (printout yellow "Canceled fibonacci sequence: " ?seq crlf)
  )
  (ros-msgs-destroy-message ?res-ptr)
  (ros-msgs-destroy-message ?req-ptr)
  (ros-msgs-destroy-client-goal-handle ?cgh-ptr)
  (retract ?response-f ?result-f ?req-f)
)

(defrule ros-msgs-action-client-send-goal-that-cancels-later
" Send out a second goal for the fibonacci sequence of order 10 once the previous goal is completed. "
  (ros-msgs-action-client (server ?server))
  (send-request)
  (not (fibonacci-goal ?))
  (not (send-request-again))
=>
  (assert (send-request-again))
  (printout yellow "Request fibonacci(10), will cancel before finish" crlf)
  (bind ?new-req (ros-msgs-create-goal-request "example_interfaces/action/Fibonacci"))
  (ros-msgs-set-field ?new-req "order" 10)
  (bind ?val (ros-msgs-get-field ?new-req "order"))
  (bind ?id (ros-msgs-async-send-goal ?new-req ?server))
  (assert (fibonacci-goal ?new-req))
  ; do not destroy goal directly, it is kept alive
  ;(ros-msgs-destroy-message ?new-req)
)

(defrule ros-msgs-action-client-request-cancel
" Send out a cancel request after the partial feedback provides a sequence of length 7. "
  (cancel-goal)
  (ros-msgs-action-client (server ?server))
  (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?ghp))
=>
  (ros-msgs-async-cancel-goal ?server ?ghp)
  (printout yellow "Canceling current goal" crlf)
)

(defrule ros-msgs-action-client-cleanup-cancel-response
" Process the cancel response and clean up afterwards. "
  ?msg-f <- (ros-msgs-cancel-response (cancel-response-ptr ?msg))
=>
  (bind ?error-code (ros-msgs-get-field ?msg "return_code"))
  (bind ?goals-canceling (ros-msgs-get-field ?msg "goals_canceling"))
  (foreach ?g ?goals-canceling
    (bind ?goal-id-msg (ros-msgs-get-field ?g "goal_id"))
    (bind ?goal-id (ros-msgs-get-field ?goal-id-msg "uuid"))
    (bind ?goal-id-str (ros-msgs-goal-uuid-to-string ?goal-id))
    (printout yellow "Canceled goal with id: " ?goal-id-str crlf)
    (ros-msgs-destroy-message ?goal-id-msg)
  )
  (ros-msgs-destroy-message ?msg)
)

(defrule ros-msgs-action-client-finalize
" Delete the client on executive finalize. "
  (executive-finalize)
  (ros-msgs-action-client (server ?server))
=>
  (printout info "Destroying action client" crlf)
  (ros-msgs-destroy-action-client ?server)
)
