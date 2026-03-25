from typing import Any, List, Callable, Union, Type, Dict, Optional, Sequence, Tuple
import operator
import array
import copy
import numpy as np
import json

from enum import Enum


def _access_attribute(obj: Any, nested_attributes: Sequence[Union[str, int]]) -> Any:
    """Access nested attribute (specified by attrs) in a given object, int are allowed to access specific indices in array attributes

    :param obj: _description_
    :type obj: Any
    :param nested_attributes: _description_
    :type nested_attributes: List[Union[str, int]]
    :raises AttributeError: If nested attribute does not exist in object
    :return: result
    :rtype: Any
    """
    result = obj
    for attr in nested_attributes:
        if isinstance(attr, int):
            # Handle list index
            try:
                result = result[attr]
            except (IndexError, TypeError) as e:
                raise AttributeError(
                    f"Cannot access index {attr} in {type(result)}"
                ) from e
        else:
            try:
                # Handle standard attribute
                result = getattr(result, attr)
            except AttributeError as e:
                raise AttributeError(
                    f"Given attribute {attr} is not part of class {type(result)} in object {type(object)}"
                ) from e
    return result


def _get_value_from_msg(msg: Any, attributes: List[str]) -> Any:
    """
    Extracts message value and ensures it is in a standard Python format (converting arrays to lists).
    """
    # 1. Reuse your existing access logic
    val = _access_attribute(msg, attributes)

    # 2. Handle the specific array case (previously in Operand.__init__)
    if isinstance(val, array.array):
        return val.tolist()

    return val


def _get_attribute_type(cls: Any, attrs: Union[Tuple, List]):
    """
    Gets the type of a nested attribute (specified by attrs) in given object

    :param obj: Object
    :type obj: Any

    :raises AttributeError: If nested attribute does not exist in object

    :return: Type of nested attribute
    :rtype: Any
    """
    try:
        result = cls()
        for attr in attrs:
            result = getattr(result, str(attr))
        return type(result)
    except AttributeError as e:
        raise AttributeError(
            f"Given nested attributes '{attrs}' are not part of class {cls}"
        ) from e


def _check_attribute(cls, expected_type, attrs: Union[Tuple, List]):
    """
    Checks if the given class has the nested attribute specified by attrs
    """
    try:
        current_cls = cls()
        for attr in attrs:
            if not hasattr(current_cls, attr):
                return False
            current_cls = getattr(current_cls, str(attr))
        # Handle the case of MultiArray data type
        if isinstance(current_cls, array.array) and (
            expected_type in [List, np.ndarray, list]
        ):
            return True
        return isinstance(current_cls, expected_type)
    except AttributeError:
        return False


class ConditionLogicOp(Enum):
    NONE = 0
    AND = 1
    OR = 2
    NOT = 3


