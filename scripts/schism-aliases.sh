# REPORT-CARD >> features/mac-build-run.feature
# Source this file from ~/.bash_profile to launch this checkout by either name.
schism_repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
printf -v schism_alias_command '%q' "$schism_repo_dir/bin/schism"
alias schism="$schism_alias_command"
alias schismtracker="$schism_alias_command"
unset schism_repo_dir schism_alias_command
