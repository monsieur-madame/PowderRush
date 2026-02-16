#!/bin/bash
# Build, cook, stage and deploy PowderRush to a connected iOS device
# Double-click this file from Finder, or run from terminal: ./DeployIOS.command
#
# Usage:
#   ./DeployIOS.command              Full build + cook + deploy
#   ./DeployIOS.command --cook-only  Cook content only (no C++ build)
#   ./DeployIOS.command --no-build   Skip C++ build, cook + deploy

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
UPROJECT="$SCRIPT_DIR/PowderRush.uproject"

if [ ! -f "$UPROJECT" ]; then
    echo "Error: PowderRush.uproject not found at $UPROJECT"
    exit 1
fi

# Read engine association from .uproject
ENGINE_VERSION=$(python3 -c "import json; print(json.load(open('$UPROJECT'))['EngineAssociation'])" 2>/dev/null || echo "")

if [ -z "$ENGINE_VERSION" ]; then
    echo "Error: Could not read EngineAssociation from .uproject"
    exit 1
fi

# Search common UE5 installation paths on macOS
SEARCH_PATHS=(
    "$HOME/Documents/Epic Games/UE_$ENGINE_VERSION"
    "$HOME/Documents/Unreal Engine/UE_$ENGINE_VERSION"
    "/Users/Shared/Epic Games/UE_$ENGINE_VERSION"
    "/Applications/Epic Games/UE_$ENGINE_VERSION"
    "/opt/UnrealEngine/UE_$ENGINE_VERSION"
)

ENGINE_DIR=""
for path in "${SEARCH_PATHS[@]}"; do
    if [ -d "$path/Engine" ]; then
        ENGINE_DIR="$path"
        break
    fi
done

# Allow override via environment variable
if [ -n "$UE_ROOT" ]; then
    ENGINE_DIR="$UE_ROOT"
fi

if [ -z "$ENGINE_DIR" ]; then
    echo "Error: Could not find UE $ENGINE_VERSION installation."
    exit 1
fi

UAT="$ENGINE_DIR/Engine/Build/BatchFiles/RunUAT.sh"

if [ ! -f "$UAT" ]; then
    echo "Error: RunUAT.sh not found at $UAT"
    exit 1
fi

echo "======================================"
echo "  PowderRush iOS Deploy"
echo "  Engine: UE $ENGINE_VERSION"
echo "======================================"
echo ""

# Parse arguments
BUILD_FLAG="-build"
COOK_ONLY=false

for arg in "$@"; do
    case $arg in
        --cook-only)
            COOK_ONLY=true
            BUILD_FLAG="-skipbuild"
            ;;
        --no-build)
            BUILD_FLAG="-skipbuild"
            ;;
    esac
done

if [ "$COOK_ONLY" = true ]; then
    echo "Cooking content only..."
    "$UAT" BuildCookRun \
        -project="$UPROJECT" \
        -platform=IOS \
        -configuration=Development \
        -skipbuild -cook -stage -pak \
        -compressed \
        -cmdline="" \
        -utf8output
else
    echo "Building, cooking, staging, and deploying..."
    "$UAT" BuildCookRun \
        -project="$UPROJECT" \
        -platform=IOS \
        -configuration=Development \
        $BUILD_FLAG -cook -stage -pak -deploy \
        -compressed \
        -cmdline="" \
        -device=IOS@all \
        -utf8output
fi

echo ""
echo "======================================"
echo "  Done!"
echo "======================================"
