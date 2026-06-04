import pytest

from lps_service_cli.ranges import NodeSelectorError, parse_node_selector


def test_parse_single_node_id() -> None:
    assert parse_node_selector("5") == [5]


def test_parse_inclusive_range() -> None:
    assert parse_node_selector("1..4") == [1, 2, 3, 4]


def test_parse_comma_separated_nodes() -> None:
    assert parse_node_selector("1,3,5") == [1, 3, 5]


def test_parse_mixed_commas_and_ranges() -> None:
    assert parse_node_selector("1,3..5,9") == [1, 3, 4, 5, 9]


def test_rejects_descending_range() -> None:
    with pytest.raises(NodeSelectorError, match="descending"):
        parse_node_selector("5..3")


def test_rejects_out_of_range_node_id() -> None:
    with pytest.raises(NodeSelectorError, match="0..255"):
        parse_node_selector("256")


def test_rejects_empty_selector() -> None:
    with pytest.raises(NodeSelectorError, match="empty"):
        parse_node_selector("")


def test_rejects_duplicate_node_id() -> None:
    with pytest.raises(NodeSelectorError, match="duplicate"):
        parse_node_selector("1,2,1")
