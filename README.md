# Project Zero

![init](img/0.png)

OpenGL project/scene for experimenting with various real-time rendering techniques.
The core of the project is implementation of the 
**Deferred Snow Deformation in Rise of the Tomb Raider** technique from book GPU 7 Pro.

The project also contains deferred shading pipeline, usage of SSBOs with atomic writes,
skybox and light volumes.

# Implementation of deferred snow deformation

Based on the technique used in the Rise of the
Tomb Raider

- Real-time game mechanic
- Compute shaders
- Mesh deformation
- Scalable complexity

## Before first render pass

- OpenGL
initialization
- Error-checking
methods!
- Camera
implementation
- Image loading
- Architecture -
namespaces

## Render passes


- Deformation and physics
- Snow filling
- Terrain
- Snow
- Spheres
- Lights
- Skybox

### Terrain

![terrain](img/terrain.png)

#### Options:

- Single dense mesh - demanding
- World chunks (with LOD) - complicated
- Single sparse mesh with tesselation


#### Height value:

- Generated from noise
- Heightmap


implemented sparse mesh with tesselation and
heightmap


### Spheres

![spheres](img/spheres.png)

#### Why?

- Easy to generate
- No animation during movement
- Later reused for light volumes

#### Implemented

- Procedurally generated mesh
- Instanced draw
- Data stored in SSBO
- Each sphere is a light source


### Deformation and physics

![fromfar](img/fromfar.png)


- Compute shader dispatch
- Deformation

- Atomic writes around the sphere location
- World to texture mapping


#### Physics

- Data in SSBO – speed, velocity, size(mass)
- Update of sphere position according to heightmap

### Snow

![snow](img/snow.png)


- Terrain lifted by certain amount
- Inherited tesselation
- Integration of deformation texture
- Texture to world mapping
- Lowering deformed points
- Computing the snow evaluation “at the edges”


#### Snow filling

- Compute shader dispatch
- Simple value addition for each pixel of deformation texture
- Except:
	- Map the camera position to the texture
	- Compute distance to this pixel as if it was in the
    middle
	- Create function that:
    	- Has huge value at the most distant pixels from camera in
       the shifted space
    	- Has almost zero value at the camera position


### Deferred Pipeline

![normals](img/normals.png)


#### G-Buffer:

- Position
- Normal
- Color

#### Implementation
- Forward geometry pass kept for debug purposes
- Terrain and snow:
	- Computing normals from tangent and bitangent


### Lights

![lights](img/lights.png)

Each sphere is a light source

#### Light volumes:
- Each sphere is expanded to light volume
- The size is computed from light attenuation equation

#### Render setup:
- The same geometry like in spheres render
- Detph check and write off
- Front-face culling enabled
- Blending enabled with add_one_one function


#### Implementaion

In vertex shader expand the light volumes

In fragment shader:
- Compute screen space coordinates
- Read data from G-Buffer
- Compute Blinn-Phong shading
- Do gamma correction

Problems:
- Large light volumes = demanding, low fps
- Small light volumes = very dark scene (no ambient)
- Gamma correction breaks the smooth attenuation
- Spheres are unlit – their faces are oriented away


#### Hacks

##### Adding ambient light:

- Another light volume centered on camera
- Set constant attenuation
- Switch off specular or set center above the scene

##### Unlit spheres:
- Set normals of spheres to (0,0,0)
- In light pass, check for this, then set specular to 1

##### Gamma correction:
- Check for light intensity after correction and decrease


### Skybox

Texture sampled by direction

#### Options:

##### Render first:
    - Depth must be 1.0
    - Propagated into g-color buffer
##### Render last:
    - Depth buffer is available
    - Check where depth = 1
    - Render sky, else discard fragment


![final](img/final.png)


# TODO

- Refactoring shaders – no copy-paste
- Add proper image loading
- Add material to the spheres
- Add spheres rotation
- Implement screen space ambient occlusion
- Add snowing
- Extend the terrain

# Tips & Warnings

- Keep consistent units
- Plan unit size ahead when working with integers
- Don’t use magic numbers in shaders
- Try to check every OpenGL call
- Use some GLSL preprocessor to avoid copypaste
- Use VBO
- To get texture write access, use glBindImageTexture amd image2D
instead of sampler
- In deferred renderer, create also specular buffer
- Keep geometry pass and G-Buffer passes usable, for debugging
- Structured SSBO – memory alignment!
- Don’t initialize camera to direction (0,-1,0) or similar
- OpenGL clamps values when rendering to GL_RGB type texture

