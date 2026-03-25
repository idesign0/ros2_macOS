// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// clang-format off
{%- if template_part == "definition" %}
void {{name_camel}}::{{template_type|snake_case}}_destroy({{message_type}}::{{template_type|camel_case}} *g) {
  std::scoped_lock map_lock{map_mtx_};
  auto it = {{template_type|snake_case}}s_.find(g);
  if (it != {{template_type|snake_case}}s_.end()) {
      {{template_type|snake_case}}s_.erase(it);
  }
}
{%- endif -%}

{%- if template_part == "registration" %}
  function_names_.insert("{{name_kebab}}-{{template_type|kebab_case}}-destroy");
  clips::AddUDF(
    env.get(), "{{name_kebab}}-{{template_type|kebab_case}}-destroy", "v", 1, 1, ";e",
    [](clips::Environment */*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue {{template_type|snake_case}};
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &{{template_type|snake_case}});

      instance->{{template_type|snake_case}}_destroy(static_cast<{{message_type}}::{{template_type|camel_case}}*>({{template_type|snake_case}}.externalAddressValue->contents));
    },
    "{{template_type|snake_case}}_destroy", this);
{%- endif -%}

{%- if template_part == "declaration" %}
  void {{template_type|snake_case}}_destroy({{message_type}}::{{template_type|camel_case}} *{{template_type|snake_case}});
{%- endif -%}
