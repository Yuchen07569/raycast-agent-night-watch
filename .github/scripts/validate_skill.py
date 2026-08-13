#!/usr/bin/env python3

from __future__ import annotations

import os
import re
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"skill validation failed: {message}", file=sys.stderr)
    raise SystemExit(1)


root = Path(sys.argv[1] if len(sys.argv) > 1 else ".agents/skills/agent-night-watch")
skill_file = root / "SKILL.md"
metadata_file = root / "agents" / "openai.yaml"
preflight_file = root / "scripts" / "preflight.sh"

for required in (skill_file, metadata_file, preflight_file):
    if not required.is_file():
        fail(f"missing {required}")

skill_text = skill_file.read_text(encoding="utf-8")
match = re.match(r"\A---\n(.*?)\n---\n", skill_text, re.DOTALL)
if not match:
    fail("SKILL.md frontmatter is missing or malformed")

frontmatter_lines = [line for line in match.group(1).splitlines() if line.strip()]
keys = [line.split(":", 1)[0].strip() for line in frontmatter_lines]
if keys != ["name", "description"]:
    fail("SKILL.md frontmatter must contain only name and description")
if frontmatter_lines[0] != "name: agent-night-watch":
    fail("skill name must be agent-night-watch")
if len(frontmatter_lines[1].split(":", 1)[1].strip()) < 40:
    fail("skill description is too short")

metadata = metadata_file.read_text(encoding="utf-8")
short_match = re.search(r'^\s*short_description:\s*"([^"]+)"\s*$', metadata, re.MULTILINE)
if not short_match or not 25 <= len(short_match.group(1)) <= 64:
    fail("short_description must contain 25 to 64 characters")
if "$agent-night-watch" not in metadata:
    fail("default_prompt must mention $agent-night-watch")
if not re.search(r"allow_implicit_invocation:\s*false", metadata):
    fail("implicit invocation must be disabled")

preflight = preflight_file.read_text(encoding="utf-8")
for forbidden in ("sudo ", "pmset -a", "with administrator privileges"):
    if forbidden in preflight:
        fail(f"preflight contains forbidden write capability: {forbidden}")
if not os.access(preflight_file, os.X_OK):
    fail("preflight.sh must be executable")

print("skill validation passed")
