#!/usr/bin/env python3

from __future__ import annotations

import re
import subprocess
from pathlib import Path


patterns = {
    "GitHub token": re.compile(r"\bgh[pousr]_[A-Za-z0-9_]{20,}\b"),
    "AWS access key": re.compile(r"\bAKIA[0-9A-Z]{16}\b"),
    "private key": re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----"),
}

tracked = subprocess.run(
    ["git", "ls-files", "-z"], check=True, capture_output=True
).stdout.split(b"\0")
findings: list[str] = []

for raw_path in tracked:
    if not raw_path:
        continue
    path = Path(raw_path.decode("utf-8"))
    if not path.is_file() or path.stat().st_size > 1_000_000:
        continue
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        continue
    for label, pattern in patterns.items():
        if pattern.search(text):
            findings.append(f"{path}: possible {label}")

if findings:
    raise SystemExit("\n".join(findings))

print("secret scan passed")
