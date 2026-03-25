import re


def parse_type(type_str):
    """Helper function to parse the type string."""
    if "Optional" in type_str:
        type_str = type_str.replace("typing.Optional", "").strip("[]")

    match = re.search(r"<class '(.*?)'>", type_str)
    if match:
        return match.group(1), []

    if "Literal" in type_str:
        literal_values = [
            val.strip().strip("'\"")
            for val in type_str[type_str.find("[") + 1 : -1].split(",")
        ]
        return "literal", literal_values

    return type_str, []
