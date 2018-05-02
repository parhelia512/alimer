# Alimer

[![Build status](https://ci.appveyor.com/api/projects/status/v0poctokc7r2xu24?svg=true)](https://ci.appveyor.com/project/amerkoleci/alimer)
[![Contribute!](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/amerkoleci/alimer/issues)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/amerkoleci/alimer/blob/master/LICENSE)

**Alimer** is a cross platform 2D and 3D game engine implemented in C++11, forked from [Turso3D](https://github.com/cadaver/turso3d).

## Goals/Planned Features

- Modern graphics/rendering API using Vulkan/Direct3D12.
- Extendible Asset Pipeline.
- GLTF 2.0 support.
- .NET Core/Standard, Mono/Xamarin scripting support.

## Credits

Alimer is fork from [Turso3D](https://github.com/cadaver/turso3d), which is licensed under the [MIT](https://github.com/cadaver/turso3d/blob/master/License.txt).

Alimer development, contributions and bugfixes by:

- Amer Koleci

The engine uses the following third-party libraries:

- [SDL 2.0.8](https://www.libsdl.org)
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [JSON for Modern C++ 3.1.2](https://github.com/nlohmann/json)
- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [Nothings](https://github.com/nothings/stb) single file libs.
  - [stb_image.h 2.19](https://github.com/nothings/stb/blob/master/stb_image.h)
  - [stb_image_write.h 1.09](https://github.com/nothings/stb/blob/master/stb_image_write.h)
  - [stb_rect_pack.h 0.11](https://github.com/nothings/stb/blob/master/stb_rect_pack.h)
  - [stb_truetype.h 1.19](https://github.com/nothings/stb/blob/master/stb_truetype.h)
  - [stb_vorbis.c 1.14](https://github.com/nothings/stb/blob/master/stb_vorbis.c)
  - [stb_textedit.h 1.12](https://github.com/nothings/stb/blob/master/stb_textedit.h)
- [volk Metaloader for Vulkan](https://github.com/zeux/volk)

Additional inspiration & research used:

- Vulkan examples (https://github.com/SaschaWillems/Vulkan).