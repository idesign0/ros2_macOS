---
title: Sugarcoat Developer Documentation
---

# Sugarcoat Developer Docs

Sugarcoat is the orchestration layer of the [EMOS](https://github.com/automatika-robotics/emos) ecosystem. It provides the foundational abstractions (components, events, actions, and the launcher) that power [EmbodiedAgents](https://github.com/automatika-robotics/embodied-agents) and [Kompass](https://github.com/automatika-robotics/kompass).

This site contains **developer documentation** for contributors and package authors building on top of Sugarcoat.

:::{admonition} Looking for usage documentation?
:class: tip

Tutorials, installation guides, and usage documentation are on the
**[EMOS Documentation](https://emos.automatikarobotics.com)** site.
:::

---

## Understand the Framework

::::{grid} 1 2 2 2
:gutter: 3

:::{grid-item-card} {material-regular}`account_tree;1.5em;sd-text-primary` Architecture
:link: development/architecture
:link-type: doc
:class-card: sugar-card

Core module structure, component lifecycle, Monitor orchestration, Launcher process graph, and I/O system.
:::

:::{grid-item-card} {material-regular}`bolt;1.5em;sd-text-primary` Event System
:link: development/event_system
:link-type: doc
:class-card: sugar-card

Condition trees, event patterns, action dispatch, fallback hierarchy, and Monitor evaluation loop.
:::

::::

## Extend & Integrate

::::{grid} 1 2 2 3
:gutter: 3

:::{grid-item-card} {material-regular}`widgets;1.5em;sd-text-primary` Custom Components
:link: development/custom_component
:link-type: doc
:class-card: sugar-card

Subclass `BaseComponent` with lifecycle hooks, run types, I/O validation, and custom actions.
:::

:::{grid-item-card} {material-regular}`cable;1.5em;sd-text-primary` Custom Types
:link: development/custom_types
:link-type: doc
:class-card: sugar-card

Add new `SupportedType` wrappers, callbacks, and type registration for custom ROS messages.
:::

:::{grid-item-card} {material-regular}`sync_alt;1.5em;sd-text-primary` Processing Pipelines
:link: development/custom_processing
:link-type: doc
:class-card: sugar-card

Inject post-processors on callbacks and pre-processors on publishers for data transformation.
:::

:::{grid-item-card} {material-regular}`smart_toy;1.5em;sd-text-primary` Robot Plugins
:link: development/custom_robot_plugin
:link-type: doc
:class-card: sugar-card

Map generic topics to robot-specific interfaces with custom types and service clients.
:::

:::{grid-item-card} {material-regular}`dashboard;1.5em;sd-text-primary` UI Elements
:link: development/custom_ui_elements
:link-type: doc
:class-card: sugar-card

Register custom input forms and output visualizations in the web UI for new data types.
:::

:::{grid-item-card} {material-regular}`science;1.5em;sd-text-primary` Testing
:link: development/testing
:link-type: doc
:class-card: sugar-card

Unit and integration testing for components, events, actions, and fallbacks.
:::

::::

## Reference

::::{grid} 1 2 2 2
:gutter: 3

:::{grid-item-card} {material-regular}`miscellaneous_services;1.5em;sd-text-primary` Built-in Services
:link: advanced/srvs
:link-type: doc
:class-card: sugar-card

Live reconfiguration services: topic replacement, parameter updates, and file-based configuration.
:::

:::{grid-item-card} {material-regular}`rocket_launch;1.5em;sd-text-primary` Systemd Deployment
:link: advanced/create_service
:link-type: doc
:class-card: sugar-card

Convert Sugarcoat recipes into systemd services for production deployment.
:::

::::

---

```{toctree}
:maxdepth: 2
:caption: Developer Guide
:hidden:

development/architecture
development/custom_component
development/custom_types
development/custom_processing
development/custom_robot_plugin
development/custom_ui_elements
development/event_system
development/testing
```

```{toctree}
:maxdepth: 1
:caption: Reference
:hidden:

advanced/create_service
advanced/srvs
```

```{toctree}
:maxdepth: 2
:caption: API Reference
:hidden:

apidocs/index
```
