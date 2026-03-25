---
title: EmbodiedAgents Developer Documentation
---

# EmbodiedAgents Developer Docs

EmbodiedAgents is the intelligence layer of the [EMOS](https://github.com/automatika-robotics/emos) ecosystem. It provides AI component abstractions, model client integrations, and semantic memory for building physical AI agents on ROS 2.

This site contains **developer documentation** for contributors extending the framework with new components, clients, models, or message types.

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

Component hierarchy, the `_execution_step()` pattern, input/output validation, trigger system, and the model/client/component layering.
:::

::::

## Extend & Customize

::::{grid} 1 2 2 3
:gutter: 3

:::{grid-item-card} {material-regular}`widgets;1.5em;sd-text-primary` Custom Components
:link: development/custom_component
:link-type: doc
:class-card: sugar-card

Build new AI components with managed I/O, trigger wiring, and model client integration.
:::

:::{grid-item-card} {material-regular}`dns;1.5em;sd-text-primary` Custom Model Clients
:link: development/custom_client
:link-type: doc
:class-card: sugar-card

Integrate new inference backends by implementing the `ModelClient` or `DBClient` contracts.
:::

:::{grid-item-card} {material-regular}`hub;1.5em;sd-text-primary` Custom Models
:link: development/custom_model
:link-type: doc
:class-card: sugar-card

Wrap new model serving platforms with attrs-based model specs and initialization parameters.
:::

:::{grid-item-card} {material-regular}`cable;1.5em;sd-text-primary` Custom Message Types
:link: development/messages
:link-type: doc
:class-card: sugar-card

Define new ROS messages, callbacks, and `SupportedType` wrappers for the type system.
:::

:::{grid-item-card} {material-regular}`layers;1.5em;sd-text-primary` Adding a New Modality
:link: development/adding_modality
:link-type: doc
:class-card: sugar-card

End-to-end guide: wire a new data modality from ROS message through callback, type wrapper, config, and component.
:::

:::{grid-item-card} {material-regular}`tune;1.5em;sd-text-primary` Advanced Components
:link: development/advanced_component
:link-type: doc
:class-card: sugar-card

Health status, fallback recovery, model client hot-swapping, local model fallbacks, and the event/action system.
:::

::::

---

```{toctree}
:maxdepth: 2
:caption: Developer Guide
:hidden:

development/architecture
development/custom_component
development/custom_client
development/custom_model
development/messages
development/adding_modality
development/advanced_component
```

```{toctree}
:maxdepth: 2
:caption: API Reference
:hidden:

apidocs/index
```
