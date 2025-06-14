# TODOs

1. Enable Alpha Blending for sprites. [Done]
2. Update orthogonal for camera and test it. [Done]
3. ReTest TGA implementation. [Done]
4. Test out texture height and width with vertex rendering. [Done]
5. Test out dynamic vertex updates. [Done]
6. Make a base Bitmap interface [Done]. 
7. Implement Multiple Texture support.
8. Reimplement slots handling behaviour. [Done]
9. Clean out implementation. [Done]


// Todos for 14th June

1. Create 3 types of Sprite
	i-> Sprite that act with world matrix (translation, scale and position with respect to the world); [Done]
    ii-> (alright have but refine) Sprite to render on screen must ignore other stuff like lighting and all.
    iii-> (need to separate) Sprite to render on the back of the screen must take into account lighting (could be enabled or disabled)
2. Sprite Animation System -> Create a system that can play animations.
3. Add Attach camera to feature so that camera always follows that sprite of guy.
4. Add Player Controller (camera should follow him) [additionally if debug is active then I should be able switch between controller - to free controller]
5. Animation State Design - should be able to create multiple states dynamically given configurations (can help later when finalizing hehe).
