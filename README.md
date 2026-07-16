# Pencil Sketch Shader
This project was developed as part of a university computer graphics course using an existing C++/OpenGL rendering framework provided by FI MUNI.
Since the original framework is not my work, this repository contains only the project folder with my imlementation. 

The goal of this project was to implement a real-time "pencil sketch" renderer.

---

## Initial State 
The original project folder already contained several files, including `application.hpp`, `application.cpp`, and `main.cpp`, providing a basic application structure, as well as forward and unlit rendering shaders. The initial scene included several 3D objects, a main camera, a light source, and UI controls.

<img width="2034" height="1150" alt="image" src="https://github.com/user-attachments/assets/002cbd64-6a94-45a5-9dbe-5cb4f9af4158" />

---

## My Implementation
To produce the final effect, I implemented a multi-pass rendering pipeline including:
- Preparation of FBOs and textures
- Implementation of multiple rendering passes
- GLSL shaders for shadow mapping, diffuse rendering, SSAO evaluation and blur, Sobel edge detection, and procedural hatching

The final image is generated using the following steps:
1. **Shadow Pass** – renders the scene from the light's point of view to create a shadow map.
2. **Diffuse Pass** – renders diffuse color, as well as depth, view-space positions, and view-space normals into the G-buffer.
3. **SSAO Pass** – computes ambient occlusion from the position and normal buffers.
4. **Blur Pass** – smooths the SSAO texture.
5. **Edge Detection Pass** – detects object outlines using a Sobel filter applied to the depth and normal textures.
6. **Final Composition Pass** – combines diffuse lighting, shadows, ambient occlusion, edge detection, and procedural hatching to produce the final pencil sketch image.

---

## Technologies
- C++
- OpenGL

---

## Screenshots
The following images show the different rendering passes used to produce the final pencil sketch effect.

### Diffuse Texture
<img width="2062" height="1070" alt="image" src="https://github.com/user-attachments/assets/d80218fd-c73c-4896-9b72-7fd0f5dd91cc" />

### Shadow Map
<img width="2200" height="1172" alt="image" src="https://github.com/user-attachments/assets/cb70724f-1650-4e9e-b6d9-0c04af306467" />

### Diffuse with Shadows
<img width="2032" height="1110" alt="image" src="https://github.com/user-attachments/assets/74f792bf-a659-4594-8f29-eaa3ac9900b7" />

### Positions in View Space
<img width="2010" height="1098" alt="image" src="https://github.com/user-attachments/assets/6f631e85-abee-466f-93fa-14512e85ff86" />

### Normals in View Space
<img width="2034" height="1084" alt="image" src="https://github.com/user-attachments/assets/8035cd99-1ce4-4129-9c29-9ca89e9e213f" />

### Depth
<img width="2028" height="1116" alt="image" src="https://github.com/user-attachments/assets/e2386456-48de-4d23-a023-7f3c144bf1b4" />

### Ambient Occlusion
<img width="2030" height="1074" alt="image" src="https://github.com/user-attachments/assets/f995b755-67e4-4029-911a-785298a6a670" />

### Edges
<img width="2056" height="1122" alt="image" src="https://github.com/user-attachments/assets/8291e537-3302-40cd-ab64-54f2c95a46f8" />

### Final Image
<img width="2046" height="1132" alt="image" src="https://github.com/user-attachments/assets/d5889c73-83a6-4bbf-8bc9-5a377be1246e" />



