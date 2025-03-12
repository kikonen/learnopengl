
# General

This is just scatchpad app for following OpenGL tutorial at learnopengl.com.

# SETUP

## Sub modules

```bash
git submodule init
git submodule update --init  --recursive
```
## Build assets

### Install ruby

Ruby+Devkit 3.4.1-2 (x64) (https://rubyinstaller.org)

```bash
$ cat ~/.irbrc
IRB.conf[:USE_AUTOCOMPLETE] = false
```

### Setup

```bash
bundle install
```

#### .rdbgrc
```
#
# https://github.com/ruby/debug
#

config set no_hint true
config set show_frames 0
config set use_short_path true
config set no_reline 1
```

#### .irbrc
```
#
# https://ruby-doc.org/3.4.1/stdlibs/irb/IRB.html#module-IRB-label-Configuration
#
# @see IRB.conf

IRB.conf[:USE_AUTOCOMPLETE] = false
#IRB.conf[:PROMPT_MODE] = :SIMPLE
#IRB.conf[:USE_PAGER] = false
#IRB.conf[:BACK_TRACE_LIMIT] = 0
#IRB.conf[:CONTEXT_MODE] = 0
#IRB.conf[:INSPECT_MODE] = false
#IRB.conf[:COMMAND_ALIASES] = {}
```


### prepare assets

```bash
ruby script/encode_ktx.rb meta --src resources/assets/textures/Metal022_1K-PNG --dry-run false --force true
```

### build assets

```bash
ruby script/encode_ktx.rb ktx --src resources/assets/textures/Metal022_1K-PNG --dry-run false --target-size 2048 --force true
```

## vcpkg-registry

### Clone vcpkg-registry
```bash
# into same level as learnopengl repo
cd $ROOT
git clone git@github.com:kikonen/vcpkg-registry.git
```
### Test vcpkg-registry
```powershell
# NOTE KI in *POWERSHELL** not bash
cd learnopengl
vcpkg install --feature-flags=registries,manifests
```

## opengl GLAD setup

1) generae package
https://glad.dav1d.de

- include
  * GL_ARB_Bindless_Texture

2) Download

3) Unzip into vendor/glad

## Vendor modules

### generator

For generating simple basic meshes
https://github.com/ilmola/generator


# TODO/Research
- "shell based grass and fur"
  + https://gim.studio/an-introduction-to-shell-based-fur-technique/
  + https://xbdev.net/directx3dx/specialX/Fur/index.php
  + https://www.researchgate.net/figure/Close-view-of-grass-rendered-with-16-shells_fig3_244493698
  + http://www.catalinzima.com/xna/tutorials/fur-rendering/
  + https://stackoverflow.com/questions/19862841/fur-shading-using-glsl
- ECS
  + https://skypjack.github.io/entt/md_docs_md_entity.html
  * https://habr.com/en/post/651921/


# Misc techniques

## OpenGL Specs
- https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html

## UBO/SSBO alignment
- https://computergraphics.stackexchange.com/questions/5810/shader-storage-buffer-indexing-by-4-bytes-instead-of-3
- https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o

## VBO buffer (interleaved or separate streams)
- https://www.reddit.com/r/vulkan/comments/rtpdvu/interleaved_vs_separate_vertex_buffers/
- https://www.reddit.com/r/opengl/comments/155jq8v/whats_better_multiple_vertex_buffers_or_a_single/

## Billboard

- https://ogldev.org/www/tutorial27/tutorial27.html
  * applied for "point sprite" billboard
- http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
  * applied for "quad" billboard
- https://gamedev.stackexchange.com/questions/113147/rotate-billboard-towards-camera
- https://stackoverflow.com/questions/18048925/opengl-billboard-matrix
- https://stackoverflow.com/questions/18048925/opengl-billboard-matrix
- https://www.mathsisfun.com/algebra/matrix-inverse.html
- https://mathinsight.org/matrix_transpose
- https://stackoverflow.com/questions/15325752/how-to-create-billboard-matrix-in-glm
- https://stackoverflow.com/questions/5467007/inverting-rotation-in-3d-to-make-an-object-always-face-the-camera
- https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  * applied for "quad" billboard
- https://www.flipcode.com/archives/Billboarding-Excerpt_From_iReal-Time_Renderingi_2E.shtml

## Frustum culling
- https://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/
- https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
- https://zeux.io/2009/01/31/view-frustum-culling-optimization-introduction/
- https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/
- https://gamedev.stackexchange.com/questions/73682/frustum-culling-based-on-mvp-matrix

