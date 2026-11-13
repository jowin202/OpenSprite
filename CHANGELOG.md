# Changelog

## Version 2.0 (11/2026)

A large internal overhaul: faster, more robust, and with a few new features.
Everything that worked in 1.95 still works. Existing `.spd`, `.spm`, `.prg` and `.bin` files load as before, and saved/exported files are byte-identical to 1.95.

### New features

- **Redo** — `Ctrl+Y` or `Ctrl+Shift+Z` (Edit menu).
- **Paste Into** (`Ctrl+Shift+V`) — draws the non-transparent pixels of the copied sprite(s) over the selected sprites. Previously this showed "Not implemented yet".
- **More actions can be undone** — changing the sprite color, background/multicolor colors, overlay color, multicolor mode, overlay mode and X/Y expansion can now be undone.
- **No empty undo steps** — a click that changes nothing no longer creates an undo step.
- **Window title** shows the current file name and marks unsaved changes.
- **Quick Editor** also shows the overlay sprite.
- **Flood fill with the overlay tools** fills the overlay sprite (the next sprite).
- **Scale Dialog** is now also in the Edit menu (before, it was only in the toolbar).
- **Open dialog** lists all supported formats: SpritePad (`.spd`), Spritemate (`.spm`), `.prg` and `.bin`.
- **Safe saving** — files are written atomically, so a crash or full disk can no longer leave a half-written file. Write errors are reported.
- **Export dialog** remembers the last format and attribute settings, and stays open if you cancel the file selection.

### Performance

- Drawing sprites is about **5× faster**.
- Slide, flip, reflect and color-swap operations are about **80× faster** on large selections.
- Scaling preview computes about 9× less data with identical results.

### Bug fixes

**Files**
- Empty, truncated or invalid files crashed the program. Now an error is shown and the current project stays open. Truncated `.spd` files load the sprites that are complete.
- SpritePad v1 files with more than 128 sprites were read incorrectly.
- Projects without animations (e.g. after deleting all animations, or imported from `.prg`/`.bin`/`.spm`) were saved as broken `.spd` files.
- Projects with more than 256 sprites were silently saved as broken `.spd` files. Saving is now refused with a hint to use Export (PRG/BIN) instead.
- A `.bin`/`.prg` file whose last sprite has no attribute byte could crash the program.
- `.spm` files with certain multicolor pixel layouts produced damaged sprites.
- File extensions are now recognized regardless of upper/lower case (e.g. `.SPM`).

**Saving changes**
- Answering "Yes" to "Save changes?" and then cancelling "Save As" discarded the changes anyway. Now the action is cancelled.
- Dropping a file onto the window outside of the sprite area opened it without asking to save changes.

**Editing**
- Just selecting a sprite with overlay enabled could modify the next sprite ("Force single color"), which made the project appear changed.
- Flood fill kept painting while the mouse was dragged.
- The Quick Editor painted the wrong pixels on expanded sprites.
- Clicking exactly on the right or bottom edge of the palette could select an invalid color and crash later.
- Deleting all sprites left an empty project that could crash. A project now always keeps at least one sprite.
- After deleting sprites, the color and mode controls showed the properties of the wrong sprite.
- After undo, the color selection boxes could show outdated values.

**Dialogs**
- Rotation and scaling crashed when used on a sprite with overlay enabled that had no next sprite.
- Rotation and scaling inserted the result at the wrong position if the selection changed while the dialog was open.
- Deleting an animation in the Animations Editor could crash the program.
- The delete button of a newly added animation did not remove it from the list.
- "Selection to Animation" did not show the new animation if the Animations Editor was already open.
- An animation consisting only of overlay sprites crashed the preview.
- Expanded sprites in the animation preview showed random pixels in the unused area.
- Cancelling a color selection in the Settings set the color to black.
- Dialogs were never freed from memory. They now also close together with the main window.

**Display**
- Expanded sprites were drawn 1 pixel outside of their area, which could leave drawing artifacts.

### Changed behavior

- **Paste** now selects the pasted sprites.
- **Undo history** is limited to 1000 steps.
- **Delete Sprite** with multiple selected sprites now selects a single sprite at that position.

### Internal

- New module `spritedata` holds the sprite model, binary encoding and a single shared renderer. It replaces five copies of the drawing code.
- The rotation and scaling dialogs share their preview and insert logic in `seriesutil`.
- Saving, change detection and selection handling in the main window were combined into shared helper functions.
- The version number is defined once in `OpenSprite.pro`.
- Requires C++17, which all supported Qt 6 compilers provide.
- Builds without compiler warnings.
- The code size shrank by about 1100 lines.
