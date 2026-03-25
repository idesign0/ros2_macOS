.. _usage_ament_index_plugin:

Ament Index Plugin
##################

Source code on :source-master:`GitHub <cx_plugins/ament_index_plugin>`.

.. admonition:: Plugin Class

  cx::AmentIndexPlugin

This plugin provides CLIPS bindings for functions of :rosdoc:`ament_index_cpp`.


Configuration
*************

This plugin has no specific configuration options.

Features
********

Note that all functions listed below catch potential exceptions thrown by `ament_index_cpp` and return `FALSE` in that case.

Functions
~~~~~~~~~

.. code-block:: lisp

  (bind ?ret (ament-index-get-package-prefix ?package-name))
  ; example args: "cx_bringup"
  ; example ret:  "/opt/ros/<ros-distro>

  (bind ?ret (ament-index-get-package-share-directory ?package-name))
  ; example args: "cx_bringup"
  ; example ret:  "/opt/ros/<ros-distro>/share/cx_bringup"

  (bind ?ret (ament-index-get-packages-with-prefixes))
  ; example ret:  ("rclcpp" "/opt/ros/<ros-distro>" "cx_bringup" "/opt/ros/<ros-distro>" ...)

  (bind ?ret (ament-index-has-resource ?res-type ?res-name))
  ; example args: "vendor_packages" "clips_vendor"
  ; example ret:  TRUE

  (bind ?ret (ament-index-get-resource ?res-type ?res-name))
  ; example args: "vendor_packages" "clips_vendor"
  ; example ret: ("opt/clips_vendor" "/opt/ros/<ros-distro>")

  (bind ?ret (ament-index-get-resources ?res-type))
  ; example args: "vendor_packages"
  ; example ret: ("clips_vendor" "/opt/ros/<ros-distro>" "<other-vendor>" "<path-to-other-vendor"> ...)

  (bind ?prefix (ament-index-get-search-paths))
  ; example ret: ("/opt/ros/<ros-distro>" "<path-to-other-prefix>" ...)

Usage Example
*************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ament_index.yaml

It calls all of the provided functions once and prints their output.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ament_index.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_ament_index"]
      cx_ament_index:
        plugins: ["ament_index", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      ament_index:
        plugin: "cx::AmentIndexPlugin"
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        batch: [
          "clips/plugin_examples/ament-index.clp"]

Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ament-index.clp`.

.. code-block:: lisp

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
