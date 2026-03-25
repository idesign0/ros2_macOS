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

(deftemplate counter (slot iteration (type INTEGER)))


(defrule tf2-tracked-pose-open
  =>
  (tf2-start-periodic-lookup "world" "turtle1" 0.2)
  (assert (counter (iteration 0)))
)

(defrule tf2-tracked-pose-print
  (tf2-tracked-pose (translation $?trans) (rotation $?rot))
  =>
  (printout green "pos at " ?trans " with rot " ?rot crlf)
  (do-for-fact ((?c counter)) TRUE (modify ?c (iteration (+ 1 ?c:iteration))))
)

(defrule tf2-tracked-pose-stop
  ?pose <- (tf2-tracked-pose (timer ?t))
  ?count <- (counter (iteration 5))
  =>
  (printout blue "Stopping timer " ?t crlf)
  (tf2-stop-periodic-lookup ?t)
  (retract ?pose ?count)
)
