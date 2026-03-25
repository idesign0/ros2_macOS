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

; This file showcases intefaces with specific ros services

(defrule set-bool-client-service-init
" Create a simple client and service using the generated bindings. "
  (not (std-srvs-set-bool-client (service "ros_cx_client")))
  (not (std-srvs-set-bool-service (name "ros_cx_srv")))
=>
  (std-srvs-set-bool-create-client "ros_cx_client")
  (printout info "Created client for /ros_cx_client" crlf)
  (std-srvs-set-bool-create-service "ros_cx_srv")
  (printout info "Created service for /ros_cx_srv" crlf)
)

; this function needs to be defined in order to respond to messages
(deffunction std-srvs-set-bool-service-callback (?service-name ?request ?response)
  (bind ?req-data (std-srvs-set-bool-request-get-field ?request "data"))
  (printout info "Received request on " ?service-name ". Data: " ?req-data crlf)
  (printout info "Received " ?req-data ", responding with same value" crlf)
  (if ?req-data then
    (std-srvs-set-bool-response-set-field ?response "success" TRUE)
    (std-srvs-set-bool-response-set-field ?response "message" (str-cat "Received the request: " ?req-data))
    (assert (send-request))
   else
    (std-srvs-set-bool-response-set-field ?response "success" FALSE)
    (std-srvs-set-bool-response-set-field ?response "message" (str-cat "Received the request: " ?req-data))
  )
)

(defrule std-srvs-send-out-request-to-outbound
  ?sr <- (send-request)
  =>
  ;example usage of sending a request
  (printout info "Additionally, request as client with data: True" crlf)
  (bind ?new-req (std-srvs-set-bool-request-create))
  (std-srvs-set-bool-request-set-field ?new-req "data" TRUE)
  (std-srvs-set-bool-send-request ?new-req "ros_cx_client")
  (std-srvs-set-bool-request-destroy ?new-req)
  (retract ?sr)
)

(defrule set-bool-client-response-received
" Create a simple client and service using the generated bindings. "
  ?msg-fact <- (std-srvs-set-bool-response (service ?service) (msg-ptr ?ptr))
=>
  (bind ?succ (std-srvs-set-bool-response-get-field ?ptr "success"))
  (bind ?msg (std-srvs-set-bool-response-get-field ?ptr "message"))
  (printout green "Received response from " ?service " with: " ?succ " (" ?msg ")" crlf)
  (std-srvs-set-bool-response-destroy ?ptr)
  (retract ?msg-fact)
)
