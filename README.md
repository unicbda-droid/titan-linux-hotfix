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
# 1. Hotfix herunterladen
git clone https://github.com/unicbda-droid/titan-linux-hotfix.git
cd titan-linux-hotfix

# 2. Ausfuehren (automatische Erkennung)
./titan-linux-hotfix.sh

# Oder mit Pfad:
./titan-linux-hotfix.sh /pfad/zu/Titan
```

## Was der Fix macht

1. Ersetzt `-nopie` durch `-no-pie` in allen generierten Makefiles und XML-Konfigurationen
2. Entfernt veraltete Precompiled-Header-Dateien (bei Aenderungen an Headern)
3. Prueft auf fehlende Abhaengigkeiten (clang, libX11, etc.)

## Build nach dem Fix

```bash
cd Titan/Projects/_Build_/Application
make
```

Hinweis: Nach jedem Export eines neuen Projekts aus dem Titan Editor muss der Fix erneut ausgefuehrt werden.
