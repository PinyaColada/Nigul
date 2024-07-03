# Nigul
<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h3 align="center">Nigul Graphics Engine</h3>

  <p align="center">
    A powerful graphics engine built from scratch in C++ using OpenGL.
    <br />
    <a href="https://github.com/yourusername/Nigul"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/yourusername/Nigul/tree/master/Source">View Source</a>
    ·
    <a href="https://github.com/yourusername/Nigul/blob/master/Source/main.cpp">View main.cpp</a>
    ·
    <a href="https://github.com/yourusername/Nigul/issues">Request Feature or Report Bug</a>
  </p>
</div>

<!-- Nigul -->
## Nigul

Nigul is a graphics engine written in C++ using OpenGL. This project was created to provide a flexible and high-performance environment for implementing and experimenting with various graphics techniques. By using OpenGL, Nigul achieves greater speed compared to Vulkan while allowing extensive control over the graphics pipeline.

Nigul utilizes the following libraries:
* **GLM** for mathematical operations
* **tinygltf** for reading and writing GLTF files
* **ImGui** and **ImGuizmo** for creating and modifying scenes with a GUI
* **stb** for parallel texture loading (GPU upload is not parallel)

This engine was developed to apply interesting graphics papers and to provide a high degree of control over the graphics pipeline.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Features

Nigul includes the following features:

### General Features
* GLTF file loading and writing
* Scene graph manipulation (modify nodes, transformations, parent-child relationships, delete nodes)
* Skybox loading

### Graphics Features
* Support for three types of lights
* Shadow mapping
* Physically Based Rendering (PBR)
* Reflections
* Screen Space Ambient Occlusion (SSAO+)
* Multisample Anti-Aliasing (MSAA)
* Chromatic aberration
* High Dynamic Range (HDR)
* Gamma correction

### In Development
* Irradiance

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Demo

You can see a showcase <a href="https://www.youtube.com/watch?v=m50P2QIQ_yI">here</a>!

<div align="center">
  <img src="https://github.com/yourusername/Nigul/blob/master/demo/GraphicsEngineExample.gif" alt="Graphics Engine Demo">
  <p>exploring the graphics capabilities!</p>
</div>

<p align="right">(<a href="#readme-top">back to top</a>)</p>
