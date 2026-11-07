#!/usr/bin/env python3
"""Repair Doxygen 1.18's alphabet-bar destinations in the generated HTML."""

from pathlib import Path
import re
import sys


def repair(directory: Path) -> None:
    for path in sorted(directory.glob("*.html")):
        if not path.name.startswith(("globals", "functions")):
            continue
        original = path.read_text()
        text = original
        # Underscore entries occupy the first globals page, while A entries
        # occupy their own page. Doxygen's alphabet bar reverses these links.
        for stem in ("globals", "globals_func"):
            text = text.replace(f'{stem}__.html#index__',
                                f'{stem}.html#index__5F')
            text = text.replace(f'{stem}.html#index_a',
                                f'{stem}_a.html#index_a')
        # Destructor headings escape the tilde but their links do not.
        text = text.replace('#index_~', '#index__7E')

        # The short enum index gets an alphabet bar but no section anchors.
        # Anchor the first enum in each letter group without changing its list.
        if path.name == "globals_enum.html" and 'id="index_' not in text:
            seen = set()

            def anchor(match: re.Match) -> str:
                letter = match[1].lower()
                if letter in seen:
                    return match[0]
                seen.add(letter)
                return f'<li id="index_{letter}">{match[1]}'

            text = re.sub(r'<li>([A-Za-z])', anchor, text)
        if text != original:
            path.write_text(text)


if __name__ == "__main__":
    repair(Path(sys.argv[1]))
