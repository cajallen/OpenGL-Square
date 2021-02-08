<a href="https://i.imgur.com/VzgdH13.mp4" title="Video of Functionality"><img src="https://i.imgur.com/A7nwmxQ.png" alt="Video" /></a>

## Compiling
This project is set up to compile with VS 2019. To set it up properly using this method, install `SDL2-2.0.14` in `C:/`, so that `C:/SDL2-2.0.14/include/` is the path to the headers. Otherwise, update the project settings to the SDL include and lib folders. 

## Operation
When the project is run, it is set up to have 3 different images. The following operations can be performed on the images:
 - Translated, by dragging the mouse relatively centered in the square, represented by the hand cursor.
 - Scaled, by dragging the mouse in the corners of the square, represented by the four directional arrow cursor.
 - Rotated, by dragging the mouse in the edges of the square, represented by a two directional diagonal arrow cursor.
 - Brightened or darkened, by scrolling up and down with the mousewheel.
 - Animated, toggled by the A key. (Warning: there are a couple niche bugs that can happen with this, it's not meant to be flawless)
 - Reset, with the R key.

To add custom images, use the `.ppm` file format with RGB plaintext (a lot of the `.ppm` files I've seen use some encoded values, but this program expects 0-255 plaintext). Then, in `Square.cpp`, create a new `Image` object with the relative path to the file as the first parameter. No more than 16 objects are supported at once, but there is no checking for this, so expect things to break if you do this. Additionally, you can change the window size by changing the `screen_height` and `screen_width` at the top of `Square.cpp`. Any aspect ratio should be supported.

## Difficulties encountered
There are a couple difficulties to talk about, but the main one is my method of doing multiple textures. The way that I do it feels incredibly wrong. I have my main `render()` loop, which calls `glClear()`, `SDL_GL_SwapWindow`, and all of that, but additionally inside of the function I loop through my `Image` objects and call their render methods. Within the `Image::render` method, I call `glUseProgram` on it's shader program, and set the uniforms associated with the texture unit and brightness value. This feels wrong because my shaders are unique per image, so having to reset the uniforms during every single image render call feels unnecessary. However, this was the method that made the program end up working of the many things I tried, and so for that I am happy with it for now.

The other relevant difficult I encountered was the coordinate system. Using [-1,1] coordinates for object logic worked easily for square aspect ratios, and was even relatively easy to fix the display with non-square aspect ratios. However, fixing the operations proved more difficult, specifically scale. As seen in the `.gif` above, the scale operation always aligns perfectly to the edge of the square, which is done by rotating the `mouse_pos` around the `rect_pos` by `rect_angle` and taking the Chebyshev distance between the two.
