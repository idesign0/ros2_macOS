from functools import partial
from typing import Any, List, Union, Callable

import numpy as np


Validator = Callable[[Any, Any, Any], None]


def gt(value: Union[int, float]) -> Validator:
    """
    Validates that field is greater than value

    :param value: Value
    :type value: int | float

    :return: Attrs validator function
    :rtype: func
    """
    return partial(__gt, ref_value=value)


def lt(value: Union[int, float]) -> Validator:
    """
    Validates that field is lesser than value

    :param value: Value
    :type value: int | float

    :return: Attrs validator function
    :rtype: func
    """
    return partial(__lt, ref_value=value)


def in_(values: List) -> Validator:
    """
    Validates that value is in a given list

    :param values: Reference list of values
    :type values: List

    :return: Attrs validator function
    :rtype: func
    """
    return partial(__in_, values=values)


def list_contained_in(values: List) -> Validator:
    """
    Validates that all elements in a given list are in values

    :param values: Reference list of values
    :type values: List

    :return: Attrs validator function
    :rtype: func
    """
    return partial(__list_contained_in, values=values)


def in_range(min_value: Union[float, int], max_value: Union[float, int]) -> Validator:
    """
    Validates that a given value is within range

    :param min_value: Minimum value
    :type min_value: Union[float, int]
    :param max_value: Maximum value
    :type max_value: Union[float, int]

    :return: Attrs validator function
    :rtype: func
    """
    return partial(__in_range_validator, min_value=min_value, max_value=max_value)


def in_range_discretized(
    step: Union[float, int],
    min_value: Union[float, int],
    max_value: Union[float, int],
) -> Validator:
    """
    Validates that a given value is within range with a given step

    :param step: Step size
    :type step: Union[float, int]
    :param min_value: Minimum value
    :type min_value: Union[float, int]
    :param max_value: Maximum value
    :type max_value: Union[float, int]

    :return: Attrs validator function
    :rtype: func
    """
    return partial(
        __in_range_discretized_validator,
        step=step,
        min_value=min_value,
        max_value=max_value,
    )


def __gt(_: Any, attribute: Any, value: Any, ref_value: Union[int, float]):
    """__gt.

    :param _:
    :type _: Any
    :param attribute:
    :type attribute: Any
    :param value:
    :type value: Any
    :param ref_value:
    :type ref_value: Union[int, float]
    """
    if value <= ref_value:
        raise ValueError(
            f"Got value of '{attribute.name}': '{value}', not greater than: '{ref_value}'"
        )


def __lt(_: Any, attribute: Any, value: Any, ref_value: Union[int, float]):
    """__lt.

    :param _:
    :type _: Any
    :param attribute:
    :type attribute: Any
    :param value:
    :type value: Any
    :param ref_value:
    :type ref_value: Union[int, float]
    """
    if value >= ref_value:
        raise ValueError(
            f"Got value of '{attribute.name}': '{value}', not less than: '{ref_value}'"
        )


def __in_(_: Any, attribute: Any, value: Any, values: List):
    """__in_.

    :param _:
    :type _: Any
    :param attribute:
    :type attribute: Any
    :param value:
    :type value: Any
    :param values:
    :type values: List
    """
    if value not in values:
        if len(values) < 5:
            raise ValueError(
                f"Got value of '{attribute.name}': '{value}', not in list: '{values}'"
            )
        else:
            raise ValueError(
                f"Got value of '{attribute.name}': '{value}', not in list: '{values[0]}, ..., {values[-1]}'"
            )


def __list_contained_in(_: Any, attribute: Any, value: List, values: List):
    """__list_contained_in.

    :param _:
    :type _: Any
    :param attribute:
    :type attribute: Any
    :param value:
    :type value: List
    :param values:
    :type values: List
    """
    if not all(val in values for val in value):
        raise ValueError(
            f"Got value of '{attribute.name}': '{value}'. All values in {attribute.name} must be within: '{values}'. Got {value}"
        )


def __in_range_validator(
    _: Any,
    attribute: Any,
    value: Union[float, int],
    min_value: Union[float, int],
    max_value: Union[float, int],
):
    """
    Check if class attribute value is within given range

    :param instance: Class instance
    :type instance: Any
    :param attribute: Class attribute
    :type attribute: Any
    :param value: Attribute value
    :type value: Union[float, int]
    :param min_value: Attribute min value
    :type min_value: Union[float, int]
    :param max_value: Attribute max value
    :type max_value: Union[float, int]

    :raises ValueError: If value is not within given range
    """
    if min_value > value or value > max_value:
        raise ValueError(
            f"Value of {attribute.name} must be between {min_value} and {max_value}. Got {value}"
        )


def __in_range_discretized_validator(
    _: Any,
    attribute: Any,
    value: Union[int, float],
    step: Union[float, int],
    min_value: Union[float, int],
    max_value: Union[float, int],
):
    """
    Check if class attribute value is a multiple of step

    :param instance: Class instance
    :type instance: Any
    :param attribute: Class attribute
    :type attribute: Any
    :param value: Attribute value
    :type value: Union[float, int]
    :param step: Attribute step value
    :type step: Union[float, int]
    :param min_value: Attribute min value
    :type min_value: Union[float, int]
    :param max_value: Attribute max value
    :type max_value: Union[float, int]

    :raises ValueError: If value is not according to correct step
    """
    if isinstance(value, int):
        if value % step and value != min_value and value != max_value:
            raise ValueError(
                f"Value of {attribute.name} must be a multiple of {step} within [{min_value}, {max_value}]. Got {value}"
            )
    else:
        all_vals = np.arange(min_value, max_value, step)
        # check precision upto 1e-05. If step is smaller, behavior would be unexpected.
        if (
            not np.any(np.isclose(value, all_vals))
            and value != min_value
            and value != max_value
        ):
            raise ValueError(
                f"Value of {attribute.name} must be a multiple of {step} (Precision limited to 1e-05). Got {value}"
            )
