# /// script
# requires-python = ">=3.12"
# dependencies = ["markdown==3.9", "segno==1.6.6", "python-barcode==0.16.1"]
# ///
"""Render TEST_PLAN.md into test_page.html: the plan with its barcodes and a result log.

The page shows every test barcode full-screen on tap (to scan it from a phone) and
records Pass / Fail / Skip and a note per step. Published as a claude.ai artifact the
results are kept in the artifact's database, so they can be read back; opened as a
local file they are kept in the browser.

Run: uv run tests/hardware/make_test_page.py
"""

from __future__ import annotations

import html
import io
import json
import re
from pathlib import Path

import barcode
import markdown
import segno
from barcode.writer import SVGWriter

HERE = Path(__file__).parent
PLAN = HERE / "TEST_PLAN.md"
OUTPUT = HERE / "test_page.html"

# Longest content shown in full on a barcode card
MAX_CARD_CONTENT = 40
# Contents the plan's barcode table describes rather than spells out.
CONTENT_OVERRIDES = {"D3": "".join(f"{i:04d}-" for i in range(1, 61))}


def parse_barcodes(plan: str) -> list[dict[str, str]]:
    """Read the barcode table at the end of the plan."""
    rows = re.findall(
        r"^\| `([DC]\d)` \| ([^|]+) \| `([^`]+)`[^|]*\| ([^|]+) \|$", plan, re.MULTILINE
    )
    codes = []
    for label, kind, content, purpose in rows:
        codes.append(
            {
                "label": label,
                "kind": kind.strip(),
                "content": CONTENT_OVERRIDES.get(label, content),
                "purpose": purpose.strip(),
            }
        )
    return codes


def svg_for(kind: str, content: str) -> str:
    """Return an inline SVG for a barcode that scales to its container."""
    if kind == "QR":
        qr = segno.make(content, error="m", micro=False)
        return qr.svg_inline(omitsize=True, border=4, dark="#000", light="#fff")
    name = {"EAN-13": "ean13", "Code 128": "code128"}[kind]
    data = content[:12] if name == "ean13" else content
    out = io.BytesIO()
    barcode.get(name, data, writer=SVGWriter()).write(
        out, options={"module_height": 18, "quiet_zone": 6, "font_size": 8}
    )
    svg = out.getvalue().decode()
    svg = svg[svg.index("<svg") :]
    width = re.search(r'width="([\d.]+)mm"', svg).group(1)
    height = re.search(r'height="([\d.]+)mm"', svg).group(1)
    svg = re.sub(r'width="[\d.]+mm" height="[\d.]+mm"', "", svg, count=1)
    return svg.replace("<svg", f'<svg viewBox="0 0 {width} {height}"', 1)


def render_plan(plan: str) -> tuple[str, list[str]]:
    """Convert the plan to HTML, turning each step into a recordable row."""
    body = plan.split("\n## Barcodes", maxsplit=1)[0]
    body = body.split("\n", 1)[1]  # the page has its own heading
    text = markdown.markdown(body, extensions=["tables", "sane_lists"])
    steps: list[str] = []

    def step(match: re.Match[str]) -> str:
        sid, gate, rest = match.group(1), match.group(2), match.group(3)
        steps.append(sid)
        badge = '<span class="gate">Gate</span>' if gate else ""
        return (
            f'<li class="step" data-step="{sid}" id="s{sid.replace(".", "-")}">'
            f'<div class="step-head"><span class="sid">{sid}</span>{badge}</div>'
            f'<div class="step-text">{rest.strip()}</div>'
            '<div class="step-controls">'
            '<button type="button" class="mark" data-status="pass">Pass</button>'
            '<button type="button" class="mark" data-status="fail">Fail</button>'
            '<button type="button" class="mark" data-status="skip">Skip</button>'
            f'<input type="text" class="note" id="note-{sid.replace(".", "-")}" '
            'placeholder="Note" aria-label="Note for step ' + sid + '">'
            "</div></li>"
        )

    text = re.sub(
        r"<li><strong>(\d+\.\d+)( GATE)?</strong>(.*?)</li>",
        step,
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"<code>([DC]\d)</code>",
        r'<button type="button" class="chip" data-code="\1">\1</button>',
        text,
    )
    text = re.sub(
        r"<h2>(\d+)\. ([^<]+)</h2>",
        r'<h2 id="phase-\1"><span class="pnum">\1</span>\2</h2>',
        text,
    )
    # The LED legend ("blue = host-mode scan running, ...") gets a swatch per colour.
    text = re.sub(
        r"\b(blue|purple|green|orange|white|red)( flash| pulse)? =",
        r'<i class="led led-\1"></i>\1\2 =',
        text,
    )
    return text, steps


def gallery(codes: list[dict[str, str]]) -> str:
    """Render the barcode cards."""
    cards = []
    for c in codes:
        content = c["content"]
        shown = content if len(content) <= MAX_CARD_CONTENT else content[:37] + "…"
        cards.append(
            f'<button type="button" class="card" data-code="{c["label"]}">'
            f'<span class="card-code">{svg_for(c["kind"], c["content"])}</span>'
            f'<span class="card-label">{c["label"]}</span>'
            f'<span class="card-kind">{html.escape(c["kind"])} · '
            f"{html.escape(c['purpose'])}</span>"
            f'<span class="card-content">{html.escape(shown)}</span></button>'
        )
    return "\n".join(cards)


def main() -> None:
    """Write test_page.html."""
    plan = PLAN.read_text()
    codes = parse_barcodes(plan)
    plan_html, steps = render_plan(plan)
    page = (HERE / "test_page.template.html").read_text()
    page = page.replace("{{PLAN}}", plan_html)
    page = page.replace("{{GALLERY}}", gallery(codes))
    page = page.replace("{{STEPS}}", json.dumps(steps))
    page = page.replace(
        "{{CODES}}",
        json.dumps(
            {
                c["label"]: {
                    "svg": svg_for(c["kind"], c["content"]),
                    "kind": c["kind"],
                    "purpose": c["purpose"],
                    "content": c["content"],
                }
                for c in codes
            }
        ),
    )
    OUTPUT.write_text(page)
    print(f"{OUTPUT}: {len(steps)} steps, {len(codes)} barcodes")  # noqa: T201


if __name__ == "__main__":
    main()
