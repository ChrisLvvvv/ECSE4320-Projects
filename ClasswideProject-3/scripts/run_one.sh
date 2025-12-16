#!/usr/bin/env bash
set -euo pipefail

job="$1"
out_json="$2"

: "${FIO_TARGET:?FIO_TARGET is not set. Refusing to run.}"
mkdir -p "$(dirname "$out_json")"

fio --output-format=json --output="$out_json" "$job"
