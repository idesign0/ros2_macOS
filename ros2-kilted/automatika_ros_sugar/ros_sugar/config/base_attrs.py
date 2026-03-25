import json
from typing import (
    Any,
    Callable,
    Dict,
    Optional,
    Union,
    get_args,
    List,
    get_origin,
    Literal,
    _GenericAlias,
)
import functools
from copy import deepcopy
import numpy as np
from attrs import asdict, define, fields_dict
from attrs import Attribute
import yaml
import toml
import os
import enum
from . import base_validators

# mapping base_validator function names to ui friendly operation names
FUNC_NAME_MAP = {
    "__gt": "greater_than",
    "__lt": "less_than",
    "__in_": "in",
    "__list_contained_in": "list_contained_in",
    "__in_range_validator": "in_range",
    "__in_range_discretized_validator": "in_range_discretized",
}


def skip_no_init(a: Attribute, _) -> bool:
    return a.init


@define
class BaseAttrs:
    """
    Implements setattr method to re-use validators at set time
    """

    def __setattr__(self, name: str, value: Any) -> None:
        """Call the validator when we set the field (by default it only runs on __init__)"""
        for attribute in [
            a for a in getattr(self.__class__, "__attrs_attrs__", []) if a.name == name
        ]:
            if attribute.validator is not None:
                attribute.validator(self, attribute, value)
        super().__setattr__(name, value)

    def __str__(self) -> str:
        """
        Pretty print of class attributes/values

        :return: _description_
        :rtype: str
        """
        print_statement = "{\n"

        first_level_keys = [attr.name for attr in self.__attrs_attrs__]
        first_level_values = [getattr(self, key) for key in first_level_keys]
        if first_level_keys.count("additional_types"):
            first_level_keys.remove("additional_types")
        for name, value in zip(first_level_keys, first_level_values):
            # Do not display private attributes
            if not name.startswith("_"):
                print_statement += f"{name}: {value}, \n"

        return print_statement.rstrip(", \n") + "\n}"

    @classmethod
    def __is_subscripted_generic(cls, some_type) -> Optional[type]:
        """
        Helper method to check if a type is from typing.Union

        :param some_type: Some type to check
        :type some_type: type

        :return: If type is from typing.Union
        :rtype: bool
        """
        return getattr(some_type, "__origin__", None)

    @classmethod
    def __get_subscribed_generic_simple_types(cls, sg_type) -> List:
        _types = get_args(sg_type)
        _parsed_types = []
        for m_type in _types:
            if nested_generic := cls.__is_subscripted_generic(m_type):
                if nested_generic is Literal:
                    _parsed_types.append(m_type)
                else:
                    _parsed_types.extend(
                        cls.__get_subscribed_generic_simple_types(m_type)
                    )
            else:
                _parsed_types.append(m_type)
        return _parsed_types

    def asdict(self, filter: Optional[Callable] = None) -> Dict:
        """Convert class to dict.
        :rtype: dict
        """
        return asdict(self, filter=filter)

    def to_dict(self) -> Dict:
        """Convert class to dict.
        :rtype: dict
        """
        return self.asdict()

    def __check_value_against_attr_type(
        self, key: str, value: Any, attribute_to_set: Any, attribute_type: type
    ):
        """Helper method to check if a given value can be parsed to a class attribute

        :param key: Attribute key name
        :type key: str
        :param value: New value to set
        :type value: Any
        :param attribute_to_set: Class attribute to set
        :type attribute_to_set: Any
        :param attribute_type: Class attribute type
        :type attribute_type: type

        :raises TypeError: If value is of incompatible type

        :return: Updated value
        :rtype: Any
        """
        # Union typing requires special treatment
        if generic_type := self.__is_subscripted_generic(attribute_type):
            _types = self.__get_subscribed_generic_simple_types(attribute_type)
            if generic_type is Union:
                # Check if the value type is one of the valid union types
                if not any(isinstance(value, t) for t in _types):
                    raise TypeError(
                        f"Trying to set with incompatible type. Attribute {key} expecting '{type(attribute_to_set)}' got '{type(value)}'"
                    )
            if generic_type is Literal:
                if not any(value == t for t in _types):
                    raise TypeError(
                        f"Trying to set a Literal attribute with incompatible value. Attribute {key} expecting '{_types}' got '{value}'"
                    )
        elif isinstance(value, List) and attribute_type is np.ndarray:
            # Turn list into numpy array
            value = np.array(value)

        # else:
        #     # If not a Union type -> check using isinstance
        #     # Handles only the origin of GenericAlias (dict, list)
        #     _attribute_type = (
        #         get_origin(attribute_type)
        #         if isinstance(attribute_type, _GenericAlias)
        #         else attribute_type
        #     )
        #     if not isinstance(value, _attribute_type):
        #         raise TypeError(
        #             f"Trying to set with incompatible type. Attribute {key} expecting '{type(attribute_to_set)}' got '{type(value)}'"
        #         )
        return value

    def __parse_from_serialized_list(self, list_attr: List, value: List) -> List:
        """Helper method to parse attribute value from a list of serialized values

        :param list_attr: _description_
        :type list_attr: list
        :param value: _description_
        :type value: list
        :raises TypeError: _description_
        :return: _description_
        :rtype: list
        """
        new_list = []
        attr_val = list_attr[0]
        for val in value:
            if hasattr(attr_val, "__attrs_attrs__"):
                if not isinstance(val, Dict):
                    raise TypeError(
                        f"Trying to set with incompatible type. Attribute expecting list of dictionaries got list of '{type(val)}'"
                    )
                attr_val.from_dict(val)
            new_list.append(deepcopy(attr_val))
        return new_list

    def from_dict(self, dict_obj: Dict) -> None:
        """
        Gets attributes values from given dictionary

        :param dict_obj: Dictionary {attribute_name: attribute_value}
        :type dict_obj: Dict

        :raises ValueError: If attribute_name in dictionary does not exists in class attributes
        :raises TypeError: If attribute_value type in dictionary does not correspond to class attribute type
        """
        for key, value in dict_obj.items():
            if key not in self.asdict().keys():
                continue
            attribute_to_set = getattr(self, key)
            attribute_type = fields_dict(self.__class__)[key].type
            # Check for nested classes
            if hasattr(attribute_to_set, "__attrs_attrs__"):
                if not isinstance(value, Dict):
                    raise TypeError(
                        f"Trying to set with incompatible type. Attribute {key} expecting dictionary got '{type(value)}'"
                    )
                attribute_to_set.from_dict(value)
            elif isinstance(attribute_to_set, List) and attribute_to_set:
                setattr(
                    self,
                    key,
                    self.__parse_from_serialized_list(attribute_to_set, value),
                )
            else:
                # Handle Any typing as it cannot be checked with isinstance
                if attribute_type is Any:
                    continue
                elif attribute_type:
                    value = self.__check_value_against_attr_type(
                        key, value, attribute_to_set, attribute_type
                    )
                setattr(self, key, value)

    def _select_nested_config(
        self, config: Dict[str, Any], key_path: Optional[str]
    ) -> Dict[str, Any]:
        if not key_path:
            return config
        keys = key_path.split(".")
        for key in keys:
            config = config.get(key, {})
        return config

    def from_file(
        self,
        file_path: str,
        nested_root_name: Union[str, None] = None,
        get_common: bool = False,
    ) -> bool:
        """
        Update class attributes from yaml, json, or toml

        :param file_path: Path to config file (.yaml, .json, .toml)
        :param nested_root_name: Nested root name for the config, defaults to None
        :param get_common: Whether to get extra config root (for merging), defaults to False
        """
        ext: str = os.path.splitext(file_path)[1].lower()

        with open(file_path, "r", encoding="utf-8") as f:
            if ext in [".yaml", ".yml"]:
                raw_config: Dict[str, Any] = yaml.safe_load(f)
            elif ext == ".json":
                raw_config = json.load(f)
            elif ext == ".toml":
                raw_config = toml.load(f)
            else:
                raise ValueError(f"Unsupported config format: {ext}")

        # Extract specific and common config sections
        config = self._select_nested_config(raw_config, nested_root_name)
        extra_config = (
            self._select_nested_config(raw_config, "/**") if get_common else {}
        )

        if not config and not extra_config:
            return False

        merged_config = {**extra_config, **config}
        # Set attributes from final merged config
        self.from_dict(merged_config)
        return True

    def to_json(self) -> Union[str, bytes, bytearray]:
        """
        Dump to json

        :return: _description_
        :rtype: str | bytes | bytearray
        """
        dictionary = self.asdict(filter=skip_no_init)
        serialized_dict = self.__dict_to_serialized_dict(dictionary)
        return json.dumps(serialized_dict)

    def __list_to_serialized_list(self, list_items: List) -> List:
        """Serialize List object items

        :param list_items: _description_
        :type list_items: list
        :return: _description_
        :rtype: list
        """
        serialized_list = []
        for item in list_items:
            # Convert numpy array to list
            if isinstance(item, np.ndarray):
                item = item.tolist()
            if isinstance(item, BaseAttrs):
                serialized_list.append(item.to_json())
            elif isinstance(item, List):
                serialized_list.append(self.__list_to_serialized_list(item))
            elif isinstance(item, tuple):
                serialized_list.append(
                    tuple(self.__list_to_serialized_list(list(item)))
                )
            elif isinstance(item, Dict):
                serialized_list.append(self.__dict_to_serialized_dict(item))
            elif type(item) not in [float, int, str, bool, type(None)]:
                serialized_list.append(str(item))
            else:
                serialized_list.append(item)
        return serialized_list

    def __dict_to_serialized_dict(self, dictionary):
        """Serialize Dictionary object items

        :param dictionary: _description_
        :type dictionary: _type_
        :return: _description_
        :rtype: _type_
        """
        for name, value in dictionary.items():
            # Convert numpy array to list
            if isinstance(value, np.ndarray):
                value = value.tolist()

            if isinstance(value, List):
                dictionary[name] = self.__list_to_serialized_list(value)
            elif isinstance(value, tuple):
                dictionary[name] = tuple(self.__list_to_serialized_list(list(value)))
            elif isinstance(value, Dict):
                dictionary[name] = self.__dict_to_serialized_dict(value)
            elif type(value) not in [float, int, str, bool, type(None)]:
                dictionary[name] = str(value)
        return dictionary

    def from_json(self, json_obj: Union[str, bytes, bytearray]) -> None:
        """
        Gets attributes values from given json

        :param json_obj: Json object
        :type json_obj: str | bytes | bytearray
        """
        dict_obj = json.loads(json_obj)
        self.from_dict(dict_obj)

    def has_attribute(self, attr_name: str) -> bool:
        """
        Checks if class object has attribute with given name

        :param attr_name: _description_
        :type attr_name: str

        :return: If object has attribute with given name
        :rtype: bool
        """
        # Get nested attributes if there
        nested_names = attr_name.split(".")
        obj_to_set = self
        for name in nested_names:
            # Raise an error if the name does not exist in the class
            if not hasattr(obj_to_set, name):
                return False
            obj_to_set = getattr(obj_to_set, name)
        return True

    def get_attribute_type(self, attr_name: str) -> Optional[type]:
        """
        Gets type of given attribute name

        :param attr_name: _description_
        :type attr_name: str

        :raises AttributeError: If class does not have attribute with given name

        :return: Attribute type
        :rtype: type
        """
        # Get nested attributes if there
        nested_names = attr_name.split(".")
        name_to_set = nested_names[0]
        obj_to_set = self
        obj_class = self
        for name_to_set in nested_names:
            # Raise an error if the name does not exist in the class
            if not hasattr(obj_to_set, name_to_set):
                raise AttributeError(
                    f"Class '{self.__class__.__name__}' does not have an attribute '{attr_name}'"
                )
            obj_class = obj_to_set
            obj_to_set = getattr(obj_to_set, name_to_set)

        complex_type = fields_dict(obj_class.__class__)[name_to_set].type

        # Extract the simple type
        SIMPLE_TYPES = {int, float, str, bool}
        origin = get_origin(complex_type)
        args = self.__get_subscribed_generic_simple_types(complex_type)

        # If it's directly a simple type
        if complex_type in SIMPLE_TYPES:
            return complex_type

        # If it's a Union (including Optional)
        if origin is Union:
            for arg in args:
                if arg in SIMPLE_TYPES:
                    return arg
        if origin is Literal:
            literal_types = [type(arg) for arg in args]
            for arg in literal_types:
                if arg in SIMPLE_TYPES:
                    return arg
        return None  # No simple type found

    def update_value(self, attr_name: str, attr_value: Any) -> bool:
        """
        Updates the value of an attribute in the class

        :param attr_name: Attribute name - can be nested name
        :type attr_name: str
        :param attr_value: Attribute value
        :type attr_value: Any

        :raises AttributeError: If class does not contain attribute with given name
        :raises TypeError: If class attribute with given name if of different type

        :return: If attribute value is updated
        :rtype: bool
        """
        # Get nested attributes if there
        nested_names = attr_name.split(".")
        name_to_set = nested_names[0]
        obj_to_set = self
        obj_class = self
        for name_to_set in nested_names:
            # Raise an error if the name does not exist in the class
            if not hasattr(obj_to_set, name_to_set):
                raise AttributeError(
                    f"Class '{self.__class__.__name__}' does not have an attribute '{attr_name}'"
                )
            obj_class = obj_to_set
            obj_to_set = getattr(obj_to_set, name_to_set)

        attribute_type = self.get_attribute_type(attr_name)

        if not attribute_type:
            raise TypeError(
                f"Class '{self.__class__.__name__}' attribute '{attr_name}' type unknown"
            )

        if not isinstance(attr_value, attribute_type):
            raise TypeError(
                f"Class '{self.__class__.__name__}' attribute '{attr_name}' expecting type '{attribute_type}', got {type(attr_value)}"
            )
        setattr(obj_class, name_to_set, attr_value)
        return True

    @staticmethod
    def _parse_validator(validator: object) -> Dict[str, Optional[Any]]:
        """
        Introspects an attrs validator object to extract its parameters.

        :param validator: The validator object to parse.
        :return: A dictionary with the validator's name and parameters.
        """
        validator_name = validator.__class__.__name__

        # Handle attrs built-in range validators (gt, ge, lt, le)
        if hasattr(validator, "bound"):
            op_name = {
                "_GreaterThenValidator": "greater_than",
                "_GreaterEqualValidator": "greater_or_equal",
                "_LessThenValidator": "less_than",
                "_LessEqualValidator": "less_or_equal",
            }.get(validator_name, "bound")
            return {op_name: validator.bound}

        # Handle functions of base_validators
        if isinstance(validator, functools.partial):
            func = validator.func
            func_name = func.__name__
            # ensure function is from base_validator
            if hasattr(base_validators, func_name):
                # map name to standard name
                op_name = FUNC_NAME_MAP.get(func_name, func_name)
                # collect bounds/parameters
                bounds = {}
                # keyword args in partial
                bounds.update(validator.keywords or {})
                return {op_name: bounds}

        # Fallback for unknown validators
        return {"unknown": "unknown"}

    @classmethod
    def get_fields_info(cls, class_object) -> Dict[str, Dict[str, Any]]:
        """
        Returns a dictionary with metadata about each field in the class.

        This includes the field's name, type annotation, and parsed validator info.

        :return: A dictionary where keys are field names and values are dicts
                 of metadata.
        """
        fields_info = {}
        # Iterate over all attributes defined by attrs
        for attr_field in class_object.__attrs_attrs__:
            if attr_field.name.startswith("_"):
                continue
            validators_list = []
            # Check if a validator exists for the field
            if attr_field.validator:
                # A validator can be a single callable or a list/tuple of them
                # if they are wrapped in attrs.validators.and_()
                # Check for composite validators (like and_())
                if hasattr(attr_field.validator, "validators"):
                    for v in attr_field.validator.validators:
                        validators_list.append(cls._parse_validator(v))

                else:  # It's a single validator
                    validators_list.append(cls._parse_validator(attr_field.validator))
            parsed_type = attr_field.type
            # Check and handle Union/Optional/Literal types:
            if generic_type := cls.__is_subscripted_generic(parsed_type):
                args = cls.__get_subscribed_generic_simple_types(parsed_type)
                # Do nothing if simple generic like Dict, List
                if not args:
                    pass
                # Execlude simple optional types
                elif generic_type is Union and type(None) in args:
                    # Optional argument
                    args.remove(type(None))
                    if len(args) > 0:
                        parsed_type = f"Optional[{args[0]}]"
                    else:
                        # skip unknown
                        continue
                elif generic_type is Literal:
                    parsed_type = f"Literal{list(args)}"
                else:
                    parsed_type = []
                    # Parse Enum to Literal
                    parsed_enum = None
                    for val_type in args:
                        if val_type is Literal:
                            parsed_type.append(f"Literal{get_args(val_type)}")
                        elif issubclass(val_type, enum.Enum):
                            values = [member.name for member in val_type]
                            parsed_enum = f"Literal{values}"
                            validators_list = []
                        else:
                            parsed_type.append(val_type)
                    # If an enum is parsed pass only the literal type
                    parsed_type = parsed_enum or parsed_type

            if type(attr_field.type) is type and (
                issubclass(parsed_type, BaseAttrs)
                or parsed_type.__base__.__name__ == "BaseAttrs"
            ):
                val: BaseAttrs = getattr(class_object, attr_field.name)
                fields_info[attr_field.name] = {
                    "type": "BaseAttrs",
                    "validators": [],
                    "value": cls.get_fields_info(val),
                }
            else:
                fields_info[attr_field.name] = {
                    "type": parsed_type,
                    "validators": validators_list,
                    "value": getattr(class_object, attr_field.name),
                }
        return fields_info
