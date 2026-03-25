# Configuration file for the Sphinx documentation builder.
import os
import sys
from datetime import date
import xml.etree.ElementTree as ET
from pathlib import Path

# Flag to signal that we are building documentation.
# This prevents __init__.py from running runtime dependency checks.
os.environ["AGENTS_DOCS_BUILD"] = "1"

sys.path.insert(0, os.path.abspath(".."))
version = ET.parse("../package.xml").getroot()[1].text
print("Found version:", version)

project = "EmbodiedAgents"
copyright = f"{date.today().year}, Automatika Robotics"
author = "Automatika Robotics"
release = version

extensions = [
    "sphinx.ext.viewcode",
    "sphinx.ext.doctest",
    "sphinx_copybutton",  # install with `pip install sphinx-copybutton`
    "autodoc2",  # install with `pip install sphinx-autodoc2`
    "myst_parser",  # install with `pip install myst-parser`
    "sphinx_design",  # install with `pip install sphinx-design`
]

autodoc2_packages = [
    {
        "module": "agents",
        "path": "../agents",
        "exclude_dirs": ["__pycache__", "utils"],
        "exclude_files": [
            "callbacks.py",
            "publisher.py",
            "component_base.py",
            "model_component.py",
            "model_base.py",
            "db_base.py",
            "executable.py",
        ],
    },
]

autodoc2_docstrings = "all"
autodoc2_class_docstring = "both"  # bug in autodoc2, should be `merge`
autodoc2_render_plugin = "myst"
autodoc2_hidden_objects = ["private", "dunder", "undoc"]
autodoc2_module_all_regexes = [
    r"agents.config",
    r"agents.models",
    r"agents.vectordbs",
    r"agents.ros",
    r"agents.clients\.[^\.]+",
]

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
    "linkify",
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
html_css_files = ["custom.css"]
html_favicon = "_static/favicon.png"

html_theme_options = {
    "announcement": 'Usage docs have moved to <a href="https://emos.automatikarobotics.com">EMOS Documentation</a>. This site only contains developer docs.',
    "light_logo": "_static/EMBODIED_AGENTS_LIGHT.png",
    "dark_logo": "_static/EMBODIED_AGENTS_DARK.png",
    "accent_color": "indigo",
    "twitter_url": "https://x.com/__automatika__",
    "github_url": "https://github.com/automatika-robotics/embodied-agents",
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

# --- LLMS.TXT CONFIGURATION ---
# Defines the order of developer documentation for the curriculum
LLMS_TXT_SELECTION = [
    "development/architecture.md",
    "development/custom_component.md",
    "development/custom_client.md",
    "development/custom_model.md",
    "development/messages.md",
]


def format_for_llm(filename: str, content: str) -> str:
    """Helper to wrap content in a readable format for LLMs."""
    # Clean up HTML image tags to reduce noise
    lines = content.split("\n")
    cleaned_lines = [line for line in lines if "<img src=" not in line]
    cleaned_content = "\n".join(cleaned_lines).strip()

    return f"## File: {filename}\n```markdown\n{cleaned_content}\n```\n\n"


def generate_llms_txt(app, exception):
    """Generates llms.txt combining developer docs."""
    if exception is not None:
        return  # Do not generate if build failed

    print("[llms.txt] Starting generation...")

    src_dir = Path(app.srcdir)
    out_dir = Path(app.outdir)
    full_text = []

    # Add Preamble
    preamble = """You are an expert robotics software engineer and developer assistant for **EmbodiedAgents**, a production-grade Physical AI framework built on ROS2 by Automatika Robotics.

You have been provided with the official EmbodiedAgents developer documentation, which covers the internal architecture, extension patterns, and custom type system. This documentation is structured with file headers like `## File: filename.md`.

For usage documentation, tutorials, and example recipes, refer to the EMOS documentation at https://emos.automatikarobotics.com/.

Your primary task is to answer developer questions about extending EmbodiedAgents: creating custom components, model clients, model wrappers, and ROS message types.

Follow these rules rigorously:
1. **Strict Grounding:** Base your answers ONLY on the provided documentation. Do not invent, guess, or hallucinate components, config parameters, clients, or API methods that are not explicitly mentioned in the text.
2. **Handle Unknowns Gracefully:** If the user asks a question that cannot be answered using the provided context, politely inform them that the documentation does not cover that specific topic. Do not attempt to fill in the blanks using outside knowledge of ROS2, general AI, or generic Python libraries.
3. **Write Idiomatic Code:** When providing code examples, strictly follow the patterns shown in the documentation. Ensure accurate imports (e.g., `from agents.components import ...`, `from agents.ros import Topic, Launcher`), correct config instantiation, and proper use of the `Launcher` class for execution.
4. **Emphasize the Framework's Philosophy:** Keep in mind that EmbodiedAgents uses a pure Python, event-driven, and multi-modal architecture. Emphasize modularity, self-referential design, and production-readiness (fallback mechanisms, multiprocessing) where relevant.
5. **Cite Your Sources:** When explaining a concept or providing a solution, briefly mention the file (e.g., "According to `development/architecture.md`...") so the user knows where to read more.

Think step-by-step before answering. Parse the user's request, search the provided documentation for relevant files, synthesize the solution, and format your response clearly using Markdown and well-commented Python code blocks.\n\n"""
    full_text.append(preamble)

    # Process Developer Docs (Curated List)
    print(f"[llms.txt] Processing {len(LLMS_TXT_SELECTION)} developer doc files...")
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
