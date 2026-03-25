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
{% set cpp_type_sep = "::" %}
{% set kebab_type_sep = "-" %}
{% set snake_type_sep = "_" %}
{%- if template_type == "" %}
  {% set cpp_type_sep = "" %}
  {% set kebab_type_sep = "" %}
  {% set snake_type_sep = "" %}
{%- endif %}
{%- if template_part == "definition" %}
clips::UDFValue {{name_camel}}::{{template_type|snake_case}}{{snake_type_sep}}get_field(clips::Environment *env,
                               {{message_type}}{{cpp_type_sep}}{{template_type|camel_case}} *req,
                               const std::string &field, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  std::vector<std::string> slots({ {% for slot in template_slots %}"{{ slot.name }}"{% if not loop.last %}, {% endif %}{% endfor %} });
  if(std::find(slots.begin(), slots.end(), field) == slots.end()) {
    RCLCPP_ERROR(*logger_, "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}get-field: cannot retrieve unknown slot name %s", field.c_str());
    Writeln(env, cx::format("{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}get-field: cannot retrieve unknown slot name {}", field.c_str()).c_str());
    clips::UDFThrowError(udfc);
    return res;
  }
  if(!req) {
    RCLCPP_ERROR(*logger_, "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}get-field: Invalid pointer {{template_type|camel_case}}");
    clips::UDFThrowError(udfc);
    return res;
  }
  res.begin = 0;
  res.range = -1;
  size_t full_length = 0;
  clips::MultifieldBuilder *mb;
{%- for slot in template_slots %}
{%- if not slot.array %}
  if (field == "{{ slot.name }}") {
{%- if slot.clips_type == "STRING" %}
    res.lexemeValue = clips::CreateString(env, std::string(req->{{ slot.name }}).c_str());
{%- endif %}
{%- if slot.clips_type == "INTEGER" %}
    res.integerValue = clips::CreateInteger(env, req->{{ slot.name }});
{%- endif %}
{%- if slot.clips_type == "FLOAT" %}
    res.floatValue = clips::CreateFloat(env, req->{{ slot.name }});
{%- endif %}
{%- if slot.clips_type == "BOOLEAN" %}
    res.lexemeValue = clips::CreateSymbol(env, req->{{ slot.name }} ? "TRUE" : "FALSE");
{%- endif %}
{%- if slot.clips_type == "EXTERNAL-ADDRESS" %}
    res.externalAddressValue = clips::CreateCExternalAddress(env, &(req->{{ slot.name }}));
{%- endif %}
  }
{%- else %}
  if (field == "{{ slot.name }}") {
  full_length = req->{{ slot.name }}.size();
  mb = clips::CreateMultifieldBuilder(env, full_length);
  for (size_t i = 0; i < full_length; i++) {
{%- if slot.clips_type == "STRING" %}
      clips::MBAppendString(mb,std::string(req->{{ slot.name }}[i]).c_str());
{%- endif %}
{%- if slot.clips_type == "INTEGER" %}
      clips::MBAppendInteger(mb,req->{{ slot.name }}[i]);
{%- endif %}
{%- if slot.clips_type == "FLOAT" %}
      clips::MBAppendFloat(mb,req->{{ slot.name }}[i]);
{%- endif %}
{%- if slot.clips_type == "BOOLEAN" %}
      clips::MBAppendSymbol(mb,req->{{ slot.name }}[i] ? "TRUE" : "FALSE");
{%- endif %}
{%- if slot.clips_type == "EXTERNAL-ADDRESS" %}
      clips::MBAppendCLIPSExternalAddress(mb, clips::CreateCExternalAddress(env, &(req->{{ slot.name }}[i])));
{%- endif %}
    }
  res.multifieldValue = clips::MBCreate(mb);
  res.range = -1;
  clips::MBDispose(mb);
  }
{%- endif -%}
{%- endfor %}
  (void) full_length;
  (void) mb;
  return res;
}
{%- endif -%}

{%- if template_part == "registration" %}
  function_names_.insert("{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}get-field");
  clips::AddUDF(
      env.get(), "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}get-field", "*", 2, 2, ";e;sy",
      [](clips::Environment *env, clips::UDFContext *udfc,
         clips::UDFValue *out) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue data_ptr, field;
        using namespace clips;
        clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &data_ptr);
        clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field);
        *out = instance->{{template_type|snake_case}}{{snake_type_sep}}get_field(env, static_cast<{{message_type}}{{cpp_type_sep}}{{template_type|camel_case}}*>(data_ptr.externalAddressValue->contents), field.lexemeValue->contents, udfc);
      },
      "{{template_type|snake_case}}{{snake_type_sep}}get_field", this);
{%- endif -%}

{%- if template_part == "declaration" %}
clips::UDFValue {{template_type|snake_case}}{{snake_type_sep}}get_field(clips::Environment *env,
                          {{message_type}}{{cpp_type_sep}}{{template_type|camel_case}} *data_ptr,
                          const std::string &field, clips::UDFContext *udfc);
{%- endif -%}
