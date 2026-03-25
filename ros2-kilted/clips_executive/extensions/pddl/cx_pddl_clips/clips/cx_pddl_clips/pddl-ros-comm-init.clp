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

(defrule pddl-service-init
" Initiate the service clients for the pddl manager "
  ?node-f <- (pddl-manager (node ?node-name) (ros-comm-init FALSE))
  (not (executive-finalize))
  =>
  ; create clients for all services
  (bind ?service-clients ?*CX-PDDL-SERVICE-CLIENTS*)
  (bind ?index 1)
  (bind ?length (length$ ?service-clients))
  (while (< ?index ?length)
      (bind ?service-name (nth$ ?index ?service-clients))
      (bind ?service-type (nth$ (+ ?index 1) ?service-clients))
      (ros-msgs-create-client
          (str-cat ?node-name "/" ?service-name)
          (str-cat "cx_pddl_interfaces/srv/" ?service-type)
      )
      (bind ?index (+ ?index 2))
  )
  (cx-pddl-interfaces-plan-temporal-create-client (str-cat ?node-name "/temp_plan"))
  (modify ?node-f (ros-comm-init TRUE))
)

(defrule pddl-service-finalize
  (executive-finalize)
  (pddl-manager (node ?node-name))
=>
  (bind ?service-clients ?*CX-PDDL-SERVICE-CLIENTS*)
  (bind ?index 1)
  (bind ?length (length$ ?service-clients))
  (while (< ?index ?length)
      (bind ?service-name (nth$ ?index ?service-clients))
      (bind ?service-type (nth$ (+ ?index 1) ?service-clients))
      (ros-msgs-destroy-client
          (str-cat ?node-name "/" ?service-name)
      )
      (bind ?index (+ ?index 2))
  )
  (cx-pddl-interfaces-plan-temporal-destroy-client (str-cat ?node-name "/temp_plan"))
)
