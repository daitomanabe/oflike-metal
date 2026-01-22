#!/usr/bin/env python3
"""
oflike Project Generator

Usage:
  python projectGenerator.py <project_name> [output_path]

Examples:
  python projectGenerator.py mySketch
  python projectGenerator.py mySketch ~/Projects
"""

import os
import sys
import shutil
from pathlib import Path

# Get the directory where this script is located
SCRIPT_DIR = Path(__file__).parent.resolve()
OFLIKE_PATH = SCRIPT_DIR.parent.resolve()
TEMPLATES_DIR = SCRIPT_DIR / "templates"


def create_project(project_name: str, output_path: Path) -> None:
    """Create a new oflike project."""

    project_dir = output_path / project_name

    # Check if project already exists
    if project_dir.exists():
        print(f"Error: Directory '{project_dir}' already exists.")
        sys.exit(1)

    print(f"Creating project: {project_name}")
    print(f"Location: {project_dir}")
    print(f"oflike path: {OFLIKE_PATH}")
    print()

    # Create directory structure
    (project_dir / "src").mkdir(parents=True)
    (project_dir / "bin").mkdir(parents=True)
    (project_dir / "build").mkdir(parents=True)

    # Copy main.cpp template
    main_template = TEMPLATES_DIR / "main.cpp"
    main_dest = project_dir / "src" / "main.cpp"
    shutil.copy(main_template, main_dest)
    print(f"  Created: src/main.cpp")

    # Generate CMakeLists.txt from template
    cmake_template = TEMPLATES_DIR / "CMakeLists.txt.template"
    cmake_dest = project_dir / "CMakeLists.txt"

    with open(cmake_template, 'r') as f:
        cmake_content = f.read()

    # Replace placeholders
    cmake_content = cmake_content.replace("{{PROJECT_NAME}}", project_name)
    cmake_content = cmake_content.replace("{{OFLIKE_PATH}}", str(OFLIKE_PATH))

    with open(cmake_dest, 'w') as f:
        f.write(cmake_content)
    print(f"  Created: CMakeLists.txt")

    # Create a simple build script
    build_script = project_dir / "build.sh"
    build_content = f"""#!/bin/bash
cd "$(dirname "$0")"
mkdir -p build
cd build
cmake ..
make -j4
"""
    with open(build_script, 'w') as f:
        f.write(build_content)
    os.chmod(build_script, 0o755)
    print(f"  Created: build.sh")

    # Create run script
    run_script = project_dir / "run.sh"
    run_content = f"""#!/bin/bash
cd "$(dirname "$0")"
./build.sh && open "build/{project_name}.app"
"""
    with open(run_script, 'w') as f:
        f.write(run_content)
    os.chmod(run_script, 0o755)
    print(f"  Created: run.sh")

    print()
    print("=" * 50)
    print(f"Project '{project_name}' created successfully!")
    print()
    print("To build and run:")
    print(f"  cd \"{project_dir}\"")
    print(f"  ./run.sh")
    print()
    print("Or manually:")
    print(f"  cd \"{project_dir}/build\"")
    print(f"  cmake ..")
    print(f"  make")
    print(f"  open {project_name}.app")
    print("=" * 50)


def print_usage():
    print(__doc__)


def main():
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)

    if sys.argv[1] in ['-h', '--help']:
        print_usage()
        sys.exit(0)

    project_name = sys.argv[1]

    # Validate project name
    if not project_name.replace('_', '').replace('-', '').isalnum():
        print(f"Error: Invalid project name '{project_name}'")
        print("Project name should only contain letters, numbers, underscores, and hyphens.")
        sys.exit(1)

    # Determine output path
    if len(sys.argv) >= 3:
        output_path = Path(sys.argv[2]).resolve()
    else:
        output_path = Path.cwd()

    # Verify templates exist
    if not TEMPLATES_DIR.exists():
        print(f"Error: Templates directory not found: {TEMPLATES_DIR}")
        sys.exit(1)

    create_project(project_name, output_path)


if __name__ == "__main__":
    main()
