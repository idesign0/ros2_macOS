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

(define (domain blocksworld)
    (:types
        block - object
    )


    (:predicates
        (on ?x - block ?y - block) ; object ?x is on ?object ?y
        (on-table ?x - block) ; ?x is directly on the table
        (clear ?x - block) ; ?x has nothing on it
        (arm-empty) ; robot isn't holding anything
        (holding ?x - block)) ; robot is holding ?x

    (:durative-action pick-up
        :parameters (?ob - block)
        :duration (= ?duration 1)
        :condition
            (and
                (at start (clear ?ob))
                (at start (on-table ?ob))
                (at start (arm-empty)))
        :effect
            (and
                (at end (not (on-table ?ob)))
                (at end (not (clear ?ob)))
                (at end (not (arm-empty)))
                (at end (holding ?ob))))

    (:durative-action put-down
        :parameters (?ob - block)
        :duration (= ?duration 1)
        :condition (at start (holding ?ob))
        :effect
            (and
                (at end (not (holding ?ob)))
                (at end (clear ?ob))
                (at end (arm-empty))
                (at end (on-table ?ob)))
    )

    (:durative-action stack
        :parameters (?ob1 - block ?ob2 - block)
        :duration (= ?duration 2)
        :condition (and (at start (holding ?ob1)) (at start (clear ?ob2)))
        :effect
            (and
                (at end (not (holding ?ob1)))
                (at end (not (clear ?ob2)))
                (at end (clear ?ob1))
                (at end (arm-empty))
                (at end (on ?ob1 ?ob2)))
    )

    (:durative-action unstack
        :parameters (?ob1 - block ?ob2 - block)
        :duration (= ?duration 1)
        :condition
            (and
                (at start (on ?ob1 ?ob2))
                (at start (clear ?ob1))
                (at start (arm-empty)))
        :effect
            (and
                (at end (holding ?ob1))
                (at end (clear ?ob2))
                (at end (not (clear ?ob1)))
                (at end (not (arm-empty)))
                (at end (not (on ?ob1 ?ob2))))
    )
);
