#!/usr/bin/env bash
set -euo pipefail

out="$1"
mkdir -p "$(dirname "$out")"

{
  echo "=== Date ==="
  date -Is
  echo
  echo "=== OS / Kernel ==="
  uname -a
  echo
  echo "=== CPU ==="
  lscpu
  echo
  echo "=== Memory ==="
  free -h
  echo
  echo "=== Block devices ==="
  lsblk -o NAME,SIZE,MODEL,SERIAL,TYPE,ROTA,TRAN,MOUNTPOINT,FSTYPE
  echo
  echo "=== FIO ==="
  fio --version || true
  echo
  echo "=== Target ==="
  echo "FIO_TARGET=${FIO_TARGET:-<unset>}"
} > "$out"
