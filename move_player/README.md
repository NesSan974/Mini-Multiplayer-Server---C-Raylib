# Raylib-Quickstart
A simple cross platform template for setting up a project with the bleeding edge raylib code.
Works with C or C++.

## Supported Platforms
Quickstart supports the main 3 desktop platforms:
* Windows
* Linux
* MacOS

# VSCode Users (all platforms)
*Note* You must have a compiler toolchain installed in addition to vscode.

* Download the quickstart
* Rename the folder to your game name
* Open the folder in VSCode
* Run the build task ( CTRL+SHIFT+B or F5 )
* You are good to go

# Linux Users
* CD into the build folder
* run `./premake5 gmake`
* CD back to the root
* run `make`
* you are good to go

# Output files
The built code will be in the bin dir

# Building for other OpenGL targets
If you need to build for a different OpenGL version than the default (OpenGL 3.3) you can specify an OpenGL version in your premake command line. Just modify the bat file or add the following to your command line

## For OpenGL 1.1
`--graphics=opengl11`

## For OpenGL 2.1
`--graphics=opengl21`

## For OpenGL 4.3
`--graphics=opengl43`

## For OpenGLES 2.0
`--graphics=opengles2`

## For OpenGLES 3.0
`--graphics=opengles3`

# License
Copyright (c) 2020-2025 Jeffery Myers

This software is provided "as-is", without any express or implied warranty. In no event 
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you 
  wrote the original software. If you use this software in a product, an acknowledgment 
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
