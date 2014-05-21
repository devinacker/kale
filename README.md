Kirby's Adventure Level Editor (KALE) 
=====================================
version 0.83b 

!(http://dl.dropboxusercontent.com/u/43107309/kale-v083.png)

This is an extremely temporary readme.

KALE is a level editor for the NES game Kirby's Adventure. It supports all known versions of the ROM and supports most level editing functionality.

Disclaimer: This is an incomplete tool based on incomplete information. As always, please back up your ROMs before using this program. If something bad happens to your files, please let me know so I can try to find and fix what went wrong.

The "Level" menu (or the equivalent toolbar buttons) can be used to switch between editing tiles, sprites (red boxes) and exits (blue boxes). In all three modes, click and drag to select, double click to edit, and press the Delete key (or Edit>Delete) to remove stuff. The usual menu items or shortcut keys can be used to cut/copy/paste tiles as well (which will be extended to sprites and tiles as well later on). Right-clicking when in sprite or exit mode will place a new one at the current cursor location.

Note: Currently, moving or editing sprites and exits doesn't result in a "dirty" editing state, and so changes to either one will be saved automatically when changing levels if no other changes were made. Actions involving sprites or exits currently aren't added to the undo/redo stack (this will be changed eventually).

Some other things about exits:
 - In order to work, an exit must be placed on a tile with behavior 0x28 (i.e. any door tile)
 - When using a warp star, there must be an exit within the level which determines where the warp star actually leads to. Ideally it should be the only exit in the level, or else the warp star may not go to the intended destination. (Currently there is no way to define custom warp star animations, as the game uses hardcoded animations based on the current world and stage number.)
 - The destination of an exit is specified as a screen number and coordinates within that screen. If the dimensions of a level are changed, exits pointing to that level may need to be updated to point to the correct screen numbers.
 
The level properties window (in the Level menu, or press Ctrl+P) can be used to resize levels, select graphics and palettes, change music, and more. The "tileset" setting determines the arrangements of 8x8 tiles and their properties, while the "tile graphics" setting determines which combinations of actual graphics are loaded. Previews of both tile and sprite graphics are presented as well.

That's it for now. Feel free to post feedback in whichever thread you got this from, or via email (d@revenant1.net), or on GitHub (https://github.com/devinacker/kale, where the latest development version can be found).
