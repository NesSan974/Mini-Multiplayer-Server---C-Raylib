#!/bin/bash


# arrête le script si une commande échoue
set -e

# Se place dans le dossier du script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Chemin du dossier bin à créer
BIN_DIR="$SCRIPT_DIR/bin"
INCLUDE_DIR="$SCRIPT_DIR/include"
SRC_DIR="$SCRIPT_DIR/src"

COMMON_DIR="$SCRIPT_DIR/../common"
COMMON_INCLUDE_DIR="$COMMON_DIR/include"
COMMON_LIB_DIR="$COMMON_DIR/lib"
# COMMON_LIB_NAME="common"

mkdir -p "$BIN_DIR"

echo "compiling.."

gcc "$SRC_DIR"/*.c -o "$BIN_DIR/server" -L"$COMMON_LIB_DIR" -lcommon -I"$INCLUDE_DIR" -I"$COMMON_INCLUDE_DIR" 


echo "✅ Build success: $BIN_DIR/server"