
# General

This is just scatchpad app for following OpenGL tutorial at learnopengl.com.

# opengl GLAD setup

1) generae package
https://glad.dav1d.de

- include
  * GL_ARB_Bindless_Texture

2) Download

3) Unzip into vendor/glad

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

## UBO/SSBO alignment
- https://computergraphics.stackexchange.com/questions/5810/shader-storage-buffer-indexing-by-4-bytes-instead-of-3
- https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o

## Bilboard

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

## Scripting
- http://lua-users.org/wiki/ScopeTutorial
- https://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html
- https://sol2.readthedocs.io/en/latest/tutorial/functions.html

## SKybox
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

## Geometry shader
- https://www.informit.com/articles/article.aspx?p=2120983&seqNum=2
- https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)
  + pass "gl_ClipDistance[CLIP_COUNT]" to pass through gs shader

## Misc
- https://stackoverflow.com/questions/5532595/about-opengl-texture-coordinates
- https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
- http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
- https://learnopengl.com/Lighting/Basic-Lighting
- http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
- https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
- https://community.khronos.org/t/sampler-array-limit-with-bindless-textures/73856/2
- https://www.khronos.org/opengl/wiki/Early_Fragment_Test
- https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
- https://iquilezles.org/articles/distfunctions/

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
