#!/bin/bash
# REPORT-CARD >> features/mac-build-run.feature
set -euo pipefail

repo_dir="$(cd "$(dirname "$0")" && pwd)"
cd "$repo_dir"
if [[ "$(uname -s)" != Darwin ]]; then
    echo "This script requires macOS." >&2
    exit 1
fi

build_mac() {
    if [[ ! -f configure ]]; then
        autoreconf -i
    fi
    if [[ ! -f Makefile ]]; then
        ./configure
    fi
    make -j"${JOBS:-4}"
}

package_mac() {
    app_dir="$repo_dir/sys/macosx/Schism_Tracker.app"
    resources_dir="$app_dir/Contents/Resources"
    mkdir -p "$app_dir/Contents/MacOS" "$resources_dir"
    command cp "$repo_dir/schismtracker" "$app_dir/Contents/MacOS/schismtracker"

    # Dynamic backends search Resources; Homebrew is not in dyld's default path.
    brew_prefix="$(brew --prefix)"
    shopt -s nullglob
    for library in "$brew_prefix"/lib/libSDL*.dylib; do
        destination="$resources_dir/$(basename "$library")"
        # Homebrew libraries are read-only; allow repeated packaging.
        if [[ -f "$destination" ]]; then
            chmod u+w "$destination"
        fi
        command cp -L "$library" "$destination"
    done
    shopt -u nullglob
    if [[ ! -f "$resources_dir/libSDL3.0.dylib" && ! -f "$resources_dir/libSDL2-2.0.0.dylib" ]]; then
        echo "SDL runtime missing. Install it with: brew install sdl3" >&2
        exit 1
    fi
    # sdl2-compat loads this exact filename relative to its own library.
    if [[ -f "$resources_dir/libSDL3.0.dylib" ]]; then
        ln -sfn libSDL3.0.dylib "$resources_dir/libSDL3.dylib"
        ln -sfn libSDL3.0.dylib "$resources_dir/libSDL3-3.0.0.dylib"
    fi
    cmp "$repo_dir/schismtracker" "$app_dir/Contents/MacOS/schismtracker"
}

build_mac
package_mac
echo "Built: $app_dir"
if [[ "${1:-}" != --no-run ]]; then
    open "$app_dir"
fi
