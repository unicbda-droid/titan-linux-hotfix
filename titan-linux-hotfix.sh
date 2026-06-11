#!/bin/bash
# Titan Engine Linux Hotfix
# Fixes -nopie -> -no-pie for modern clang/gcc compilers
# Usage: ./titan-linux-hotfix.sh [/pfad/zu/titan]

set -e

if [ -n "$1" ]; then
  TITAN="$1"
else
  # Auto-detect common locations
  for dir in "$HOME/Titan" "/usr/local/Titan" "/opt/Titan"; do
    if [ -f "$dir/Titan Editor" ] || [ -f "$dir/Titan Editor.exe" ]; then
      TITAN="$dir"
      break
    fi
  done
fi

if [ -z "$TITAN" ] || [ ! -d "$TITAN" ]; then
  echo "Fehler: Titan-Verzeichnis nicht gefunden."
  echo "Usage: $0 /pfad/zu/Titan"
  exit 1
fi

echo "=== Titan Engine Linux Hotfix ==="
echo "Titan-Verzeichnis: $TITAN"
echo ""

FIXES=0

# 1. -nopie -> -no-pie in allen generierten Build-Dateien
echo "[1/3] Fixe Linker-Flag -nopie -> -no-pie..."
FOUND=$(grep -rl --include="Makefile-*.mk" --include="configurations.xml" "\-nopie" "$TITAN/Projects/_Build_/" 2>/dev/null | wc -l)
if [ "$FOUND" -gt 0 ]; then
  find "$TITAN/Projects/_Build_/" \( -name "Makefile-*.mk" -o -name "configurations.xml" \) -exec grep -l "\-nopie" {} \; 2>/dev/null | while read f; do
    sed -i 's/-nopie/-no-pie/g' "$f"
    echo "  Gefixt: $f"
    FIXES=$((FIXES + 1))
  done
else
  echo "  Keine -nopie-Flags gefunden. (bereits gefixt?)"
fi

# 2. Entferne stale Precompiled Header
echo "[2/3] Entferne veraltete Precompiled Header..."
STALE=$(find "$TITAN/Projects/_Build_/" -name "stdafx.h.pch" 2>/dev/null | wc -l)
if [ "$STALE" -gt 0 ]; then
  find "$TITAN/Projects/_Build_/" -name "stdafx.h.pch" -delete 2>/dev/null
  echo "  $STALE .pch-Dateien entfernt."
else
  echo "  Keine veralteten PCH-Dateien gefunden."
fi

# 3. Dependencies check
echo "[3/3] Pruefe Abhaengigkeiten..."
MISSING=0
for pkg in clang make libxmu-dev libxi-dev libxinerama-dev libxrandr-dev libxcursor-dev libudev-dev libopenal-dev libgl1-mesa-dev libxxf86vm-dev zlib1g-dev; do
  if ! dpkg -s "$pkg" &>/dev/null 2>&1; then
    echo "  FEHLT: $pkg"
    MISSING=$((MISSING + 1))
  fi
done

# Check libodbc
if ! ldconfig -p | grep -q libodbc.so 2>/dev/null; then
  echo "  FEHLT: libodbc (unixodbc)"
  MISSING=$((MISSING + 1))
fi

if [ "$MISSING" -gt 0 ]; then
  echo ""
  echo "  Installiere fehlende Pakete mit:"
  echo "  sudo apt-get install clang make libxmu-dev libxi-dev libxinerama-dev \\"
  echo "       libxrandr-dev libxcursor-dev libudev-dev libopenal-dev \\"
  echo "       libgl1-mesa-dev libxxf86vm-dev zlib1g-dev unixodbc"
else
  echo "  Alle Abhaengigkeiten OK."
fi

echo ""
echo "=== Hotfix abgeschlossen ==="
echo "Starte den Build mit:"
echo "  cd \"$TITAN/Projects/_Build_/Application\" && make"
