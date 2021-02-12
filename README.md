(click for gif)
<a href="https://i.imgur.com/VzgdH13.mp4" title="Video of Functionality"><img src="https://i.imgur.com/A7nwmxQ.png" alt="Video" /></a>

## Compiling
This project is set up to compile with VS 2019. To set it up properly using this method, install `SDL2-2.0.14` in `C:/`, so that `C:/SDL2-2.0.14/include/` is the path to the headers. Otherwise, update the project settings to the SDL include and lib folders. 

## Operation
The program can be run by running in `x64/Debug/Project1.exe` on Windows.
When the project is run, it is set up to have 3 different images. The following operations can be performed on the images:
 - Translated, by dragging the mouse relatively centered in the square, represented by the hand cursor.
 - Scaled, by dragging the mouse in the corners of the square, represented by the four directional arrow cursor.
 - Rotated, by dragging the mouse in the edges of the square, represented by a two directional diagonal arrow cursor.
 - Brightened or darkened, by scrolling up and down with the mousewheel.
 - Animated, toggled by the A key. (Warning: there are a couple niche bugs that can happen with this, it's not meant to be flawless)
 - Reset, with the R key.

To add custom images, use the `.ppm` file format with RGB plaintext (a lot of the `.ppm` files I've seen use some encoded values, but this program expects 0-255 plaintext). Then, in `Square.cpp`, create a new `Image` object with the relative path to the file as the first parameter. No more than 16 objects are supported at once, but there is no checking for this, so expect things to break if you do this. Additionally, you can change the window size by changing the `screen_height` and `screen_width` at the top of `Square.cpp`. Any aspect ratio should be supported.

## Difficulties Encountered
There are a couple difficulties to talk about, but the main one is my method of doing multiple textures. At first, I could not figure out the OpenGL code to use multiple textures, as I didn't fully understand what VBOs, VAOs, and binding things was doing. I have my main `render()` loop, which calls `glClear()`, `SDL_GL_SwapWindow`, and all of that, but additionally inside of the function I loop through my `Image` objects and call their render methods. Within the `Image::render` method, I call `glUseProgram` on it's shader program, and I used to set the uniforms associated with the texture unit and brightness value. This was wrong because my shaders are unique per image, so having to reset the uniforms during every single image render call was unnecessary. Now, I am able to set the texture during Image construction, and in the Image render loop I use the shader program, bind vao, bind vbo, buffer the data, and draw. This is better than having to set all of the uniforms, but I do still have doubts of whether or not it is optimal.

The other relevant difficult I encountered was the coordinate system. Using [-1,1] coordinates for object logic worked easily for square aspect ratios, and was even relatively easy to fix the display with non-square aspect ratios. However, fixing the operations proved more difficult, specifically scale. As seen in the `.gif` above, the scale operation always aligns perfectly to the edge of the square, which is done by rotating the `mouse_pos` around the `rect_pos` by `rect_angle` and taking the Chebyshev distance between the two. Unfortunately, this was not something I could get working with the odd coordinate system that results from using stretched UV coordinates. I switched to pixel coordinates until the vertices array is set as a solution.

## Extra Features Added
I added `Graphical Indicator of what motion is being performed`(The mouse cursor changes), `Animating the square` (by pressing A), and `Multiple squares`. The aspect ratio can also be set to whatever, but it doesn't
support changing it during runtime.