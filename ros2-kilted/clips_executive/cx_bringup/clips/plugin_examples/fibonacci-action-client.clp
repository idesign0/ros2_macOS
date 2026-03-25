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


(defrule fibonacci-action-client-init
" Create a simple client using the generated bindings. "
  (not (example-interfaces-fibonacci-client (server "ros_cx_fibonacci")))
  (not (executive-finalize))
=>
  (example-interfaces-fibonacci-create-client "ros_cx_fibonacci")
  (printout green "Created client for /ros_cx_fibonacci" crlf)
)

(defrule fibonacci-client-send-goal
" Request computation of the fibonacci sequence of order 5. "
  (example-interfaces-fibonacci-client (server ?server))
  (not (send-request))
  =>
  (assert (send-request))
  (bind ?goal (example-interfaces-fibonacci-goal-create))
  (assert (fibonacci-goal ?goal))
  (example-interfaces-fibonacci-goal-set-field ?goal "order" 5)
  (example-interfaces-fibonacci-send-goal ?goal ?server)
  (printout green "Sending goal fibonacci(5)" crlf)

  ; do not destroy the goal here, only do it once the goal is fully processed and finished
  ; (example-interfaces-fibonacci-goal-destroy ?goal)
)

(defrule fibonacci-client-get-feedback
" Print any partial compuation result received so far and perpare cancelation once the received sequence is of length 7. "
  (declare (salience 100))
  ?f <- (example-interfaces-fibonacci-goal-feedback (server ?server) (client-goal-handle-ptr ?ghp) (feedback-ptr ?fp))
  =>
  (bind ?g-id (example-interfaces-fibonacci-client-goal-handle-get-goal-id ?ghp))
  (bind ?g-stamp (example-interfaces-fibonacci-client-goal-handle-get-goal-stamp ?ghp))
  (bind ?g-status (example-interfaces-fibonacci-client-goal-handle-get-status ?ghp))
  (bind ?g-is-f-aware (example-interfaces-fibonacci-client-goal-handle-is-feedback-aware ?ghp))
  (bind ?g-is-r-aware (example-interfaces-fibonacci-client-goal-handle-is-result-aware ?ghp))
  ; the stamp seems to be broken (looks like a rclcpp_action issue)
  (printout debug "[" (- (now) ?g-stamp) "] " ?g-status " " ?g-id " f " ?g-is-f-aware " r " ?g-is-r-aware crlf)
  (bind ?part-seq (example-interfaces-fibonacci-feedback-get-field ?fp "sequence"))
  (printout green "partial sequence: " ?part-seq   crlf)
  (if (= (length$ ?part-seq) 7) then
     (assert (cancel-goal))
  )
  (example-interfaces-fibonacci-feedback-destroy ?fp)
  (retract ?f)
)

(defrule fibonacci-client-cleanup-after-wrapped-result
" Process result of finished compuatation, printing the final sequence. "
  (declare (salience 10))
  ?f <- (example-interfaces-fibonacci-goal-response (server ?server) (client-goal-handle-ptr ?ghp))
  ?g <- (example-interfaces-fibonacci-wrapped-result (server ?server) (goal-id ?uuid) (code ?code) (result-ptr ?rp))
  (test (eq ?uuid (example-interfaces-fibonacci-client-goal-handle-get-goal-id ?ghp)))
  ?request-goal <- (fibonacci-goal ?goal)
  (time ?now)
  =>
  (bind ?g-status (example-interfaces-fibonacci-client-goal-handle-get-status ?ghp))
  (if (> ?g-status 3) then ; status is final in one way or another
    (bind ?seq (example-interfaces-fibonacci-result-get-field ?rp "sequence"))
    (if (= ?g-status 4) then
      (printout green "Final fibonacci sequence: " ?seq crlf)
    )
    (if (= ?g-status 5) then
      (printout green "Canceled fibonacci sequence: " ?seq crlf)
    )
    (example-interfaces-fibonacci-result-destroy ?rp)
    (retract ?g)
    (bind ?g-id (example-interfaces-fibonacci-client-goal-handle-get-goal-id ?ghp))
    (bind ?g-stamp (example-interfaces-fibonacci-client-goal-handle-get-goal-stamp ?ghp))
    (bind ?g-is-f-aware (example-interfaces-fibonacci-client-goal-handle-is-feedback-aware ?ghp))
    (bind ?g-is-r-aware (example-interfaces-fibonacci-client-goal-handle-is-result-aware ?ghp))
    (printout debug "Final goal response [" (- (now) ?g-stamp) "] " ?uuid " " ?g-status " " ?g-id " f " ?g-is-f-aware " r " ?g-is-r-aware crlf)
    (example-interfaces-fibonacci-client-goal-handle-destroy ?ghp)
    (retract ?f)
    (example-interfaces-fibonacci-goal-destroy ?goal)
    (retract ?request-goal)
  )
)

(defrule fibonacci-client-send-goal-that-cancels-later
" Send out a second goal for the fibonacci sequence of order 10 once the previous goal is completed. "
  (example-interfaces-fibonacci-client (server ?server))
  (send-request)
  (not (fibonacci-goal ?))
  (not (send-request-again))
  =>
  (printout green "Request fibonacci(10), will cancel before finish" crlf)
  (assert (send-request-again))
  (bind ?goal (example-interfaces-fibonacci-goal-create))
  (assert (fibonacci-goal ?goal))
  (example-interfaces-fibonacci-goal-set-field ?goal "order" 10)
  (example-interfaces-fibonacci-send-goal ?goal ?server)
)

(defrule fibonacci-request-cancel
" Send out a cancel request after the partial feedback provides a sequence of length 7. "
  (cancel-goal)
  (example-interfaces-fibonacci-client (server ?server))
  (example-interfaces-fibonacci-goal-response (server ?server) (client-goal-handle-ptr ?ghp))
=>
  (example-interfaces-fibonacci-client-cancel-goal ?server ?ghp)
  (printout green "Canceling current goal" crlf)
)

(defrule fibonacci-client-cleanup
  (executive-finalize)
  (example-interfaces-fibonacci-client (server ?client))
  =>
  (example-interfaces-fibonacci-destroy-client ?client)
)

(defrule fibonacci-accepted-goal-cleanup
  (executive-finalize)
  ?f <- (fibonacci-goal ?p)
  =>
  (example-interfaces-fibonacci-goal-destroy ?p)
  (retract ?f)
)
