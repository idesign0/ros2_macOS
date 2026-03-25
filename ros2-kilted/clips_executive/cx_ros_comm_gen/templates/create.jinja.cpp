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
clips::UDFValue {{name_camel}}::{{template_type|snake_case}}_create(clips::Environment *env) {
  std::shared_ptr<{{message_type}}::{{template_type|camel_case}}> ptr = std::make_shared<{{message_type}}::{{template_type|camel_case}}>();
  std::scoped_lock map_lock{map_mtx_};
  {{template_type|snake_case}}s_[ptr.get()] = ptr;
  clips::UDFValue res;
  res.externalAddressValue = clips::CreateCExternalAddress(env, ptr.get());
  return res;
}
{%- endif -%}

{%- if template_part == "registration" %}
  function_names_.insert("{{name_kebab}}-{{template_type|kebab_case}}-create");
  clips::AddUDF(
      env.get(), "{{name_kebab}}-{{template_type|kebab_case}}-create", "e", 0, 0, "",
      [](clips::Environment *env, clips::UDFContext *udfc,
         clips::UDFValue *out) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        using namespace clips;
        *out = instance->{{template_type|snake_case}}_create(env);
      },
      "{{template_type|snake_case}}_create", this);
{%- endif -%}

{%- if template_part == "declaration" %}
  clips::UDFValue {{template_type|snake_case}}_create(clips::Environment *env);
{%- endif -%}
