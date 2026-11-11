#!/bin/bash
# Usage: convert-changelog-rpm.sh docs/changelog.md > changelog-section
# Emits an RPM %changelog body from docs/changelog.md (rpm counterpart of convert-changelog.sh).
set -e
maint="Nicholas Yoder <nicholasyoder@proton.me>"
first=1
while IFS= read -r line; do
  if [[ $line =~ ^"* Release "([0-9.]+)" - "([0-9]{4}-[0-9]{2}-[0-9]{2})$ ]]; then
    [[ $first -eq 0 ]] && echo
    first=0
    echo "* $(date -d "${BASH_REMATCH[2]}" '+%a %b %d %Y') $maint - ${BASH_REMATCH[1]}-1"
  elif [[ $line =~ ^"  - "(.+)$ ]]; then
    line="${BASH_REMATCH[1]}"; echo "- ${line//%/%%}"
  fi
done < "$1"
