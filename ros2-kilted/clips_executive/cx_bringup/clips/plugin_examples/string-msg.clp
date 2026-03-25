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

; This file showcases intefaces with specific ROS msgs

(defrule string-pub-sub-init
" Create a simple publisher and subscriber using the generated bindings. "
  (not (std-msgs-string-subscription (topic "ros_cx_in")))
  (not (std-msgs-string-publisher (topic "ros_cx_out")))
=>
  (printout green "------------------ ")
  (printout bold  "string msg example")
  (printout green " ------------------" crlf)
  (printout green "| ")
  (printout blue  "Creates a subscription to /ros_cx_in and a publisher")
  (printout green " |" crlf)
  (printout green "| ")
  (printout  blue "on /ros_cx_out. Whenever a message on /ros_cx_in is ")
  (printout green " |" crlf)
  (printout green "| ")
  (printout  blue "received, a response is published on /ros_cx_out    ")
  (printout green " |" crlf)
  (printout green "| ")
  (printout  blue "with content \"Hello World!\".                        ")
  (printout green " |" crlf)
  (printout green "--------------------------------------------------------" crlf)
  (std-msgs-string-create-publisher "ros_cx_out")
  (printout info "Created publisher for /ros_cx_out" crlf)
  (std-msgs-string-create-subscription "ros_cx_in")
  (printout info "Listening for String messages on /ros_cx_in" crlf)
)

(defrule string-publisher-recv-and-answer
" React to incoming messages and answer (on a different topic) "
  (std-msgs-string-subscription (topic ?sub))
  ?msg-f <- (std-msgs-string-message (topic ?sub) (msg-ptr ?inc-msg))
  (std-msgs-string-publisher (topic ?pub))
  =>
  ; fetch the content of the message and print it
  (bind ?recv (std-msgs-string-get-field ?inc-msg "data"))
  (printout blue "Recieved via " ?sub " :" ?recv crlf)
  ; make sure to actually destroy the message to free heap-allocated memory for it, once the message is processed and can be removed
  (std-msgs-string-destroy-message ?inc-msg)
  (retract ?msg-f)

  ; example of how to create and send a new message
  (printout green "Sending Hello World Message in response!" crlf)
  (bind ?msg (std-msgs-string-create-message))
  (std-msgs-string-set-field ?msg "data" "Hello world!")
  (std-msgs-string-publish ?msg ?pub)
  ; destroy the msg after usage to free up the memory
  (std-msgs-string-destroy-message ?msg)
)


(defrule string-msg-pub-sub-finalize
" Delete the subscription and publisher on executive finalize. "
  (executive-finalize)
  (std-msgs-string-subscription (topic ?sub))
  (std-msgs-string-publisher (topic ?pub))
=>
  (printout debug "Destroying subscription " ?sub crlf)
  (printout debug "Destroying publisher " ?pub crlf)
  (std-msgs-string-destroy-subscription ?sub)
  (std-msgs-string-destroy-publisher ?pub)
)


(defrule ros-msgs-message-cleanup
" Delete any incoming msg on executive finalize. "
  (executive-finalize)
  ?msg-f <- (std-msgs-string-message (msg-ptr ?ptr))
=>
  (std-msgs-string-destroy-message ?ptr)
  (retract ?msg-f)
)
