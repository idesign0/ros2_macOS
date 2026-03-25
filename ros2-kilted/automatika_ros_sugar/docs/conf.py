# Configuration file for the Sphinx documentation builder.
import os
import sys
from datetime import date
import xml.etree.ElementTree as ET
from pathlib import Path


sys.path.insert(0, os.path.abspath(".."))
version = ET.parse("../package.xml").getroot()[1].text
print("Found version:", version)

project = "Sugarcoat"
copyright = f"{date.today().year}, Automatika Robotics"
author = "Automatika Robotics"
release = version

extensions = [
    "sphinx.ext.viewcode",
    "sphinx.ext.doctest",
    "sphinx_copybutton",  # install with `pip install sphinx-copybutton`
    "autodoc2",  # install with `pip install sphinx-autodoc2`
    "myst_parser",  # install with `pip install myst-parser`
    "sphinxcontrib.youtube",
    "sphinx_design",
]

autodoc2_packages = [
    {
        "path": "../ros_sugar",
        "module": "ros_sugar",
        "exclude_files": [
            "utils.py",
            "io/utils.py",
        ],
    },
]
autodoc2_module_all_regexes = [r"core\*"]
autodoc2_hidden_objects = ["private", "dunder", "undoc"]
autodoc2_class_docstring = "both"  # bug in autodoc2, should be `merge`
autodoc2_render_plugin = "myst"


templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "README*"]

myst_enable_extensions = [
    "amsmath",
    "attrs_inline",
    "colon_fence",
    "deflist",
    "dollarmath",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",  # install with pip install linkify-it-py
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
]
myst_heading_anchors = 7  # to remove cross reference errors with md

language = "en"
html_theme = "shibuya"  # install with `pip install shibuya`
html_static_path = ["_static"]
html_css_files = [
    "custom.css",
]
html_favicon = "_static/favicon.png"

html_theme_options = {
    "announcement": 'Usage docs have moved to <a href="https://emos.automatikarobotics.com">EMOS Documentation</a>. This site only contains developer docs.',
    "light_logo": "_static/SUGARCOAT_LIGHT.png",
    "dark_logo": "_static/SUGARCOAT_DARK.png",
    "accent_color": "indigo",
    "twitter_url": "https://x.com/__automatika__",
    "github_url": "https://github.com/automatika-robotics/sugarcoat",
    "discord_url": "https://discord.gg/B9ZU6qjzND",
    "globaltoc_expand_depth": 1,
    "open_in_chatgpt": True,
    "open_in_claude": True,
    # Navigation Links (Top bar)
    "nav_links": [
        {"title": "EMOS Docs", "url": "https://emos.automatikarobotics.com/"},
        {"title": "Automatika Robotics", "url": "https://automatikarobotics.com/"},
    ],
}

LLMS_TXT_SELECTION = [
    "development/architecture.md",
    "development/custom_types.md",
    "development/event_system.md",
    "development/testing.md",
    "advanced/create_service.md",
    "advanced/srvs.md",
]


def format_for_llm(filename: str, content: str) -> str:
    """Helper to wrap content in a readable format for LLMs."""
    # Clean up HTML image tags to reduce noise
    lines = content.split("\n")
    cleaned_lines = [line for line in lines if "<img src=" not in line]
    cleaned_content = "\n".join(cleaned_lines).strip()

    return f"## File: {filename}\n```markdown\n{cleaned_content}\n```\n\n"


def generate_llms_txt(app, exception):
    """Generates llms.txt combining manual docs and autodoc2 API docs."""
    if exception is not None:
        return  # Do not generate if build failed

    print("[llms.txt] Starting generation...")

    src_dir = Path(app.srcdir)
    out_dir = Path(app.outdir)
    full_text = []

    # Add Preamble
    preamble = (
        "# Sugarcoat Developer Documentation\n\n"
        "The following text contains the developer documentation for the Sugarcoat framework "
        "by Automatika Robotics. It is optimized for context ingestion.\n\n"
    )
    full_text.append(preamble)

    # Process Manual Docs (Curated List)
    print(f"[llms.txt] Processing {len(LLMS_TXT_SELECTION)} manual files...")
    for relative_path in LLMS_TXT_SELECTION:
        file_path = src_dir / relative_path
        if file_path.exists():
            content = file_path.read_text(encoding="utf-8")
            full_text.append(format_for_llm(relative_path, content))
        else:
            print(f"[llms.txt] Warning: Manual file not found: {relative_path}")

    # Write output to the build root
    output_path = out_dir / "llms.txt"
    try:
        output_path.write_text("".join(full_text), encoding="utf-8")
        print(f"[llms.txt] Successfully generated: {output_path}")
    except Exception as e:
        print(f"[llms.txt] Error writing file: {e}")


def setup(app):
    """Plugin to post build and copy markdowns as well"""
    app.connect("build-finished", generate_llms_txt)
