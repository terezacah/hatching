# Pencil Sketch Shader
The goal of this project was to implement a real-time "pencil sketch" renderer.

This project was developed as part of a university computer graphics course using an existing C++/OpenGL rendering framework provided by FI MUNI. Due to licensing restrictions of the original framework, the source code is not included in this repository.

---

## Initial State 
The original project folder already contained several files, providing a basic application structure. The initial scene included several 3D objects, a main camera, a light source, and UI controls.

---

## My Implementation
To produce the final effect, I extended the existing application by implementing a multi-pass rendering pipeline including:
- Preparation of FBOs and textures
- GLSL shaders for shadow mapping, diffuse rendering, SSAO evaluation and blur, Sobel edge detection, and procedural hatching
- Integration of all rendering passes into the application's rendering loop

The final image is generated using the following steps:
1. **Shadow Pass** – renders the scene from the light's point of view to create a shadow map.
2. **Diffuse Pass** – renders diffuse color, as well as depth, view-space positions, and view-space normals into the G-buffer.
3. **SSAO Pass** – computes ambient occlusion from the position and normal buffers.
4. **Blur Pass** – smooths the SSAO texture.
5. **Edge Detection Pass** – detects object outlines using a Sobel filter applied to the depth and normal textures.
6. **Final Pass** – combines diffuse lighting, shadows, ambient occlusion, edge detection, and procedural hatching to produce the final pencil sketch image.

---

## Technologies
- C++
- OpenGL

---

## Final Image
<img width="2046" height="1132" alt="image" src="https://github.com/user-attachments/assets/d5889c73-83a6-4bbf-8bc9-5a377be1246e" />



