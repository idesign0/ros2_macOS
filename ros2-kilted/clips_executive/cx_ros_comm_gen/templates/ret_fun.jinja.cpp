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
clips::UDFValue {{name_camel}}::{{template_type|snake_case}}_{{template_call_fun|snake_case}}(clips::Environment *env, void *{{template_type|snake_case}}_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto {{template_type|snake_case}} = {{template_type|snake_case}}s_.at({{template_type|snake_case}}_raw);
  clips::UDFValue res;
  if(!{{template_type|snake_case}}) {
    RCLCPP_ERROR(*logger_, "{{template_type|snake_case}}_{{template_call_fun|snake_case}}: Invalid pointer to {{template_type|camel_case}}");
    res.lexemeValue = clips::CreateSymbol(env, "FALSE");
    clips::UDFThrowError(udfc);
    return res;
  }
{%- if template_ret_type == "Bool" %}
{% set ret_type_bit = "b" %}
  res.lexemeValue = clips::CreateSymbol(env, {{template_type|snake_case}}->{{template_call_fun|snake_case}}() ? "TRUE" : "FALSE");
{%- elif template_ret_type == "Integer" %}
{% set ret_type_bit = "l" %}
  res.integerValue = clips::CreateInteger(env, {{template_type|snake_case}}->{{template_call_fun|snake_case}}());
{%- elif template_ret_type == "Float" %}
{% set ret_type_bit = "d" %}
  res.floatValue = clips::Float(env, {{template_type|snake_case}}->{{template_call_fun|snake_case}}());
{%- elif template_ret_type == "String" %}
{% set ret_type_bit = "s" %}
  res.lexemeValue = clips::String(env, {{template_type|snake_case}}->{{template_call_fun|snake_case}}().c_str());
{%- endif -%}

  return res;
}
{%- endif -%}

{%- if template_part == "registration" %}
  function_names_.insert("{{name_kebab}}-{{template_type|kebab_case}}-{{template_call_fun|kebab_case}}");
  clips::AddUDF(
    env.get(),"{{name_kebab}}-{{template_type|kebab_case}}-{{template_call_fun|kebab_case}}", "{{ret_type_bit}}", 1, 1, ";e",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue {{template_type|snake_case}};
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &{{template_type|snake_case}});
       try {
        *out = instance->{{template_type|snake_case}}_{{template_call_fun|snake_case}}(env, {{template_type|snake_case}}.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown {{template_type|snake_case}} %s", e.what());
       }
    },
    "{{template_type|snake_case}}_{{template_call_fun|snake_case}}", this);
{%- endif -%}

{%- if template_part == "declaration" %}
  clips::UDFValue {{template_type|snake_case}}_{{template_call_fun|snake_case}}(clips::Environment *env, void *raw, clips::UDFContext *udfc);
{%- endif -%}
