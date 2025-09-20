
#!/bin/bash

echo "setting thing up"

# arrête le script si une commande échoue
set -e

# common/
# ├── include/         # headers publics (.h)
# ├── src/             # sources (.c)
# ├── build/           # fichiers objets (.o)
# └── lib/             # bibliothèques compilées (.a ou .so)

# Se place dans le dossier du script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

#Definition des dossiers utiles
SRC_DIR="$SCRIPT_DIR/src"
BUILD_DIR="$SCRIPT_DIR/build"
INCLUDE_DIR="$SCRIPT_DIR/include"
LIB_DIR="$SCRIPT_DIR/lib"
LIB_NAME="libcommon.a"

# Crée les dossiers s'ils n'existent pas
mkdir -p "$BUILD_DIR"
mkdir -p "$LIB_DIR"

#clean
rm -rf "$BUILD_DIR/*"
rm -f "$LIB_DIR/$LIB_NAME"

echo "compiling.."


# compile tous les .c en .o
for src_file in "$SRC_DIR"/*.c; do

    filename=$(basename "$src_file")

    objfilename="${filename%.c}.o"
    obj="$BUILD_DIR/$objfilename"

    gcc -c "$src_file" -I"$INCLUDE_DIR" -o "$obj"

done

# crée l'archive statique
ar rcs "$LIB_DIR/$LIB_NAME" "$BUILD_DIR"/*.o   

echo "✅ Build success: $LIB_DIR/$LIB_NAME"