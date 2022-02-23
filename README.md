# OpenGLSetup

Description:

Move your robot left and right to dodge the enemy robots' projectiles while you use your own to destroy them.

The enemy pathing UI shown in img1.png and img2.png is used to dictate the path the enemmy will take. You can add and delete paths nodes on with the middle mouse button, use the middle mouse button on a node and it dissapears, use it on an empty space and it creates a new one.

The direction the bot was facing is always in the direction it is moving in, this was done using 2D vector cross product.

The direction the enemy gun is facing is based on where the player robot is, so we used the same function as the enemy bots movement but also had to subtract the angle of the gun to the player, which I calculated with some tan function.

Controls:

Move Left: Left Arrow
Move Right: Right Arrow
Start: a
Restart: r
Toggle First Person Mode: f
Shoot: Spacebar
