.. _usage_protobuf_plugin:

Protobuf Plugin
###############

Source code on :source-master:`GitHub <cx_plugins/protobuf_plugin>`.

.. admonition:: Plugin Class

  cx::ProtobufPlugin

This plugin enables communication via protobuf through :docsite:`protobuf_comm`, a C++ library to handle communication via Protocol Buffers (`protobuf`_) using a simple framing protocol.

.. note::

  This Plugin was ported from the fawkes and thus is subject to the GPLv2+ license.

Configuration
*************

:`pkg_share_dirs`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    When specifying relative paths, look at the share directories of the listed packages to resolve them (in the specified order).

:`proto_paths`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    Location to directories containing .proto files. The messages found in the given directories are registered automatically for usage.
    Supports absolute paths or relative paths using the share directories specified above.


Features
********

The utilized :docsite:`protobuf_comm` library (de)-serializes proto using a framing protocol that other communication endpoints therefore need to adhere to, as well.
Refer to it's documentation to get details on the layout of the frame headers.

It requires messages using `proto2`_ syntax that additionally define a ``CompType`` enum acting as a unique identifier (using a tmple ``COMP_ID`` and ``MESSAGE_TYPE``.
An example message is depicted below:

.. code-block:: proto

  message SearchRequest {
    enum CompType {
      COMP_ID = 100;
      MSG_TYPE = 1;
    }
    required string query = 1;
  }

Messages can be exchanged either via establishing TCP Client-Server connections or via UDP broadcast peers.

Registering Messages
~~~~~~~~~~~~~~~~~~~~

Messages need to be registered before they can be used.
The easiest way is to utilize the configuration options to specify directories that host proto definitions (``.proto`` files).
This automatically registers all found messages when the plugin is loaded in a CLIPS environment.

It is also possible to instead register types explicitly by looking them up from a linked library.
However, this requires to build a shared library from proto files and then link it together with the :docsite:`cx_protobuf_plugin` plugin, creating a new linked plugin.

In order to accomplish this, this plugin offers two cmake macros:

.. code-block:: cmake

  # Generates a new plugin called NEW_PLUGIN_NAME (should be a name in CamelCase, e.g., CustomProtobufPlugin) given a list of protobuf message definitions PROTO_FILES.
  cx_generate_linked_protobuf_plugin_from_proto(NEW_PLUGIN_NAME PROTO_FILES)
  # Generates a new plugin called NEW_PLUGIN_NAME (should be a name in CamelCase, e.g., CustomProtobufPlugin) given a shared library that is linked against via target_link_libraries().
  cx_generate_linked_protobuf_plugin_from_lib(NEW_PLUGIN_NAME SHARED_LIBRARY_TARGET)

Facts
~~~~~

.. code-block:: lisp

  ; Asserted whenever a message is received
  (deftemplate protobuf-msg
    (slot type (type STRING))            ; (package + "." +) message-name
    (slot comp-id (type INTEGER))
    (slot msg-type (type INTEGER))
    (slot rcvd-via (type SYMBOL)
      (allowed-values STREAM BROADCAST)
    )
    (multislot rcvd-from                 ; address and port
      (cardinality 2 2)
    )
    (slot rcvd-at (type FLOAT))          ; ros timestamp in seconds
    (slot client-type (type SYMBOL)
      (allowed-values SERVER CLIENT PEER)
    )
    (slot client-id (type INTEGER))
    (slot ptr (type EXTERNAL-ADDRESS))
  )

  ; Asserted whenever a message handled by a  client could not be processed
  (deftemplate protobuf-receive-failed
    (slot comp-id (type INTEGER))
    (slot msg-type (type INTEGER))
    (slot rcvd-via (type SYMBOL)
      (allowed-values STREAM BROADCAST)
    )
    (multislot rcvd-from (cardinality 2 2))
    (slot client-id (type INTEGER))
    (slot message (type STRING))
  )

  ; Asserted whenever a message handled by a server could not be processed
  (deftemplate protobuf-server-receive-failed
    (slot comp-id (type INTEGER))
    (slot msg-type (type INTEGER))
    (slot rcvd-via (type SYMBOL)
      (allowed-values STREAM BROADCAST)
    )
    (multislot rcvd-from (cardinality 2 2))
    (slot client-id (type INTEGER))
    (slot message (type STRING))
  )

  ; asynchronously asserted once a client is created via pb-connect
  (protobuf-client-connected ?client-id)
  ; asynchronously asserted once a client is disconnected via pb-disconnect
  (protobuf-client-disconnected ?client-id)
  ; asynchronously asserted once a server is created via pb-server-enable
  (protobuf-server-client-connected ?client-id ?endpoint ?port)
  ; asynchronously asserted once a server is created via pb-server-disable
  (protobuf-server-client-connected ?client-id)

Functions
~~~~~~~~~

.. code-block:: lisp

  ; functions for processing messages:
  (bind ?res (pb-field-names ?msg))
  (bind ?res (pb-field-type ?msg ?field-name))
  (bind ?res (pb-has-field ?msg ?field-name))
  (bind ?res (pb-field-label ?msg ?field-name))
  (bind ?res (pb-field-value ?msg ?field-name))
  (bind ?res (pb-field-list ?msg ?field-name))
  (bind ?res (pb-field-is-list ?msg ?field-name))
  (bind ?res (pb-create ?full-name))
  (pb-set-field ?msg ?field-name ?value)
  (pb-add-list ?msg ?field-name ?list)
  ;
  (bind ?res (pb-tostring ?msg))

  ; functions for using a stream server or clients
  (pb-server-enable ?port)
  (pb-server-disable)
  (pb-send ?client-id ?msg)
  (bind ?res (pb-connect ?host ?port))
  (pb-disconnect ?client-id)

  ; functions for using broadcast peers
  (bind ?res (pb-peer-create ?address ?port))
  (bind ?res (pb-peer-create-local ?address ?send-port ?recv-port))
  (bind ?res (pb-peer-create-crypto ?address ?port ?crypto ?cypher))
  (bind ?res (pb-peer-create-local-crypto ?address ?send-port ?recv-port ?crypto ?cypher))
  (pb-peer-destroy ?peer-id)
  (pb-peer-setup-crypto ?peer-id ?key ?cypher)
  (pb-broadcast ?peer-id ?msg)

  ; In order to use types from a linked library, they need to be registered via this function first.
  (bind ?res (pb-register-type ?full-name))    ; returns TRUE if successful, FALSE otherwise

Rules
~~~~~

Per default the Plugin ensures that all asserted facts are cleaned up with lowest possible salience.

.. code-block:: lisp

  (defglobal
    ?*PRIORITY-PROTOBUF-RETRACT*    = -10000
  )

  (defrule protobuf-cleanup-receive-failed
    (declare (salience ?*PRIORITY-PROTOBUF-RETRACT*))
    ?f <- (protobuf-receive-failed (comp-id ?cid) (msg-type ?mt)
      (rcvd-from ?host ?port) (message ?msg))
    =>
    (retract ?f)
    (printout t "Protobuf rcv fail for " ?cid ":" ?mt " from " ?host ":" ?port ": " ?msg crlf)
  )

  (defrule protobuf-cleanup-server-receive-failed
    (declare (salience ?*PRIORITY-PROTOBUF-RETRACT*))
    ?f <- (protobuf-server-receive-failed (comp-id ?cid) (msg-type ?mt)
      (rcvd-from ?host ?port) (message ?msg))
    =>
    (retract ?f)
    (printout t "Protobuf server rcv fail for " ?cid ":" ?mt " from " ?host ":" ?port ": " ?msg crlf)
  )

  (defrule protobuf-cleanup-message
    (declare (salience ?*PRIORITY-PROTOBUF-RETRACT*))
    ?pf <- (protobuf-msg (ptr ?p))
    =>
    (pb-destroy ?p)
    (retract ?pf)
  )

Usage Example: Register Message via Configuration
*************************************************

A minimal working example is provided by the :docsite:`cx_bringup` package.

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/protobuf.yaml

It registers a proto file using the configuration options and then creates two UDP broadcast peers communicating via ports ``4445`` and ``4444`` on ``127.0.0.1``.

.. _proto msg example:

Message
~~~~~~~

File :source-master:`cx_bringup/proto/cx_bringup/SearchRequest.proto`.

.. code-block:: proto

  syntax = "proto2";

  message SearchRequest {
    enum CompType {
      COMP_ID = 100;
      MSG_TYPE = 1;
    }
    required string query = 1;
    required int32 page_number = 2;
    required int32 results_per_page = 3;
  }

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/protobuf.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_protobuf"]
      cx_protobuf:
        plugins: ["executive", "protobuf", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"
        publish_on_refresh: false
        assert_time: true
        refresh_rate: 1
      protobuf:
        plugin: "cx::ProtobufPlugin"
        pkg_share_dirs: ["cx_bringup"]
        proto_paths: ["proto/cx_bringup"]
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/protobuf.clp"]



Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/protobuf.clp`.

.. code-block:: lisp

  (defrule protobuf-init-example-peer
    (not (peer ?any-peer-id))
    =>
    (bind ?peer-1 (pb-peer-create-local 127.0.0.1 4444 4445))
    (bind ?peer-2 (pb-peer-create-local 127.0.0.1 4445 4444))
    (assert (peer ?peer-1))
    (assert (peer ?peer-2))
  )

  (defrule peer-send-msg
    (peer ?peer-id)
    (not (protobuf-msg))
    =>
    (bind ?msg (pb-create "SearchRequest"))
    (pb-set-field ?msg "query" "hello")
    (pb-set-field ?msg "page_number" ?peer-id)
    (pb-set-field ?msg "results_per_page" ?peer-id)
    (pb-broadcast ?peer-id ?msg)
    (pb-destroy ?msg)
  )
  (defrule protobuf-msg-read
    (protobuf-msg (type ?type) (comp-id ?comp-id) (msg-type ?msg-type)
      (rcvd-via ?via) (rcvd-from ?address ?port) (rcvd-at ?rcvd-at)
      (client-type ?c-type) (client-id ?c-id) (ptr ?ptr))
    =>
    (printout blue ?c-id "("?c-type") received" ?type
      " (" ?comp-id " " ?msg-type ") from " ?address ":" ?port "
      " (- (now)  ?rcvd-at) "s ago" crlf
    )
    (bind ?var (pb-tostring ?ptr))
    (printout yellow ?var crlf)
  )

  (defrule protobuf-close-peer
    (executive-finalize)
    ?f <- (peer ?any-peer-id)
    =>
    (pb-peer-destroy ?any-peer-id)
    (retract ?f)
  )

Usage Example: Register Message via Linked Plugin
*************************************************

A minimal working example is provided by the :docsite:`cx_bringup` package.

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/protobuf_linked.yaml

It creates a server on port ``4446`` and a client that sends messages (using the same :ref:`message definitions <proto msg example>` as in the first example).

This time the messages are not registered directly through the config, but rather through linking them directly to the loaded plugin.
Therefore, a linked plugin is generated:

.. code-block:: cmake

  cx_generate_linked_protobuf_plugin_from_proto("BringupProtobufPlugin" proto/cx_bringup/SearchRequest.proto)

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/protobuf_linked.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_protobuf"]
      cx_protobuf:
        plugins: ["executive", "protobuf", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"
        publish_on_refresh: false
        assert_time: true
        refresh_rate: 1
      protobuf:
        plugin: "cx::BringupProtobufPlugin"
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/protobuf-linked.clp"]


Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/protobuf-linked.clp`.

.. code-block:: lisp

  (defrule peer-send-msg
    (client ?c-id)
    (protobuf-client-connected ?c-id)
    (not (protobuf-msg))
    =>
    (bind ?msg (pb-create "SearchRequest"))
    (pb-set-field ?msg "query" "hello")
    (pb-set-field ?msg "page_number" ?c-id)
    (pb-set-field ?msg "results_per_page" ?c-id)
    (pb-send ?c-id ?msg)
    (pb-destroy ?msg)
  )

  (defrule protobuf-msg-read
    (protobuf-msg (type ?type) (comp-id ?comp-id) (msg-type ?msg-type)
      (rcvd-via ?via) (rcvd-from ?address ?port) (rcvd-at ?rcvd-at)
      (client-type ?c-type) (client-id ?c-id) (ptr ?ptr))
    =>
    (printout blue ?c-id "("?c-type") received" ?type
      " (" ?comp-id " " ?msg-type ") from " ?address ":" ?port "
      " (- (now)  ?rcvd-at) "s ago" crlf
    )
    (bind ?var (pb-tostring ?ptr))
    (printout yellow ?var crlf)
  )

  (defrule protobuf-close
    (executive-finalize)
    ?f <- (client ?any-client)
    =>
    (pb-disconnect ?any-client)
    (pb-server-disable)
    (retract ?f)
  )
