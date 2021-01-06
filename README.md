# Procudural Terrain Generation Project
### Written by Asher Farr for CSCI 4229 Intro to Computer Graphics
## Controls:
WASD to fly around
Arrow keys to move camera
Q/E to change the number of octaves of noise
[/] to change the scale of noise
1/2 to decrease/increase persistence of noise
3/4 to decrease/increase lacunarity of noise
;/' keys to decrease/increase the slope of noise
-/+ keys to decrease/increase the vertical shift of the noise
k to toggle scrolling terrain
Spacebar to seed a new world

## Instructions
This project requires GLFW, GLM, and Stb_image. GLFW should link if you have it installed.
To use GLM [download it](https://github.com/g-truc/glm) and add the glm folder to the project directory.
To use Stb_image [download stb_image.h](https://github.com/nothings/stb) and add it to the project directory.

To compile run make

In order to use the program, have fun playing with the parameters. I got lost making a bunch of combinations.
You could also just fly around the  world and enjoy watching the days pass by.

## What I'm proud of:

 - I built this using the OpenGL core profile which has a very steep learning curve compared to the classic openGL we were taught in class
 - My terrain generates instantly because the terrain geneation is built in a vertex shader
 - I implimented per pixel Phong shading and my normals are generated on the fly per pixel as well in the fragment shader
 - Because of the normals being generated in this way, my water appears to have depth even though it is completley flat
 - The water has another set of normals generated in the fragment shader which add a wavey texture using turbulent noise and it animates to look like "real" waves (this is my favorite feature)
 - I figured out how to to use std140 uniform buffer objects, which although passing data is not that impressive graphically, it took me so long so I'm pretty proud of it.

## Future plans
If I were to revisit this project, I would like to add procudural plants distributed throughout the world. I would also add instanced rendering with some sort of Level of Detail modification in order to make the world an infinite plane instead of the single square that currently exists. A skybox would probably be in order as well.