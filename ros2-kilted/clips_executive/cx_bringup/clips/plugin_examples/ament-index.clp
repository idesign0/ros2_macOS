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

(deffunction print-pairs (?pairs ?key-str ?value-str)
  (bind ?length (length$ ?pairs))
  (bind ?index 1)

  (while (< ?index (min 10 ?length))
     (bind ?key (nth$ ?index ?pairs))
     (bind ?value (nth$ (+ ?index 1) ?pairs))
     (printout green "    " ?key-str ": " ?key " "?value-str ": " ?value crlf)
     (bind ?index (+ ?index 2))
  )
  (if (> ?length 10) then
   (printout green "    " " ... " crlf)
  )
)
(deffunction print-in-lines (?multi)
  (bind ?length (length$ ?multi))
  (bind ?index 1)

  (while (< ?index (min 5 ?length))
     (bind ?key (nth$ ?index ?multi))
     (printout green "    " ?key crlf)
     (bind ?index (+ ?index 1))
  )
  (if (> ?length 5) then
   (printout green "    " " ... " crlf)
  )
)

(printout info "(ament-index-get-package-prefix \"cx_bringup\")" crlf)
(printout green "    " (ament-index-get-package-prefix "cx_bringup") crlf)

(printout info "(ament-index-get-packages-with-prefixes)" crlf)
(print-pairs (ament-index-get-packages-with-prefixes) "Package" "Prefix")

(printout info "(ament-index-get-package-share-directory \"rclcpp\")" crlf)
(printout green "    " (ament-index-get-package-share-directory "rclcpp") crlf)

(printout info "(ament-index-get-resource \"vendor_packages\" \"clips_vendor\")" crlf)
(print-pairs (ament-index-get-resource "vendor_packages" "clips_vendor") "Content" "Path")

(printout info "(ament-index-get-resources \"vendor_packages\")" crlf)
(print-pairs (ament-index-get-resources "vendor_packages") "Package" "Prefix")

(printout info "(ament-index-get-search-paths)" crlf)
(print-in-lines (ament-index-get-search-paths))
