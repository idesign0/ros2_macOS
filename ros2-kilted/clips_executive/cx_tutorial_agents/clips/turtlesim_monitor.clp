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

(defglobal
  ?*TURTLE-POSE-TOPIC* = "turtle1/pose"
  ?*TURTLE-POSE-TYPE* = "turtlesim/msg/Pose"
  ?*SAFE-AREA-LOWER-BOUND* = 1.0
  ?*SAFE-AREA-UPPER-BOUND* = 10.0
)

(defrule turtle-pose-topic-create-subscription
" Create subscription to monitor the pose."
=>
  (unwatch facts ros-msgs-message)
  (unwatch rules turtle-pose-receive)
  (ros-msgs-create-subscription ?*TURTLE-POSE-TOPIC* ?*TURTLE-POSE-TYPE*)
  (printout info "Listening to " ?*TURTLE-POSE-TOPIC* crlf)
)

(defrule turtle-pose-receive
" React to incoming messages andcheck for critical pose. "
  (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
  ?msg-f <- (ros-msgs-message (topic ?t) (msg-ptr ?inc-msg))
  =>
  (bind ?x (ros-msgs-get-field ?inc-msg "x"))
  (bind ?y (ros-msgs-get-field ?inc-msg "y"))
  (if (or
    (< ?x ?*SAFE-AREA-LOWER-BOUND*)
    (< ?y ?*SAFE-AREA-LOWER-BOUND*)
    (> ?x ?*SAFE-AREA-UPPER-BOUND*)
    (> ?y ?*SAFE-AREA-UPPER-BOUND*)
  )
  then
    (printout yellow "turtle out of bounds" crlf)
    (assert (turtle-out-of-bounds))
  )
  (ros-msgs-destroy-message ?inc-msg)
  (retract ?msg-f)
)

(defrule turtle-pose-destroy-subscription
" Delete the subscription on executive finalize. "
  (executive-finalize)
  (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
=>
  (printout info "Destroying subscription for " ?*TURTLE-POSE-TOPIC* crlf)
  (ros-msgs-destroy-subscription ?*TURTLE-POSE-TOPIC*)
)
