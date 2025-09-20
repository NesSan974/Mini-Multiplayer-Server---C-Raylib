#!/bin/bash

set -e

echo "🔗 Compiling libcommon..."
./common/compile.sh
echo "✅ libcommon compiled"

echo "🎮 Compiling Raylib move_player ..."
cd move_player/build/
./premake5 gmake
cd ../
make
echo "✅ Complete"

echo "⚙️ Compiling server ..."
./server/compile.sh
echo "✅ ALL Build Finished"
