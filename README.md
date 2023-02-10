
# General

This is just scatchpad app for following OpenGL tutorial at learnopengl.com.

# opengl GLAD setup

1) generae package
https://glad.dav1d.de

- include
  * GL_ARB_Bindless_Texture

2) Download

3) Unzip into vendor/glad

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

## Misc
- https://stackoverflow.com/questions/5532595/about-opengl-texture-coordinates

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
