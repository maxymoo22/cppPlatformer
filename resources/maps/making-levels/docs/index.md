# Welcome to the documentation!

Look around in here if you want to learn how to make your own levels, and with cool features.

## Moving platforms

* Create a new polygon object. Make sure the type property is set to `mp`
* Properties:
* `tileGID` - the id of the tile to render on this platform.
* `direction` - An integer; `1` for horizontal, `2` for vertical and `3` for diagonal
* `centerX` - The x-coordinate value in pixels of the center of the tile. Needed for rendering it in the correct location
* `centerY` - Same as above but it's the y-coordinate instead
* `horizontalVelocity` - Horizontal velocity of the platform in m/s.
* `verticalVelocity` - Vertical velocity of the platform in m/s. If the platform moves diagonally, this value usually shouldn't be set to the same value
as `horizontalVelocity` because it will result in unexpected movement for the platform
* `usesButton` - A boolean. Check this property if you want the platform to be controlled by standing on a button
* `boundaryLeft` - The value in pixels that the left side of the platform shouldn't go beyond
* `boundaryRight` - Same as above, but for the right side
* `boundaryTop` - Same as above, but for the top of the platform
* `boundaryBottom` - Same as above, but for the bottom
* If the platform travels horizontally, only the left and right boundaries will be used. If the platform travles vertically, only the top and bottom boundaries will be used. A diagonally travelling platform will use all 4
* If any of the properties are not set or not set correctly, the platform will just be skipped when loading the map and will not be displayed.

## Buttons for moving platforms

* Create a new polygon object and makes sure the type is set to `button`
* Properties:
* `platformID` - The id of the platform this button controlls. You can find this ID by slecting the moving platform object with the sekect object tool, and then looking at the properties. it is the first builtin property. This number can't be bigger than `100000`.

## Commands

* `mkdocs new [dir-name]` - Create a new project.
* `mkdocs serve` - Start the live-reloading docs server.
* `mkdocs build` - Build the documentation site.
* `mkdocs -h` - Print help message and exit.

## Project layout

    mkdocs.yml    # The configuration file.
    docs/
        index.md  # The documentation homepage.
        ...       # Other markdown pages, images and other files.
