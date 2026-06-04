from __future__ import annotations


class NodeSelectorError(ValueError):
    """Raised when a node selector cannot be parsed."""


def _parse_node_id(text: str) -> int:
    if not text.isdigit():
        raise NodeSelectorError(f"invalid node id '{text}'")

    value = int(text)
    if value < 0 or value > 255:
        raise NodeSelectorError(f"node id {value} outside valid range 0..255")

    return value


def parse_node_selector(selector: str) -> list[int]:
    """Parse selectors like '5', '1..12', and '1,3..5'."""
    if selector == "":
        raise NodeSelectorError("empty node selector")

    result: list[int] = []
    seen: set[int] = set()

    for raw_part in selector.split(","):
        part = raw_part.strip()
        if part == "":
            raise NodeSelectorError("empty selector component")

        if ".." in part:
            pieces = part.split("..")
            if len(pieces) != 2:
                raise NodeSelectorError(f"invalid range '{part}'")
            start = _parse_node_id(pieces[0].strip())
            end = _parse_node_id(pieces[1].strip())
            if end < start:
                raise NodeSelectorError(f"descending range '{part}' is not supported")
            values = range(start, end + 1)
        else:
            values = [_parse_node_id(part)]

        for value in values:
            if value in seen:
                raise NodeSelectorError(f"duplicate node id {value}")
            seen.add(value)
            result.append(value)

    return result
