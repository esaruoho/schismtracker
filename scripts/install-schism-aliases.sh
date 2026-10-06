#!/bin/bash
# REPORT-CARD >> features/mac-build-run.feature
set -euo pipefail
repo_dir="$(cd "$(dirname "$0")/.." && pwd)"
profile="${1:-$HOME/.bash_profile}"
printf -v source_line 'source %q' "$repo_dir/scripts/schism-aliases.sh"
touch "$profile"
if ! rg -Fqx "$source_line" "$profile"; then
    printf '\n# Schism Tracker fork launch shortcuts\n%s\n' "$source_line" >> "$profile"
fi
echo "Installed schism and schismtracker aliases in $profile"
echo "For this terminal: $source_line"
