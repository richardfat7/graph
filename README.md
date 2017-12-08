Basic Requirements:
You should accomplish the following goals to get the basic points.

- [ ] Create 3 planets (A, B, C), 1 light source box (E) and 1 space vehicle (D), and place them properly. Keep the
aspect ratio of the scene correct when we change the window size with “glutReshapeFunc” function. You can
load any possible objects or images as your planets, vehicle.

- [X] Create a Skybox (H) as the background and it should rotate accordingly when you change viewpoint and look
around the Universe. No translation and zoom should happen for Skybox.

- [X] Generate an asteroid ring cloud (G) which contains at least 200 random floating rocks around planet C. Those
floating rocks should have random sizes and locations in a limited range.

- [X] Conduct single texture mapping and normal mapping for planet A; single texture mapping for planet B, space
vehicle D and every rock in G; Conduct multiple texture mapping for planet C. Textures used for A, B, C, D
and G should be different from each other. You can use any texture you like, even a chocolate or strawberry.

- [X] Basic light rendering (ambient, diffuse and specular) should be obviously observed on A, B, C, D and G.
Please properly set your lighting parameters for clear demonstration. Keyboard interaction is allowed for you
to tune parameters during demonstration.

- [X] For planet A and C, they should do self-rotation all the time. For planet B, it should move around A at a
designed orbit (circle or other forms as you like). Also, B should have a self-rotation at the same time, just
like what moon does against earth.

- [X] For light source E, it should move around the center of the universe. It could be better if you can make this
movement periodically, just like sunrise and sunset. Here, we represent and visualize the light source with a
box at the light position. Basically, only pure color is required for the light source.

- [X] For space vehicle D, it should basically move around A at a designed orbit.

- [X] For asteroid ring cloud G, all the floating rocks in it should move around planet C simultaneously.

- [X] For viewpoint switch, at least 3 distinctive viewpoints (such as –X, +X, +Y axis) should be provided as
choices so that we can explore your Universe from different aspects.

- [X] Fog effect. Fill the whole universe with fog. When we move around, we should be able to experience the

- [ ] Graphical User Interface. A GUI should be created with the GLUI Library (similar as the figure shows)
which includes the module:
(a) A spinner to control the speed of vehicle.
(b) A radio group to set viewpoint.
(c) A checkbox to turn on/off fog effect, a radio group to set fog color.

- [ ] For interaction:
a) Keyboard. Please set lower case ‘a’, ‘s’, ‘d’ as switch between 3 different viewpoints.
b) Real-time speed and orbit control. Please set the up and down arrow keys to control the speed of the
space vehicle. Left and right arrow keys to increase/decrease the orbit radius size of the space vehicle.
c) Mouse. Please make the setting so that we can zoom in/out when we use the middle wheel of mouse.
Control the position of camera by mouse (same as assignment 2) so that we can move around your
universe
different fog density and visibility, shown as figures. Fog color should be able to be changed in GUI part.




BONUS
- [X] 1. Add another visible light source. All basic light rendering results should then be changed according to the
summation property of the Phong Illumination Model. (4 points)
- [ ] 2. Collision Detection. Detect the collision between any two objects. Once the collision happens, disable the
rendering of one object to make it disappear. You can choose Landmark distance/Bounding
box/Bounding sphere. (Marking depends on the elegance of your algorithm. Full mark is 6 points)
- [ ] 3. Provide extra viewpoints on the space vehicle. By doing this, we can really enjoy our space travel and
have a close touch with the planet and floating rocks. (4 points)
- [ ] 4. Represent the real-time trajectory of space vehicle D with a string of stars (F). You need to record and
update several real-time positions of the vehicle and render stars on those positions. Shown as the
following figure. (5 points)
[X]5. Generate the asteroid ring cloud with more than 5000 floating rocks. Some basic generation method will
becomes un-renderable as so many repeated objects need to be rendered at the same time. So you are
encouraged to explore the amazing instanced rendering. Code of instancing will be checked during
 demonstration. (6 points)
- [ ] 6. Use more “advanced” texture mappings to make objects more realistic, such as shadow mapping
environment mapping, displacement mapping etc. (7 points)
- [ ] 7. Play background music with IrrKlang Library. (2 points)

index		texture   
---------------------------------------    
SB	skybox (vaoSB)	skybox  0    
1	earth (vao0)	earth 21   
			earth_normal 2    
1	moon (vao0)	moon 3   
1	planet (vao0)	planetT1 4   
			planetT2 5    
2	rock (vao1)	fat 6    
5	lightbox (vao4)	lightbox 7    
3	plane (vao2)                 helicopter 8     
4	starfy (vao3)	starfy 9    