## GPU frustum/occlusion culling
- https://cpp-rendering.io/indirect-rendering/
- https://registry.khronos.org/OpenGL/extensions/ARB/ARB_indirect_parameters.txt
- https://forum.unity.com/threads/gpu-frustum-culling-tips.1102627/
- https://bazhenovc.github.io/blog/post/gpu-driven-occlusion-culling-slides-lif/

## CPU frustum culling
 - Fast Extraction of Viewing Frustum Planes from the World- View-Projection Matrix
- http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
- https://www.reddit.com/r/gamedev/comments/5zatbm/frustum_culling_in_opengl_glew_c/
- https://donw.io/post/frustum-point-extraction/
- https://iquilezles.org/articles/frustum/
- https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
- https://stackoverflow.com/questions/8115352/glmperspective-explanation

## Compute shader
- https://computergraphics.stackexchange.com/questions/400/synchronizing-successive-opengl-compute-shader-invocations
- https://www.reddit.com/r/vulkan/comments/lh9cu1/do_compute_shaders_only_parallelize_up_to_local/

## GPU Texture Compression
- KTX software
  + https://github.com/KhronosGroup/KTX-Software
  + https://github.com/KhronosGroup/3D-Formats-Guidelines/blob/main/KTXDeveloperGuide.md
  + https://github.khronos.org/KTX-Software/libktx/index.html
  + https://github.khronos.org/KTX-Software/ktxtools/toktx.html
  + https://github.com/snowkit/ktx-format

**ktx albedo**
```bash
ktxinfo.exe Planks09_col_2048.ktx
```

**Compress**
```bash
toktx.exe --verbose --genmipmap --encode astc Planks09_col_2048.ktx Planks09_col_2048.png
```

## Scripting
- http://lua-users.org/wiki/ScopeTutorial
- https://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html
- https://sol2.readthedocs.io/en/latest/tutorial/functions.html
- https://github.com/ThePhD/sol2/issues/90
- https://blog.rubenwardy.com/2020/07/22/sol3-vector-extension/
- https://www.youtube.com/watch?v=1d8SbBIGvUY&list=PLtI3Y3gOLHgpqnRPhqRW4ahSaXePoILNn&index=1

## Skybox
- https://www.flipcode.com/archives/Skybox_With_A_Single_Quad.shtml
- https://www.rioki.org/2013/03/07/glsl-skybox.html

## Height map
- https://learnopengl.com/Guest-Articles/2021/Tessellation/Height-map
- https://stackoverflow.com/questions/41713631/what-is-the-correct-way-to-sample-a-16-bit-height-map-in-opengl

## Tessellation
- https://gamedev.stackexchange.com/questions/87616/opengl-quad-tessellation-control-shader
- https://ogldev.org/www/tutorial30/tutorial30.html
- https://yiweimao.github.io/blog/tessellation/
- https://www.khronos.org/opengl/wiki/Tessellation

## G-buffer
- https://learnopengl.com/Advanced-Lighting/Deferred-Shading
- https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
- https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
  * vec3 alignment *is* broken in opengl std140
- https://www.reddit.com/r/GraphicsProgramming/comments/zxrww8/should_i_have_tangents_in_my_gbuffer_in_a/
  * tangent space lighting typically happens in forward renderers
- normal encoding
  + https://gamedev.stackexchange.com/questions/162248/correctly-transforming-normals-for-g-buffer-in-deferred-rendering
  + http://c0de517e.blogspot.com/2015/01/notes-on-g-buffer-normal-encodings.html
    * example of g-buffer encoding
  +  https://github.khronos.org/KTX-Software/ktxtools/ktx_encode.html
  + https://gamedev.stackexchange.com/questions/203188/problems-with-normal-recovery-from-storage-as-two-floats-and-normal-from-depth-b
  + https://aras-p.info/texts/CompactNormalStorage.html
  * https://gamedev.stackexchange.com/questions/46304/how-does-gl-int-2-10-10-10-rev-work-for-color-data

## Skeletal animation
- https://github.com/assimp/assimp
- https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html
- https://realitymultiplied.wordpress.com/2016/04/12/assimp-skeletal-animation-tutorial-intro/
- https://xphere.me/2019/05/bones-animation-with-openglassimpglm/
- https://www.gamedev.net/forums/topic/688121-skeletal-animation-assimp-glm-and-me-in-between33/
- https://realitymultiplied.wordpress.com/2016/04/12/assimp-skeletal-animation-tutorial-intro/
- https://animcoding.com/post/animation-tech-intro-part-1-skinning
- https://vladh.net/game-engine-skeletal-animation/
- https://community.khronos.org/t/skeletal-animation-bones-for-models-or-bones-for-meshes/76044/2
- https://forum.babylonjs.com/t/how-weapon-is-attached-to-a-rigged-animation/17603/2
- https://blog.demofox.org/2012/09/21/anatomy-of-a-skeletal-animation-system-part-1/

