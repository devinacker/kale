Kirby's Adventure Level Editor (KALE)
version 0.81

This is an extremely temporary readme.

KALE is a level editor for the NES game Kirby's Adventure. It supports all known versions of the ROM and supports most level editing functionality.

The "Level" menu (or the equivalent toolbar buttons) can be used to switch between editing tiles, sprites (red boxes) and exits (blue boxes). In all three modes, click and drag to select, double click to edit, and press the Delete key (or Edit>Delete) to remove stuff. The usual menu items or shortcut keys can be used to cut/copy/paste tiles as well (which will be extended to sprites and tiles as well later on). Right-clicking when in sprite or exit mode will place a new one at the current cursor location.

(Note: Currently, moving or editing sprites and exits don't result in a "dirty" editing state (and so changes to either one will be saved automatically when changing levels if no other changes were made), and actions involving sprites or exits aren't added to the undo/redo stack. This will be changed eventually.)

Some other things about exits:
 - In order to work, an exit must be placed on a tile with behavior 0x28 (i.e. any door tile)
 - When using a warp star, there must be an exit on the level's highest-numbered screen (I think) which determines where the warp star actually leads to. (Currently there is no way to define custom warp star animations, and I am not sure how the game determines which animation to use, but this will be fleshed out in future versions of the editor, hopefully.)
 - The destination of an exit is specified as a screen number and coordinates within that screen. If the dimensions of a level are changed, exits pointing to that level may need to be updated to point to the correct screen numbers.
 
The level properties window (in the Level menu, or press Ctrl+P) can be used to resize levels, select graphics and palettes, change music, and more. The "tileset" setting determines the arrangements of 8x8 tiles and their properties, while the "tile graphics" setting determines which combinations of actual graphics are loaded. Previews of both tile and sprite graphics are presented as well.

That's it for now. Feel free to post feedback in whichever thread you got this from, or via email (d@revenant1.net), or on GitHub (https://github.com/devinacker/kale, where the latest development version can be found).