class Condition:
    """Condition class used for defining events on incoming topic data.
    It captures the logic of an expression like `topic.msg.data > 5` to be evaluated at runtime.

    ```{list-table}
    :widths: 15 20 65
    :header-rows: 1

    * - Attribute
      - Type
      - Description

    * - **topic_source**
      - `Topic`
      - The source Topic object that provides the data stream.

    * - **attribute_path**
      - `List[str | int]`
      - The traversal path to access the specific field within the ROS message (e.g., `['header', 'stamp', 'sec']`).

    * - **operator_func**
      - `Callable`
      - The comparison function (from `ConditionOperators`) used to evaluate the message field against the reference value.

    * - **ref_value**
      - `Any`
      - The reference value (or list of values) to compare the message field against.
    ```
    """

    def __init__(
        self,
        # Composite Attributes
        sub_conditions: Optional[List["Condition"]] = None,
        logic_operator: ConditionLogicOp = ConditionLogicOp.NONE,  # Assuming ConditionLogicOp.NONE or similar
        # Simple Condition (leaf) Attributes
        topic_name: Optional[str] = None,
        topic_msg_type: Optional[str] = None,
        topic_qos_config: Optional[Dict] = None,
        attribute_path: Optional[List[str]] = None,
        operator_func: Optional[Callable] = None,
        ref_value: Any = None,
    ):
        # Initialize Composite Attributes
        self.sub_conditions = sub_conditions if sub_conditions is not None else []
        self.logic_operator = logic_operator

        # Initialize Simple Condition Attributes
        # NOTE: we maintain the serialized topic data instead of the topic object (to avoid circular imports)
        self.topic_name = topic_name
        self.topic_msg_type = topic_msg_type
        self.topic_qos_config = topic_qos_config
        self.attribute_path = attribute_path if attribute_path is not None else []
        self.operator_func = operator_func
        self.ref_value = ref_value

    def __repr__(self):
        return f"Condition(sub_conditions={self.sub_conditions}, logic_operator={self.logic_operator}, topic={self.topic_name}, path={self.attribute_path}, op={self.operator_func})"

    def _readable(self) -> str:
        if self.logic_operator != ConditionLogicOp.NONE:
            text = ""
            for c in self.sub_conditions:
                new_text = c._readable()
                text = text + f" {self.logic_operator} " + new_text
            return text
        if self.operator_func:
            return f"{self.topic_name}{self.attribute_path} {ConditionOperators.get_name(self.operator_func)} {self.ref_value}"
        else:
            return f"{self.topic_name}"

    # --------------------------------------------------------------------------
    # Serialization / Deserialization
    # --------------------------------------------------------------------------
    def to_dict(self) -> Dict[str, Any]:
        """Convert the condition tree to a dictionary."""
        if self.logic_operator != ConditionLogicOp.NONE:
            # --- Serialize Composite Node ---
            return {
                "type": "composite",
                "logic_operator": self.logic_operator.value,
                "sub_conditions": [c.to_dict() for c in self.sub_conditions],
            }
        else:
            # --- Serialize Leaf Node ---
            return {
                "type": "simple",
                "topic_name": self.topic_name,
                "topic_msg_type": self.topic_msg_type,
                "topic_qos_config": self.topic_qos_config,
                "attribute_path": self.attribute_path,
                "operator": self._serialized_operator(),
                "ref_value": self.ref_value,
            }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Condition":
        """Reconstruct the condition tree from a dictionary."""
        node_type = data.get(
            "type", "simple"
        )  # Default to simple for backward compatibility

        if node_type == "composite" or "sub_conditions" in data:
            # --- Deserialize Composite Node ---
            logic_op = ConditionLogicOp(data.get("logic_operator"))
            subs_data = data.get("sub_conditions", [])
            sub_conditions = [cls.from_dict(sub) for sub in subs_data]

            return cls(sub_conditions=sub_conditions, logic_operator=logic_op)

        else:
            # --- Deserialize Leaf Node ---
            return cls(
                topic_name=data.get("topic_name"),
                topic_msg_type=data.get("topic_msg_type"),
                topic_qos_config=data.get("topic_qos_config"),
                attribute_path=data.get("attribute_path", []),
                ref_value=data.get("ref_value", None),
                operator_func=cls._deserialized_operator(data.get("operator", "none")),
            )

    def to_json(self) -> str:
        """Serialize to JSON string."""
        return json.dumps(self.to_dict(), indent=4)

    @classmethod
    def from_json(cls, json_str: str) -> "Condition":
        """Deserialize from JSON string."""
        data = json.loads(json_str)
        return cls.from_dict(data)

    # --------------------------------------------------------------------------
    # Existing Operator Helpers
    # --------------------------------------------------------------------------
    def _serialized_operator(self) -> str:
        if self.operator_func is None:
            return "none"
        return ConditionOperators.get_name(self.operator_func)

    @classmethod
    def _deserialized_operator(cls, serialized_operator) -> Optional[Callable]:
        if serialized_operator == "none":
            return None
        return ConditionOperators.get_operator(serialized_operator)

    def _get_involved_topics(self) -> Dict:
        """Recursively collect all Topic objects involved in this condition"""
        if self.logic_operator != ConditionLogicOp.NONE:
            topics = {}
            for c in self.sub_conditions:
                new_topics = c._get_involved_topics()
                topics = {**topics, **new_topics}
            return topics  # Return unique involved topics
        return {
            self.topic_name: {
                "msg_type": self.topic_msg_type,
                "qos_profile": self.topic_qos_config,
            }
        }

    def evaluate(self, topic_data_cache: Dict[str, Any]) -> bool:
        """Recursively evaluate the condition using all sub-conditions

        :param topic_data_cache: A dictionary mapping topic_name -> latest_ros_msg
        :type topic_data_cache: Dict[str, Any]

        :return: True if the condition is met, else False
        :rtype: bool
        """
        # --- Composite Logic ---
        if self.logic_operator == ConditionLogicOp.AND:
            return all(c.evaluate(topic_data_cache) for c in self.sub_conditions)
        elif self.logic_operator == ConditionLogicOp.OR:
            return any(c.evaluate(topic_data_cache) for c in self.sub_conditions)
        elif self.logic_operator == ConditionLogicOp.NOT:
            return not self.sub_conditions[0].evaluate(topic_data_cache)

        # --- Leaf Logic (ConditionLogicOp.NONE) ---
        # Check if the data for this topic is received
        topic_name = self.topic_name
        if topic_name not in topic_data_cache:
            return False  # Fail safe if data hasn't arrived yet

        msg = topic_data_cache[topic_name]

        # Extract value
        try:
            val = _get_value_from_msg(msg, self.attribute_path)
            # Compare
            # NOTE: operator_func is None -> return True for 'On Any' condition (msg is available)
            return (
                True
                if self.operator_func is None
                else self.operator_func(val, self.ref_value)
            )
        except AttributeError:
            # Safe Fallback
            return False

    # --- Overload Bitwise Operators for Composed Conditions ---
    def __and__(self, other):
        if isinstance(other, Condition):
            return Condition(
                sub_conditions=[self, other], logic_operator=ConditionLogicOp.AND
            )
        else:
            # To handle topic (can be passed to events for 'on any' logic)
            try:
                other_condition = Condition(
                    topic_name=other.name,
                    topic_msg_type=other.msg_type.__name__,
                    topic_qos_config=other.qos_profile.to_dict(),
                    attribute_path=[],
                )
                return Condition(
                    sub_conditions=[self, other_condition],
                    logic_operator=ConditionLogicOp.AND,
                )
            except Exception as e:
                raise TypeError(
                    f"Incompatible condition type. Cannot use '&' operator between a Condition object and object of type {type(other)}"
                ) from e

    def __rand__(self, other):
        """Handles: Topic & Condition"""
        # Since & is commutative in logic, we can just reuse __and__
        return self.__and__(other)

    def __or__(self, other):
        if isinstance(other, Condition):
            return Condition(
                sub_conditions=[self, other], logic_operator=ConditionLogicOp.OR
            )
        else:
            # To handle topic (can be passed to events for 'on any' logic)
            try:
                other_condition = Condition(
                    topic_name=other.name,
                    topic_msg_type=other.msg_type.__name__,
                    topic_qos_config=other.qos_profile.to_dict(),
                    attribute_path=[],
                )
                return Condition(
                    sub_conditions=[self, other_condition],
                    logic_operator=ConditionLogicOp.OR,
                )
            except Exception as e:
                raise TypeError(
                    f"Incompatible condition type. Cannot use '|' operator between a Condition object and object of type {type(other)}"
                ) from e

    def __ror__(self, other):
        """Handles: Topic | Condition"""
        # Since & is commutative in logic, we can just reuse __and__
        return self.__or__(other)

    def __invert__(self):
        return Condition(sub_conditions=[self], logic_operator=ConditionLogicOp.NOT)


