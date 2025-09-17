# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++ OpenGL graphics engine and renderer built as a learning project following the LearnOpenGL tutorial series. The project implements a comprehensive 3D rendering engine with advanced features including deferred shading, physically-based rendering (PBR), skeletal animation, physics simulation, particle systems, and scripting support.

## Build System and Commands

### Visual Studio Project
The project uses Visual Studio 2022 with MSBuild:
- Main project file: `learnopengl.vcxproj`
- Solution file: `learnopengl.sln`
- Target platform: x64
- Available configurations: Debug, Release, Build

### Running the Application
```bash
# Debug build
script/run_debug.cmd
# Release build  
script/run_release.cmd
```

### Asset Pipeline
The project includes a Ruby-based asset processing pipeline for texture compression:

```bash
# Setup asset pipeline
make setup

# Process all assets (default 1024px)
make all TARGET_SIZE=2048

# Process metadata only
make meta TARGET_SIZE=1024

# Build compressed textures
make build TARGET_SIZE=2048
```

Asset processing commands:
```bash
# Single asset processing
ruby script/encode_ktx.rb build --src resources/assets/textures/Metal022_1K-PNG --dry-run false --target-size 2048 --encode --combine --force true
```

### Dependencies
The project uses vcpkg for package management:
- vcpkg configuration: `vcpkg.json` and `vcpkg-configuration.json`
- Custom vcpkg registry support
- Key dependencies: OpenGL, GLFW, GLM, Assimp, OpenAL, Lua, fmt, EnTT

## Architecture Overview

### Core Engine Structure
- **Engine**: Base engine class (`src/engine/Engine.h`) providing the main game loop
- **SampleApp**: Main application implementation (`src/sample_app/SampleApp.h`) extending Engine
- **Registry**: Central registry system (`src/registry/Registry.h`) managing all engine subsystems
- **Scene**: Scene graph system for managing 3D objects and their hierarchies

### Key Subsystems

#### Rendering Pipeline
- **Deferred Rendering**: G-buffer based deferred shading pipeline
- **Forward Rendering**: For transparent objects and special materials  
- **Multi-pass System**: Organized rendering passes (depth pre-pass, G-buffer, lighting, post-processing)
- **Batch Rendering**: Efficient batched draw calls to reduce state changes
- **Level-of-Detail (LOD)**: Automatic LOD mesh switching based on distance

#### Graphics Features
- **PBR Materials**: Physically-based rendering with metallic-roughness workflow
- **Shadow Mapping**: Cascaded shadow maps (CSM) for directional lights
- **Screen Space Ambient Occlusion (SSAO)**: Enhanced ambient lighting
- **High Dynamic Range (HDR)**: HDR rendering with tone mapping
- **Bloom**: Post-processing bloom effect
- **Skybox**: Environment mapping and skybox rendering
- **Instanced Rendering**: Efficient rendering of multiple identical objects

#### Asset Management
- **Mesh Loading**: Assimp-based model loading supporting various formats (OBJ, FBX, etc.)
- **Texture System**: KTX texture compression and GPU texture management
- **Material System**: Comprehensive material management with custom materials
- **Asset Registry**: Centralized asset management and caching

#### Animation System
- **Skeletal Animation**: Full bone-based character animation
- **Animation Blending**: Support for animation state transitions
- **Socket System**: Attachment points for weapons/accessories

#### Physics Integration
- **ODE Physics**: Open Dynamics Engine integration
- **Collision Detection**: Comprehensive collision detection and response
- **Ray Casting**: Physics-based ray casting for interaction
- **Rigid Body Dynamics**: Full rigid body simulation

#### Scripting
- **Lua Integration**: Lua scripting for game logic and animations
- **Command System**: Command-based scripting interface
- **Coroutines**: Support for async script execution
- **Event System**: Publish-subscribe event handling

#### Additional Systems
- **Particle System**: GPU-accelerated particle effects
- **Audio System**: OpenAL-based 3D audio
- **Text Rendering**: Font atlas-based text rendering
- **Decal System**: Dynamic decal projection
- **Terrain**: Height-map based terrain rendering with tessellation
- **Navigation**: Recast/Detour navigation mesh generation
- **ImGui Integration**: Debug UI and editor interface

### File Organization
- `src/`: All source code organized by subsystem
- `shader/`: GLSL shaders for rendering pipeline
- `scene/`: YAML scene definitions and Lua scripts
- `resources/`: Assets, textures, models, audio files
- `script/`: Build and utility scripts

### Naming Conventions
- Class names: PascalCase (e.g., `RenderContext`, `NodeRegistry`)
- Member variables: m_ prefix (e.g., `m_registry`, `m_enabled`)
- Constants: ALL_CAPS with underscores
- Files: Match class names, headers (.h), implementations (.cpp)

## Development Workflow

### Scene System
Scenes are defined in YAML format (`scene/*.yml`) with support for:
- Node hierarchies and transformations
- Material assignments and custom materials
- Lighting setup (directional, point, spot lights)
- Physics bodies and collision shapes
- Particle generators and effects
- Lua script attachments

### Shader Development
- Modular shader system with include support
- Shared shader components in `shader/_*.glsl` files
- Render pass specific shaders (g_*.vs/fs for G-buffer pass)
- Compute shaders for GPU-driven rendering features

### Testing
The project includes test scenes in `scene/` directory for validating specific features:
- `scene_test.yml`: Basic rendering test
- `scene_physics.yml`: Physics simulation test
- `scene_animation.yml`: Skeletal animation test
- `scene_particle.yml`: Particle system test

### Debugging
The engine includes comprehensive debugging tools:
- OpenGL debug callbacks and error handling
- Performance profiling and FPS monitoring
- Render state visualization
- Physics debug rendering
- Memory usage tracking