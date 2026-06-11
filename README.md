# Titan Engine Linux Hotfix

Behebt den `-nopie` Linker-Fehler unter Linux, der mit modernen clang/gcc Versionen auftritt:

```
clang++: error: unsupported option -nopie for target x86_64-pc-linux-gnu
```

## Ursache

Die Titan Engine generiert NetBeans-Projekte mit dem veralteten Flag `-nopie`.
Moderne Compiler (Clang >= 12, GCC >= 10) verwenden stattdessen `-no-pie`.

## Installation

```bash
# 1. Repo klonen
git clone https://github.com/unicbda-droid/titan-linux-hotfix.git
cd titan-linux-hotfix

# 2. Kompilieren (optional, fertiges Binary liegt bei)
gcc -std=c99 -O2 -o titan-linux-hotfix titan-linux-hotfix.c

# 3. Ausfuehren (automatische Suche nach nbproject)
./titan-linux-hotfix

# Oder mit Pfad:
./titan-linux-hotfix /pfad/zu/Titan
```

## Was der Fix macht

1. Ersetzt `-nopie` durch `-no-pie` in allen generierten Makefiles und XML-Konfigurationen
2. Entfernt veraltete Precompiled-Header-Dateien (`.pch`, `.gch`)

## Build nach dem Fix

```bash
cd Titan/Projects/_Build_/Application
make
```

Hinweis: Nach jedem Export eines neuen Projekts aus dem Titan Editor muss der Fix erneut ausgefuehrt werden.