class ConditionOperators:
    """
    Registry of supported operators for Conditions.
    Enables serialization by mapping function names to callables.
    """

    _registry = {}

    # --- Internal Decorator ---
    def _register(func, registry=_registry):
        registry[func.__name__] = func
        return func

    @classmethod
    def get_operator(cls, name: str) -> Callable:
        """Retrieve function by name (for Deserialization)."""
        if name not in cls._registry:
            raise ValueError(f"Unknown operator: {name}")
        return cls._registry[name]

    @classmethod
    def get_name(cls, func: Callable) -> str:
        """Retrieve name by function (for Serialization)."""
        # Search registry for the function
        for name, registered_func in cls._registry.items():
            if registered_func == func:
                return name
        # Fallback for standard operators if passed directly
        if func == operator.eq:
            return "equals"
        if func == operator.ne:
            return "not_equals"
        if func == operator.gt:
            return "greater_than"
        if func == operator.ge:
            return "greater_or_equal"
        if func == operator.lt:
            return "less_than"
        if func == operator.le:
            return "less_or_equal"

        raise ValueError(
            f"Operator function {func} is not registered in ConditionOperators"
        )

    # --- Standard Comparison Operators ---

    @staticmethod
    @_register
    def equals(op_obj, ref):
        return op_obj == ref

    @staticmethod
    @_register
    def not_equals(op_obj, ref):
        return op_obj != ref

    @staticmethod
    @_register
    def greater_than(op_obj, ref):
        return op_obj > ref

    @staticmethod
    @_register
    def greater_or_equal(op_obj, ref):
        return op_obj >= ref

    @staticmethod
    @_register
    def less_than(op_obj, ref):
        return op_obj < ref

    @staticmethod
    @_register
    def less_or_equal(op_obj, ref):
        return op_obj <= ref

    # --- Container / Set Logic Operators ---
    # Helper to extract raw value from Operand if needed
    @staticmethod
    def _unwrap(op_obj):
        return op_obj.value if hasattr(op_obj, "value") else op_obj

    @staticmethod
    def _unwrap_list(op_obj) -> Union[List, Tuple]:
        val = ConditionOperators._unwrap(op_obj)
        return val if isinstance(val, (list, tuple)) else [val]

    @staticmethod
    @_register
    def is_in(op_obj, ref_list):
        """Checks if Topic Value is IN Reference List"""
        # Note: We unwrap because 'Operand in List' checks for the object instance, not the value
        return ConditionOperators._unwrap(op_obj) in ref_list

    @staticmethod
    @_register
    def not_in(op_obj, ref_list):
        """Checks if Topic Value is NOT IN Reference List"""
        return ConditionOperators._unwrap(op_obj) not in ref_list

    @staticmethod
    @_register
    def contains(op_obj, ref_val):
        """Topic (string) contains a reference string"""
        return ref_val in op_obj

    @staticmethod
    @_register
    def not_contains(op_obj, ref_val):
        """Topic (string) does not contain a reference string"""
        return ref_val not in op_obj

    @staticmethod
    @_register
    def contains_any(op_obj, ref_list):
        """Topic (list) contains ANY of Reference (list)"""
        val_list = ConditionOperators._unwrap_list(op_obj)
        return any(item in val_list for item in ref_list)

    @staticmethod
    @_register
    def contains_all(op_obj, ref_list):
        """Topic (list) contains ALL of Reference (list)"""
        val_list = ConditionOperators._unwrap_list(op_obj)
        return all(item in val_list for item in ref_list)

    @staticmethod
    @_register
    def not_contains_any(op_obj, ref_list):
        """Topic (list) contains NONE of Reference (list)"""
        val_list = ConditionOperators._unwrap_list(op_obj)
        return not any(item in val_list for item in ref_list)

    @staticmethod
    @_register
    def not_contains_all(op_obj, ref_list):
        """Topic (list) does MISSING at least one of Reference (list)"""
        val_list = ConditionOperators._unwrap_list(op_obj)
        return not all(item in val_list for item in ref_list)


