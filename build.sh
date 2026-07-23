#!/usr/bin/env bash
# Local build script for WasmHot.
#
# Pipeline: clang-format -> cmake (Ninja) -> build -> clang-tidy
#
# Debug builds enable AddressSanitizer and GDB-friendly flags (configured in
# CMakeLists.txt). Release / RelWithDebInfo options are also configured there.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

BUILD_TYPE="Release"
JOBS="$(nproc 2>/dev/null || echo 4)"
RUN_FORMAT=1
RUN_TIDY=1
FORMAT_CHECK=0
CLEAN=0
VERBOSE=0
FAST=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  -b, --build-type TYPE   Build type: Debug, Release or RelWithDebInfo (default: Release)
  -j, --jobs N            Number of parallel build jobs (default: auto)
  --clean                 Remove build directory before configuring
  --no-format             Skip clang-format
  --no-tidy               Skip clang-tidy
  --check                 Only check formatting, do not modify source files
  --fast                  Skip clang-format and clang-tidy (same as --no-format --no-tidy)
  --verbose               Verbose build output
  -h, --help              Show this help message

Examples:
  $(basename "$0") -b Release
  $(basename "$0") -b Debug --clean
  $(basename "$0") -b RelWithDebInfo
  $(basename "$0") --fast -b Release
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --no-format)
            RUN_FORMAT=0
            shift
            ;;
        --no-tidy)
            RUN_TIDY=0
            shift
            ;;
        --check)
            FORMAT_CHECK=1
            shift
            ;;
        --fast)
            FAST=1
            RUN_FORMAT=0
            RUN_TIDY=0
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "RelWithDebInfo" ]]; then
    echo "Error: build type must be Debug, Release or RelWithDebInfo (got '$BUILD_TYPE')" >&2
    exit 1
fi

# -----------------------------------------------------------------------------
# Tool detection
# -----------------------------------------------------------------------------
find_tool() {
    local primary="$1"
    shift
    for candidate in "$primary" "$@"; do
        if command -v "$candidate" >/dev/null 2>&1; then
            echo "$candidate"
            return 0
        fi
    done
    echo "Error: required tool '$primary' not found" >&2
    return 1
}

CMAKE="$(find_tool cmake)"
NINJA="$(find_tool ninja ninja-build)"

if [[ $RUN_FORMAT -eq 1 ]]; then
    CLANG_FORMAT="$(find_tool clang-format clang-format-21 clang-format-20 clang-format-19 clang-format-18)"
fi

if [[ $RUN_TIDY -eq 1 ]]; then
    CLANG_TIDY="$(find_tool clang-tidy clang-tidy-21 clang-tidy-20 clang-tidy-19 clang-tidy-18)"
fi

# -----------------------------------------------------------------------------
# Step 1: clang-format
# -----------------------------------------------------------------------------
if [[ $RUN_FORMAT -eq 1 ]]; then
    echo "==> Step 1/4: Running clang-format..."
    if [[ $FORMAT_CHECK -eq 1 ]]; then
        find "$PROJECT_ROOT/src" -type f \
            \( -name '*.cpp' -o -name '*.cc' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' \) \
            -print0 | xargs -0 "${CLANG_FORMAT}" --dry-run --Werror
        echo "clang-format check passed."
    else
        find "$PROJECT_ROOT/src" -type f \
            \( -name '*.cpp' -o -name '*.cc' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' \) \
            -print0 | xargs -0 "${CLANG_FORMAT}" -i
        echo "clang-format done."
    fi
else
    echo "==> Step 1/4: Skipping clang-format."
fi

# -----------------------------------------------------------------------------
# Step 2: CMake configure
# -----------------------------------------------------------------------------
if [[ $CLEAN -eq 1 ]]; then
    echo "==> Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

echo "==> Step 2/4: Configuring with CMake (Ninja, $BUILD_TYPE)..."
CMAKE_ARGS=(
    -S "$PROJECT_ROOT"
    -B "$BUILD_DIR"
    -G Ninja
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
)

"$CMAKE" "${CMAKE_ARGS[@]}"

# -----------------------------------------------------------------------------
# Step 3: Build
# -----------------------------------------------------------------------------
echo "==> Step 3/4: Building with $JOBS parallel job(s)..."
BUILD_ARGS=(--build "$BUILD_DIR" --parallel "$JOBS")
if [[ $VERBOSE -eq 1 ]]; then
    BUILD_ARGS+=(--verbose)
fi

BUILD_FAILED=0
if ! "$CMAKE" "${BUILD_ARGS[@]}"; then
    BUILD_FAILED=1
fi

# -----------------------------------------------------------------------------
# Step 4: clang-tidy (only if build succeeded)
# -----------------------------------------------------------------------------
if [[ $BUILD_FAILED -eq 1 ]]; then
    echo "Error: build failed, skipping clang-tidy" >&2
    exit 1
fi

if [[ $RUN_TIDY -eq 1 ]]; then
    echo "==> Step 4/4: Running clang-tidy..."

    if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
        echo "Error: compile_commands.json not found in $BUILD_DIR" >&2
        exit 1
    fi

    find "$PROJECT_ROOT/src" -type f \
        \( -name '*.cpp' -o -name '*.cc' -o -name '*.c' \) \
        -print0 | xargs -0 "${CLANG_TIDY}" -p "$BUILD_DIR"
    echo "clang-tidy done."
else
    echo "==> Step 4/4: Skipping clang-tidy."
fi

echo "==> Build complete: $BUILD_TYPE"
echo "    Binary: $BUILD_DIR/WasmHotServer"