## CSM
- https://ogldev.org/www/tutorial49/tutorial49.html
- https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
- https://learnopengl.com/Guest-Articles/2021/CSM
- https://stackoverflow.com/questions/28847767/cascaded-shadow-maps-not-quite-right
- https://stackoverflow.com/questions/40323029/cascaded-shadow-map-texture-access-bug-on-openglgl
- https://www.reddit.com/r/vulkan/comments/g0l8u5/sampler2darrayshadow_doesnt_work/

## Geometry shader
- https://www.informit.com/articles/article.aspx?p=2120983&seqNum=2
- https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)
  + pass "gl_ClipDistance[CLIP_COUNT]" to pass through gs shader

## Physics
- Open Dynamis Engine
  * https://ode.org/wiki/index.php
  * https://ode.org/wiki/index.php/Special:ShortPages

## Quaternions
- https://eater.net/quaternions
- https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
- http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
- https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
- https://gamedev.stackexchange.com/questions/149006/direction-vector-to-quaternion
- https://glm.g-truc.net/0.9.0/api/a00135.html
- https://miz-ar.info/glm-notes/gtc/quaternion.html
- https://www.3dgep.com/understanding-quaternions/
- https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another

## Audio
- OpenAL
  * https://www.openal.org
  * https://en.wikipedia.org/wiki/OpenAL
- OpenAL tutorial
  * https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
  * https://indiegamedev.net/2020/02/25/the-complete-guide-to-openal-with-c-part-2-streaming-audio/
  * https://indiegamedev.net/2020/04/12/the-complete-guide-to-openal-with-c-part-3-positioning-sounds/
- Video tutorial 2
  * https://www.youtube.com/watch?v=ptjIf_9CKGE&list=PLRIWtICgwaX2VNpAFjAZdlQw2pA1-5kU8&index=2
- Libaries
  * https://github.com/kcat/openal-soft
  * https://github.com/adamstark/AudioFile
  * https://github.com/mackron/dr_libs

## Misc
- https://stackoverflow.com/questions/5532595/about-opengl-texture-coordinates
- https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
- http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
- https://learnopengl.com/Lighting/Basic-Lighting
- http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
- https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
- https://community.khronos.org/t/sampler-array-limit-with-bindless-textures/73856/2
- https://www.khronos.org/opengl/wiki/Early_Fragment_Test
- https://www.khronos.org/opengl/wiki/Image_Format
- https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
- https://iquilezles.org/articles/distfunctions/
- http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

## C++
- https://en.cppreference.com/w/
- https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
- https://blog.andreiavram.ro/cpp-custom-allocators-first-step/
- https://en.cppreference.com/w/cpp/language/new
- https://medium.com/@yakupcengiz/comparing-std-map-and-std-unordered-map-in-c-92aa18c5dc39

## Visual C++
- https://preshing.com/20110711/visual-c-performance-pitfalls/
- __security_check_cookie
  + https://preshing.com/20110807/the-cost-of-buffer-security-checks-in-visual-c/
  + https://nfrechette.github.io/2020/06/26/buffer_security_checks/
  + https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/security-init-cookie
  + https://learn.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks?view=msvc-170

## SIMD
- https://gamedev.stackexchange.com/questions/132549/how-to-use-glm-simd-using-glm-version-0-9-8-2

## UUID
- https://github.com/mariusbancila/stduuid
- https://stackoverflow.com/questions/2351087/what-is-the-best-32bit-hash-function-for-short-strings-tag-names
- http://www.orthogonal.com.au/computers/hashstrings/
- https://github.com/google/crc32c

## Libraries
- https://github.com/nothings/stb
- https://github.com/wqking/eventpp

# References

- https://learnopengl.com
- https://glad.dav1d.de
- https://www.glfw.org/faq.html#11---what-is-glfw
- https://www.glfw.org/docs/latest/
- https://open.gl/drawing
- https://github.com/nothings/stb
- https://github.com/nothings/stb/blob/master/stb_image.h
- http://paulbourke.net/dataformats/obj/
- http://paulbourke.net/dataformats/mtl/
- http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