class MsgConditionBuilder:
    """Helper class to build paths for accessing ROS message attributes in topics.
    Used for parsing topic message attributes for Actions and Event parsers.
    """

    def __init__(self, topic, path=None):
        self._name = topic.name
        self._type = topic.ros_msg_type
        self._topic = topic
        self._base = path or []

    def __getattr__(self, name: str) -> "MsgConditionBuilder":
        # Validate that the attribute exists in the message type
        try:
            augmented_base = self._base + [name]
            start = self._type()
            for key, attribute_name in enumerate(augmented_base):
                if not hasattr(start, attribute_name):
                    old_bases = ".".join(augmented_base[:key])
                    error = (
                        f"Available attributes: {start.get_fields_and_field_types()}"
                        if hasattr(start, "get_fields_and_field_types")
                        else ""
                    )
                    raise AttributeError(
                        f"Message '{self._type.__name__}.{old_bases}' has no attribute: '{augmented_base[key]}'. "
                        + error
                    )
                start = getattr(start, attribute_name)
            return MsgConditionBuilder(self._topic, augmented_base)
        except Exception:
            # NOTE: This exception is added to avoid bugs when using an object of the class with system introspection (like pickle or inspect).
            raise AttributeError("See the parent error raised above") from None

    def __deepcopy__(self, memo):
        # Manually create the new object to avoid errors from deepcopy which creates empty object instances before populating their dictionary.
        new_obj = MsgConditionBuilder(self._topic, copy.deepcopy(self._base, memo))
        return new_obj

    def as_tuple(self) -> Tuple:
        return tuple(self._base)

    @property
    def name(self) -> str:
        return self._name

    @property
    def topic(self):
        return self._topic

    @topic.setter
    def topic(self, value):
        self._topic = value
        self._name = value.name
        self._type = value.ros_msg_type

    @property
    def type(self) -> Type:
        return self._type

    # TODO: Enable accessing the processed output in conditions and action attributes
    # @property
    # def processed_output(self) -> "MsgConditionBuilder":
    #     augmented_base = self._base + ["get_output"]
    #     return MsgConditionBuilder(self._topic, augmented_base)

    def get_value(self, object_value=None) -> Any:
        val = object_value or self._type()
        for attribute_name in self._base:
            val = getattr(val, attribute_name)
        return val

    def to_json(self):
        self_dict = {"topic": self.topic.to_json(), "path": self._base}
        return json.dumps(self_dict)

    def _check_similar_type(self, other):
        """Helper method to ensure compatible condition types"""
        val = self.get_value()

        # Allow Exact Match
        if type(val) is type(other):
            return

        # Allow Numeric Compatibility (int vs float)
        if isinstance(val, (int, float)) and isinstance(other, (int, float)):
            return

        # Allow Sequence Compatibility (list vs array.array vs tuple)
        # We explicitly check for list-like types excluding strings
        sequence_types = (list, tuple, array.array, np.ndarray)
        if isinstance(val, sequence_types) and isinstance(other, sequence_types):
            return

        # If none of the above passed, types are incompatible
        raise TypeError(
            f"Cannot construct condition from incompatible types: "
            f"Topic attribute type is '{type(val).__name__}', "
            f"and reference type is '{type(other).__name__}'"
        )

    def _make_condition(self, op: Callable, value: Any) -> Condition:
        if value is not None and not _check_attribute(
            self._topic.ros_msg_type,
            type(value),
            self._base,
        ):
            raise TypeError(
                f"Condition Initialization error. Cannot initiate using attribute '{self._base}' for class '{self._topic.ros_msg_type}' with trigger of type '{type(value)}'. Trigger should be of type: '{_get_attribute_type(self._topic.ros_msg_type, self._base)}'"
            )
        return Condition(
            topic_name=self._topic.name,
            topic_msg_type=self._topic.msg_type.__name__,
            topic_qos_config=self._topic.qos_profile.to_dict(),
            attribute_path=self._base,
            operator_func=op,
            ref_value=value,
        )

    # --- Dunder Methods for Conditional Parsing ---
    def __eq__(self, other) -> Condition:  # type: ignore
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.equals, other)

    def __ne__(self, other) -> Condition:  # type: ignore
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.not_equals, other)

    def __lt__(self, other) -> Condition:
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.less_than, other)

    def __le__(self, other) -> Condition:
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.less_or_equal, other)

    def __gt__(self, other) -> Condition:
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.greater_than, other)

    def __ge__(self, other) -> Condition:
        self._check_similar_type(other)
        return self._make_condition(ConditionOperators.greater_or_equal, other)

    def __invert__(self) -> Condition:
        """
        Shorthand for checking if a boolean is False.
        Usage: ~topic.msg.is_enabled  (Equivalent to is_enabled == False)
        """
        self._check_similar_type(False)
        # You can map this to == False, or a specific NOT operator if you have one
        return self._make_condition(ConditionOperators.equals, False)

    def is_in(self, other: Union[List, Tuple, str]) -> Condition:
        """
        Check if the topic value is inside the provided list/tuple.
        Usage: topic.msg.status.is_in([1, 2, 3])
        """
        # We use operator.contains.
        # When Event executes: operator.contains(Operand(msg_val), [1,2,3])
        # This calls Operand.__contains__([1,2,3])
        if isinstance(other, str):
            self._check_similar_type(other)
            return self._make_condition(ConditionOperators.is_in, other)
        return self._make_condition(ConditionOperators.is_in, other)

    def not_in(self, other: Union[List, Tuple, str]) -> Condition:
        """
        Check if the topic value is NOT inside the provided list/tuple.
        Usage: topic.msg.status.not_in([0, -1])
        """
        if isinstance(other, str):
            self._check_similar_type(other)
            return self._make_condition(ConditionOperators.not_in, other)

        return self._make_condition(ConditionOperators.not_in, other)

    # --- Helper to safely get list from Operand ---
    @staticmethod
    def _get_value_as_list(op_obj) -> Union[List, Tuple]:
        # Helper to ensure we are comparing against a list, even if the topic is scalar
        val = op_obj.value
        return val if isinstance(val, (list, tuple)) else [val]

    def contains_any(self, other: Union[List, Tuple]) -> Condition:
        """
        True if the topic value contains AT LEAST ONE of the values in 'other'.
        Equivalent to: set(topic) & set(other) is not empty
        """
        return self._make_condition(ConditionOperators.contains_any, other)

    def contains_all(self, other: Union[List, Tuple]) -> Condition:
        """
        True if the topic value contains ALL of the values in 'other'.
        Equivalent to: set(other).issubset(set(topic))
        """
        return self._make_condition(ConditionOperators.contains_all, other)

    def contains(self, other: Union[List, Tuple, str]) -> Condition:
        """
        If value contains another string
        If other is a list: works same as contains_all
        """
        # We use operator.contains.
        # When Event executes: operator.contains(Operand(msg_val), "some string")
        if isinstance(other, str):
            self._check_similar_type(other)
            return self._make_condition(ConditionOperators.contains, other)
        return self.contains_all(other)

    def not_contains_any(self, other: Union[List, Tuple]) -> Condition:
        """
        True if the topic value contains NONE of the values in 'other'.
        Equivalent to: set(topic).isdisjoint(set(other))
        """
        return self._make_condition(ConditionOperators.not_contains_any, other)

    def not_contains_all(self, other: Union[List, Tuple]) -> Condition:
        """
        True if the topic value is MISSING at least one value from 'other'.
        Inverse of contains_all.
        """
        return self._make_condition(ConditionOperators.not_contains_all, other)

    def not_contains(self, other: Union[List, Tuple, str]) -> Condition:
        """
        If value does not contain another string
        If other is a list: works same as not_contains_all
        """
        # We use operator.contains.
        # When Event executes: operator.contains(Operand(msg_val), "some string")
        if isinstance(other, str):
            self._check_similar_type(other)
            return self._make_condition(ConditionOperators.not_contains, other)
        return self.not_contains_all(other)

    def is_true(self) -> Condition:
        """
        Create a condition checking if the boolean attribute is True.
        Usage: topic.msg.is_enabled.is_true()
        """
        # Ensure we are comparing against a boolean type in the msg
        self._check_similar_type(True)
        return self._make_condition(ConditionOperators.equals, True)

    def is_false(self) -> Condition:
        """
        Create a condition checking if the boolean attribute is False.
        Usage: topic.msg.is_enabled.is_false()
        """
        self._check_similar_type(False)
        return self._make_condition(ConditionOperators.equals, False)

    def __getitem__(self, key: int):
        """
        Allow array indexing in the path.
        Usage: topic.msg.data[0] > 5
        """
        if not isinstance(key, int):
            raise TypeError("Only integer indices are supported for arrays.")

        # Add the integer index to the path
        augmented_base = self._base + [key]

        # Validation Logic (Optional but recommended)
        # We need to temporarily check if the current object is indexable
        # This is harder to validate statically with types, but we can try:
        # start = self._get_value_at_current_path()
        # if not hasattr(start, '__getitem__'): error...

        return MsgConditionBuilder(self._topic, augmented_base)
