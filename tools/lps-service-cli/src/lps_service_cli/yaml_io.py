from __future__ import annotations

from collections.abc import Mapping
from pathlib import Path
from typing import Any

import yaml


def dumps_yaml(document: Mapping[str, object]) -> str:
    return yaml.safe_dump(document, sort_keys=False, allow_unicode=True)


def loads_yaml(text: str) -> dict[str, Any]:
    document = yaml.safe_load(text)
    if not isinstance(document, dict):
        raise ValueError("YAML document must be a top-level mapping")
    return document


def write_yaml_file(path: str | Path, document: Mapping[str, object]) -> None:
    Path(path).write_text(dumps_yaml(document), encoding="utf-8")


def read_yaml_file(path: str | Path) -> dict[str, Any]:
    return loads_yaml(Path(path).read_text(encoding="utf-8"))
