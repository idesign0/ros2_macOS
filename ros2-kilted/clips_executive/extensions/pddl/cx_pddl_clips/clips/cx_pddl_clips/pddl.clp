
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

; batch this file to load all of the pddl definitions
(foreach ?f (create$
deftemplates.clp
saliences.clp
pddl-instance.clp
pddl-object-change.clp
pddl-fluent-change.clp
pddl-numeric-fluent-change.clp
pddl-action-condition.clp
pddl-action-get-effect.clp
pddl-action-names.clp
pddl-clear-goals.clp
pddl-create-goal-instance.clp
pddl-get-fluents.clp
pddl-get-numeric-fluents.clp
pddl-get-predicates.clp
pddl-get-type-objects.clp
pddl-planning-filter.clp
pddl-set-goals.clp
)
(load* (str-cat (ament-index-get-package-share-directory "cx_pddl_clips") "/clips/cx_pddl_clips/" ?f))
)
