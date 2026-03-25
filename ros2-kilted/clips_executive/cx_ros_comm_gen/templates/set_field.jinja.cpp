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
void {{name_camel}}::{{template_type|snake_case}}{{snake_type_sep}}set_field({{message_type}}{{cpp_type_sep}}{{template_type}} *req,
                                       const std::string &field,
                                       clips::UDFValue value, clips::UDFContext *udfc) {
  (void) value;
  std::scoped_lock map_lock{map_mtx_};
  std::vector<std::string> slots({ {% for slot in template_slots %}"{{ slot.name }}"{% if not loop.last %}, {% endif %}{% endfor %} });
  if(std::find(slots.begin(), slots.end(), field) == slots.end()) {
    RCLCPP_ERROR(*logger_, "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: cannot retrieve unknown slot name %s", field.c_str());
    clips::UDFThrowError(udfc);
    return;
  }
  if(!req) {
    RCLCPP_ERROR(*logger_, "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: Invalid pointer {{template_type|camel_case}}");
    clips::UDFThrowError(udfc);
    return;
  }

  clips::Multifield *multi;
{%- for slot in template_slots %}
{%- if not slot.array %}
  if (field == "{{ slot.name }}") {
{%- if slot.clips_type == "STRING" %}
    req->{{ slot.name }} = value.lexemeValue->contents;
{%- endif %}
{%- if slot.clips_type == "INTEGER" %}
    req->{{ slot.name }} = value.integerValue->contents;
{%- endif %}
{%- if slot.clips_type == "FLOAT" %}
    req->{{ slot.name }} = value.floatValue->contents;
{%- endif %}
{%- if slot.clips_type == "BOOLEAN" %}
    req->{{ slot.name }} = std::string(value.lexemeValue->contents).compare(std::string("TRUE")) ? false : true;
{%- endif %}
{%- if slot.clips_type == "EXTERNAL-ADDRESS" %}
    req->{{ slot.name }} = *static_cast<{{slot.type}} *>(value.externalAddressValue->contents);
{%- endif %}
  }
{%- else %}
  if (field == "{{ slot.name }}") {
    multi = value.multifieldValue;
  {%- if slot.clips_type == "STRING" %}
    std::vector<{{slot.cpp_type}}> value_vector_{{slot.name}};
    for (size_t i = 0; i < multi->length; i++) {
      switch (multi->contents[i].header->type) {
      case STRING_TYPE:
      case SYMBOL_TYPE:
        value_vector_{{slot.name}}.push_back(multi->contents[i].lexemeValue->contents);
        break;
      default:
        RCLCPP_ERROR(*logger_,
                     "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: Unexpected Type %i (expected STRING/SYMBOL) of %li nth argument of UDF",
                     multi->contents[i].header->type, i);
        clips::UDFThrowError(udfc);
      }
    }

    req->{{ slot.name }} = value_vector_{{slot.name}};
  {%- endif %}
  {%- if slot.clips_type == "INTEGER" %}
    std::vector<{{slot.cpp_type}}> value_vector_{{slot.name}};
    for (size_t i = 0; i < multi->length; i++) {
      switch (multi->contents[i].header->type) {
      case INTEGER_TYPE:
        value_vector_{{slot.name}}.push_back(multi->contents[i].integerValue->contents);
        break;
      default:
        RCLCPP_ERROR(*logger_,
                     "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: Unexpected Type %i (expected INTEGER) of %li nth argument of UDF",
                     multi->contents[i].header->type, i);
        clips::UDFThrowError(udfc);
      }
    }
    req->{{ slot.name }} = value_vector_{{slot.name}};
  {%- endif %}
  {%- if slot.clips_type == "FLOAT" %}
    std::vector<{{slot.cpp_type}}> value_vector_{{slot.name}};
    for (size_t i = 0; i < multi->length; i++) {
      switch (multi->contents[i].header->type) {
      case FLOAT_TYPE:
        value_vector_{{slot.name}}.push_back(multi->contents[i].floatType->contents);
        break;
      default:
        RCLCPP_ERROR(*logger_,
                     "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: Unexpected Type %i (expected FLOAT) of %li nth argument of UDF",
                     multi->contents[i].header->type, i);
        clips::UDFThrowError(udfc);
      }
    }
    req->{{ slot.name }} = value_vector_{{slot.name}};
  {%- endif %}
  {%- if slot.clips_type == "EXTERNAL-ADDRESS" %}
    std::vector<{{slot.type}}> value_vector_{{slot.name}};
    for (size_t i = 0; i < multi->length; i++) {
      switch (multi->contents[i].header->type) {
      case EXTERNAL_ADDRESS_TYPE:
        value_vector_{{slot.name}}.push_back(*static_cast<{{slot.type}}*>((multi->contents[i].externalAddressValue->contents)));
        break;
      default:
        RCLCPP_ERROR(*logger_,
                     "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field: Unexpected Type %i (expected EXTERNAL-ADDRESS) of %li nth argument of UDF",
                     multi->contents[i].header->type, i);
        clips::UDFThrowError(udfc);
      }
    }
    req->{{ slot.name }} = value_vector_{{slot.name}};
  {%- endif %}
  }
{%- endif -%}
{%- endfor %}
  (void)multi;
  (void)udfc;
}
{%- endif -%}
{%- if template_part == "registration" %}
  function_names_.insert("{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field");
  clips::AddUDF(
      env.get(), "{{name_kebab}}-{{template_type|kebab_case}}{{kebab_type_sep}}set-field", "v", 3, 3, ";e;sy;*",
      [](clips::Environment */*env*/, clips::UDFContext *udfc,
         clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue data_ptr, field, value;
        using namespace clips;
        clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &data_ptr);
        clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field);
        clips::UDFNthArgument(udfc, 3, ANY_TYPE_BITS, &value);

        instance->{{template_type|snake_case}}{{snake_type_sep}}set_field(static_cast<{{message_type}}{{cpp_type_sep}}{{template_type|camel_case}}*>(data_ptr.externalAddressValue->contents), field.lexemeValue->contents, value, udfc);
      },
      "{{template_type|snake_case}}{{snake_type_sep}}set_field", this);

{%- endif -%}
{%- if template_part == "declaration" %}
  void {{template_type|snake_case}}{{snake_type_sep}}set_field({{message_type}}{{cpp_type_sep}}{{template_type|camel_case}} *msg, const std::string &field, clips::UDFValue value, clips::UDFContext *udfc);
{%- endif -%}
