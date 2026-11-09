#!/usr/bin/env python3
"""
check_console_tags.py
This script checks the tags that start the console messages of the engine: every message of
core, platform, and app starts with [TYPE::CLASS_NAME::function_name], with the three parts
always present (TYPE is ERROR, WARNING, INFO, or SUCCESS; CLASS_NAME is the class in capitals
with underscores, or the name of the file in capitals for a function that is not in a class;
function_name is the function that prints the message, with ~Name for a destructor).

Usage (from any folder of the repository):

    python tests/scripts/check_console_tags.py

It reads the tracked C++ files of app, core, and platform (Git is needed) and stops with an
error when a tag:
  - does not have the three parts, or does not have a TYPE of the four (for example [INFO::GUI],
    [WARNING::main], or [RENDERER::~Renderer]);
  - has a class part that is not the capitals of a class or struct that the file declares or
    defines, or of the name of the file (for example INPUTMANAGER in Input_Manager.cpp);
  - names a function that does not appear in the file as a function (a typo, or a function
    of another file).
It does not check that the function is the one that encloses the message: that needs a C++
parser. The lines that continue a message are not tags and are not checked.

Exit code: 0 when every tag follows the format, 1 otherwise, and 2 when it cannot read the files.
"""

import os
import re
import subprocess
import sys

REPOSITORY_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

SOURCE_FOLDERS = ("app", "core", "platform")
EXTENSIONS = (".cpp", ".h")
# third-party or generated code that does not print messages in the format
EXCLUDED_FILES = {"core/glad.c", "core/stb_image.cpp"}

# something that looks like a tag: [WORD::...] with a first part in capitals (a lowercase first part is C++ code)
CANDIDATE = re.compile(r"\[([A-Z][A-Z0-9_]*)((?:::[^\]\s\"\\]*)+)\]")
# the types without any class or function, for example [ERROR]
BARE_TYPE = re.compile(r"\[(?:ERROR|WARNING|INFO|SUCCESS)\]")
FULL_TAG = re.compile(r"^\[(ERROR|WARNING|INFO|SUCCESS)::([A-Z][A-Z0-9_]*)::(~?[A-Za-z_][A-Za-z0-9_]*)\]$")
DECLARED_CLASS = re.compile(r"\b(?:class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)")
DEFINED_METHOD = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)::~?[A-Za-z_][A-Za-z0-9_]*\s*\(")


def report_error(message):
    print(f"[ERROR::CONSOLE_TAGS::check] {message}", file=sys.stderr)


def get_files():
    # lists the tracked sources with Git, so that build folders and other generated files are never read
    try:
        output = subprocess.check_output(
            ["git", "-C", REPOSITORY_ROOT, "ls-files", "--", *SOURCE_FOLDERS], text=True, encoding="utf-8"
        )
    except (OSError, subprocess.CalledProcessError) as error:
        report_error(f"Cannot list the files with Git: {error}")
        return None
    files = [f for f in output.split("\n") if f.endswith(EXTENSIONS) and f not in EXCLUDED_FILES]
    return sorted(files)


def get_allowed_classes(text, path):
    # the capitals of the file name, and of the classes that the file declares or defines
    allowed = {os.path.splitext(os.path.basename(path))[0].upper()}
    allowed.update(name.upper() for name in DECLARED_CLASS.findall(text))
    allowed.update(name.upper() for name in DEFINED_METHOD.findall(text))
    return allowed


def check_file(path):
    errors = []
    tag_count = 0
    with open(os.path.join(REPOSITORY_ROOT, path), encoding="utf-8", errors="replace", newline="") as source:
        text = source.read()
    allowed = get_allowed_classes(text, path)
    # the text without its tags, to look for the names of the functions that the tags mention
    code = CANDIDATE.sub("", text)

    for number, line in enumerate(text.split("\n"), 1):
        for match in BARE_TYPE.finditer(line):
            tag_count += 1
            errors.append(f"{path}:{number}: {match.group(0)} has no class and no function: use [TYPE::CLASS_NAME::function_name]")
        for match in CANDIDATE.finditer(line):
            tag_count += 1
            tag = match.group(0)
            full = FULL_TAG.match(tag)
            if not full:
                errors.append(
                    f"{path}:{number}: {tag} does not follow [TYPE::CLASS_NAME::function_name] "
                    "(TYPE is ERROR, WARNING, INFO, or SUCCESS; the three parts are always present)"
                )
                continue
            class_part, function_part = full.group(2), full.group(3)
            if class_part not in allowed:
                errors.append(
                    f"{path}:{number}: {tag} has the class part {class_part}, which is not the capitals "
                    f"(with underscores) of a class of this file or of its name; expected one of: {', '.join(sorted(allowed))}"
                )
            name = function_part.lstrip("~")
            pattern = r"~\s*" + re.escape(name) + r"\s*\(" if function_part.startswith("~") else r"\b" + re.escape(name) + r"\s*\("
            if not re.search(pattern, code):
                errors.append(f"{path}:{number}: {tag} names {function_part}, which is not a function of this file")
    return tag_count, errors


def check():
    files = get_files()
    if files is None:
        return 2
    if not files:
        report_error("No source files found")
        return 2
    total_tags = 0
    all_errors = []
    for path in files:
        count, errors = check_file(path)
        total_tags += count
        all_errors.extend(errors)
    if all_errors:
        for error in all_errors:
            report_error(error)
        report_error(f"{len(all_errors)} problems found in {total_tags} tags in {len(files)} files")
        return 1
    print(f"[SUCCESS::CONSOLE_TAGS::check] {total_tags} tags in {len(files)} files, all in the format [TYPE::CLASS_NAME::function_name]")
    return 0


if __name__ == "__main__":
    sys.exit(check())
