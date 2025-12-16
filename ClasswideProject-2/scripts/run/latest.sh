#!/usr/bin/env bash
set -euo pipefail
# usage: latest.sh "glob"
ls -t $1 | head -n 1
