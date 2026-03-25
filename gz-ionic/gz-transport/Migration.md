# Note on deprecations

A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Transport 11.X to 12.X

### Deprecated

1. The `ignition` namespace is deprecated and will be removed in future versions.  Use `gz` instead.
1. Header files under `ignition/...` are deprecated and will be removed in future versions.
     Use `gz/...` instead.
1. The following `IGN_` prefixed environment variables are deprecated and will be removed in future versions.
     Use the `GZ_` prefixed versions instead!
     1. `IGN_TRANSPORT_USERNAME` -> `GZ_TRANSPORT_USERNAME`

     1. `IGN_TRANSPORT_PASSWORD` -> `GZ_TRANSPORT_PASSWORD`
     1. `IGN_PARTITION` -> `GZ_PARTITION`
     1. `IGN_IP` -> `GZ_IP`
     1. `IGN_TRANSPORT_TOPIC_STATISTICS` -> `GZ_TRANSPORT_TOPIC_STATISTICS`
     1. `IGN_DISCOVERY_MSG_PORT` -> `GZ_DISCOVERY_MSG_PORT`
     1. `IGN_DISCOVERY_MULTICAST_IP` -> `GZ_DISCOVERY_MULTICAST_IP`
     1. `IGN_DISCOVERY_SRV_PORT` -> `GZ_DISCOVERY_SRV_PORT`
     1. `IGN_RELAY` -> `GZ_RELAY`
     1. `IGN_TRANSPORT_LOG_SQL_PATH` -> `GZ_TRANSPORT_LOG_SQL_PATH`
     1. `IGN_TRANSPORT_RCVHWM` -> `GZ_TRANSPORT_RCVHWM`
     1. `IGN_TRANSPORT_SNDHWM` -> `GZ_TRANSPORT_SNDHWM`
     1. `IGN_VERBOSE` -> `GZ_VERBOSE`
1. The following `IGN_` prefixed macros are deprecated and will be removed in future versions.
   Additionally, they will only be available when including the corresponding `ignition/...` header.
   Use the `GZ_` prefix instead.
     1. `IGN_ZMQ_POST_4_3_1`
     1. `IGN_CPPZMQ_POST_4_7_0`
     1. `ign_strcat`, `ign_strcpy`, `ign_sprintf`, `ign_strdup`
1. The `IgnTransportNode` class is deprecated and will be removed in future versions. Use `GzTransportNode` instead.
     Similarly, the `Ign` prefixed members of that class will be removed in future versions. Use the `Gz` prefixed members instead.

### Breaking Changes

1. The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name

## Gazebo Transport 9.X to 10.X

### Addition

1. Dependency on `cli` component of `gz-utils`.
    * [GitHub pull request 229](https://github.com/gazebosim/gz-transport/pull/229)

## Gazebo Transport 8.X to 9.X

### Removed

1. Remove deprecations before 9.x.x release
    * [GitHub pull request 175](https://github.com/gazebosim/gz-transport/pull/175)

## Gazebo Transport 7.X to 8.X

### Deprecated

1. NodeShared::TriggerSubscriberCallbacks
    * [BitBucket pull request 404](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/404)

1. The discovery wire protocol changed to use gz::msgs::Discovery
   instead of C-structs. The Packet.hh header file is deprecated, which
   contained the Header, SubscriptionMsg, and AdvertiseMessage classes. The
   version of the wire protocol has bumped from 9 to 10. This means Gazebo
   Transport 8+ will not work with Gazebo Transport 7 and below.
    * [BitBucket pull request 403](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/403)

## Gazebo Transport 6.X to 7.X

### Removed

1. The `gz.hh` file is not longer installed.
    * [BitBucket pull request 367](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/367)

## Gazebo Transport 4.X to 5.X

## Gazebo Transport 3.X to 4.X

1. Service responder callbacks passed to `Node::Advertise` should now return
   a boolean value instead of taking in a boolean output parameter. The existing
   functions have been deprecated.
    * [BitBucket pull request 260](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/260)
    * [BitBucket pull request 228](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/228)
