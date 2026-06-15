#!/usr/bin/env python3
import re
import argparse
import subprocess
from pathlib import Path


def get_commit_id():
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, check=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return ""


def main():
    parser = argparse.ArgumentParser(description="Bump version in about.hpp")
    parser.add_argument("file", help="Path to about.hpp")
    parser.add_argument("--bump-minor", action="store_true",
                        help="Increment minor version and reset patch to 0")
    args = parser.parse_args()

    path = Path(args.file)
    content = path.read_text()

    major = int(re.search(r"#define SIMPLEX_ABOUT_VERSION_MAJOR\s+(\d+)", content).group(1))
    minor = int(re.search(r"#define SIMPLEX_ABOUT_VERSION_MINOR\s+(\d+)", content).group(1))
    patch = int(re.search(r"#define SIMPLEX_ABOUT_VERSION_PATCH\s+(\d+)", content).group(1))

    if args.bump_minor:
        minor += 1
        patch = 0
    else:
        patch += 1

    commit_id = get_commit_id()

    content = re.sub(
        r'(#define SIMPLEX_ABOUT_COMMIT_IDENTIFER\s+)"[^"]*"',
        f'\\1"{commit_id}"',
        content
    )
    content = re.sub(
        r"(#define SIMPLEX_ABOUT_VERSION_MAJOR\s+)\d+",
        f"\\g<1>{major}",
        content
    )
    content = re.sub(
        r"(#define SIMPLEX_ABOUT_VERSION_MINOR\s+)\d+",
        f"\\g<1>{minor}",
        content
    )
    content = re.sub(
        r"(#define SIMPLEX_ABOUT_VERSION_PATCH\s+)\d+",
        f"\\g<1>{patch}",
        content
    )

    path.write_text(content)
    print(f"Version: {major}.{minor}.{patch} ({commit_id})")


if __name__ == "__main__":
    main()
