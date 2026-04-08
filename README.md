# OpenSprite

[![Windows C++ Qt Build](https://github.com/jowin202/OpenSprite/actions/workflows/windows-build.yml/badge.svg)](https://github.com/jowin202/OpenSprite/actions/workflows/windows-build.yml)
[![Mac OS C++ Qt Build](https://github.com/jowin202/OpenSprite/actions/workflows/mac-build.yml/badge.svg)](https://github.com/jowin202/OpenSprite/actions/workflows/mac-build.yml)

An open-source sprite editor for creating Commodore 64 sprites.

This GPL3 project uses the open-source version of the [Qt Framework](https://download.qt.io).

![OpenSprite Main Window](docs/screenshots/main_window.png)

---

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Interface Overview](#interface-overview)
- [Editing Sprites](#editing-sprites)
- [Color Modes](#color-modes)
- [Transformations](#transformations)
- [Rotation](#rotation)
- [Scaling](#scaling)
- [Overlay Sprites](#overlay-sprites)
- [Animations](#animations)
- [Export & Import](#export--import)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Settings](#settings)
- [Building from Source](#building-from-source)
- [macOS First-Time Launch](#macos-first-time-launch)

---

## Features

- Pixel-level editor for 24×21 C64 sprites
- Single-color and multicolor sprite modes
- Overlay sprite support (two sprites rendered as one)
- Rotation and scaling dialogs with live preview
- Animation editor with ping-pong and overlay support
- Export to `.prg` and `.bin` with SpritePad / SEUCK attribute compatibility
- Import from `.spd`, `.prg`, `.bin`, and `.spm`
- Full undo, copy/paste, and sprite range selection
- Native C64 16-color palette

---

## Getting Started

1. Launch OpenSprite.
2. A new empty project with one sprite opens automatically.
3. Click **Add Sprite** (or press `Ins`) to add more sprites.
4. Click any pixel in the editor canvas to paint it.
5. Use **File → Save Project** (`Ctrl+S`) to save as `.spd`.
6. Use **File → Export As** (`Ctrl+Shift+E`) to export as `.prg` or `.bin` for use on the C64.

---

## Interface Overview

![Interface Overview](docs/screenshots/interface_overview.png)

| Area | Description |
|------|-------------|
| **Canvas** | Central editing area — click to paint pixels |
| **Sprite Strip** | Row of all sprites in the project — click to select |
| **Color Palette** | C64 16-color palette — left-click to set left button color, right-click to set right button color |
| **Properties Panel** | Sprite options: multicolor mode, overlay, expand X/Y |
| **Menu Bar** | File, Edit, and Help menus |
| **Status Bar** | Shows the currently selected sprite index |

---

## Editing Sprites

### Drawing

Left-click and drag on the canvas to draw with the **left button color**. Right-click and drag to draw with the **right button color**.

### Flood Fill

Click the **bucket fill** button in the toolbar to activate flood fill mode. Click any pixel on the canvas to fill the connected area with the selected color.

![Flood Fill](docs/screenshots/flood_fill.png)

### Selecting Sprites

Click a sprite in the sprite strip to select it. To select a range, click the first sprite and shift-click the last one. Most edit operations (transform, copy, delete) apply to the entire selection.

### Adding and Deleting Sprites

- **Edit → Add Sprite** — appends a new blank sprite
- **Edit → Delete Sprite** (`Del`) — removes all selected sprites

### Copy, Cut, Paste

| Action | Shortcut |
|--------|----------|
| Copy | `Ctrl+C` |
| Cut | `Ctrl+X` |
| Paste | `Ctrl+V` |
| Select All | `Ctrl+A` |

---

## Color Modes

### Single-Color Mode

Each sprite uses one **sprite color** plus the transparent/background color. Pixels are 1×1 on the C64.

### Multicolor Mode

Enable **MC Mode** in the properties panel to switch to multicolor. In this mode, a sprite can use three colors — the sprite color, **MC1**, and **MC2** — plus the transparent background. Pixels are rendered as "fat pixels" (2× width) to accommodate the 2-bit color encoding of the C64.

![Color Modes](docs/screenshots/color_modes.png)

### Color Swapping

Use **Edit → Color Switch** to swap colors across all pixels of the selected sprites:

- **Sprite Color ↔ MC1**
- **Sprite Color ↔ MC2**
- **MC1 ↔ MC2**

---

## Transformations

All transformation operations apply to the current sprite selection.

### Slide

Shift all pixels by one row or column (wrapping around the edges):

| Action | Shortcut |
|--------|----------|
| Slide Up | `Ctrl+U` |
| Slide Down | `Ctrl+D` |
| Slide Left | `Ctrl+L` |
| Slide Right | `Ctrl+R` |

### Flip

Mirror the sprite content:

- **Edit → Flip Left to Right** — horizontal mirror
- **Edit → Flip Top to Bottom** — vertical mirror

### Reflect

Copy one half of the sprite onto the other:

- **Edit → Reflect Left to Right** — mirrors the right half to the left
- **Edit → Reflect Top to Bottom** — mirrors the top half to the bottom

---

## Rotation

**Edit → Rotate** (`Ctrl+Shift+R`) opens the rotation dialog.

![Rotation Dialog](docs/screenshots/rotation_dialog.png)

| Option | Description |
|--------|-------------|
| **Angle** | Total rotation angle in degrees |
| **Steps** | Number of intermediate sprites to generate |
| **Include Overlay** | Rotate together with the overlay sprite |

A live preview shows all rotation steps before you apply them. OpenSprite uses a high-quality shear-based rotation with error diffusion to preserve multicolor accuracy.

Clicking **OK** appends the rotated sprites to the end of the project.

---

## Scaling

**Edit → Scale** opens the scaling dialog.

![Scaling Dialog](docs/screenshots/scaling_dialog.png)

| Option | Description |
|--------|-------------|
| **Scale Factor** | Target scale (0.1× – 4.0×) |
| **Steps** | Number of intermediate sprites to generate |
| **Include Overlay** | Scale together with the overlay sprite |

Scaling uses bilinear interpolation with exponential progression between steps. Clicking **OK** appends the scaled sprites to the end of the project.

---

## Overlay Sprites

The C64 supports overlapping two sprites on top of each other for richer graphics. OpenSprite represents this as two consecutive sprites:

1. Select a sprite and check **Overlay Next Sprite** in the properties panel.
2. The next sprite in the list is treated as the overlay layer.
3. An **Overlay Color** selector appears for the second sprite's color.
4. Optionally check **Force Overlay to Single Color** to prevent the overlay sprite from using multicolor mode (required for certain C64 overlay techniques).

![Overlay Sprites](docs/screenshots/overlay_sprites.png)

Overlay sprites are rendered combined in the editor canvas. Rotation and scaling operations can optionally include the overlay sprite.

---

## Animations

**File → Animations Editor** (`Ctrl+Shift+A`) opens the animation manager.

![Animation Editor](docs/screenshots/animation_editor.png)

Each animation defines:

| Property | Description |
|----------|-------------|
| **From** | First sprite index in the animation |
| **To** | Last sprite index in the animation |
| **Timer** | Frame delay in C64 cycles |
| **Ping Pong** | Plays forward then backward for smooth looping |
| **Overlay** | Enable overlay rendering during playback |

A preview plays the animation in real time inside the dialog.

### Creating an Animation from a Selection

Select a range of sprites in the sprite strip, then choose **Edit → Selection to Animation** (`Ctrl+Shift+A`). A new animation entry is created automatically with the From/To range pre-filled.

---

## Export & Import

### Supported Formats

| Format | Read | Write | Notes |
|--------|------|-------|-------|
| `.spd` | Yes | Yes | Native project format — preserves all data |
| `.prg` | Yes | Yes | C64 PRG executable with configurable load address |
| `.bin` | Yes | Yes | Raw binary without PRG header |
| `.spm` | Yes | — | SpritePad project import |

### Export Dialog

**File → Export As** (`Ctrl+Shift+E`) opens the export dialog.

![Export Dialog](docs/screenshots/export_dialog.png)

| Option | Description |
|--------|-------------|
| **Format** | PRG (with $0801 BASIC stub) or BIN (raw data) |
| **Start Address** | Load address in hex (e.g. `$3000`), PRG only |
| **Attributes** | SpritePad, SEUCK, or Empty |

Once exported, **File → Export** (`Ctrl+E`) re-exports to the same file without opening the dialog.

### Drag and Drop

Drag a `.spd`, `.prg`, `.bin`, or `.spm` file onto the OpenSprite window to import it. If the current project has unsaved changes, you will be prompted to save first.

---

## Keyboard Shortcuts

### File

| Action | Shortcut |
|--------|----------|
| New Project | `Ctrl+N` |
| Open / Import | `Ctrl+O` |
| Save Project | `Ctrl+S` |
| Save As | `Ctrl+Shift+S` |
| Export | `Ctrl+E` |
| Export As | `Ctrl+Shift+E` |
| Animations Editor | `Ctrl+Shift+A` |
| Zoom In | `Ctrl++` |
| Zoom Out | `Ctrl+-` |

### Edit

| Action | Shortcut |
|--------|----------|
| Undo | `Ctrl+Z` |
| Cut | `Ctrl+X` |
| Copy | `Ctrl+C` |
| Paste | `Ctrl+V` |
| Select All | `Ctrl+A` |
| Delete Sprite | `Del` |
| Slide Up | `Ctrl+U` |
| Slide Down | `Ctrl+D` |
| Slide Left | `Ctrl+L` |
| Slide Right | `Ctrl+R` |
| Rotate | `Ctrl+Shift+R` |

### Color Selection (number keys)

| Key | Selects |
|-----|---------|
| `0` | Transparent / Background |
| `1` | Sprite Color |
| `2` | MC1 |
| `3` | MC2 |
| `4` | Overlay Color (if overlay active) |
| `5` | Overlay Transparent (if overlay active) |

---

## Settings

**File → Settings** opens the display settings dialog.

![Settings Dialog](docs/screenshots/settings_dialog.png)

| Setting | Default | Description |
|---------|---------|-------------|
| Sprites per Row | 4 | Number of sprites displayed per row in the sprite strip |
| Horizontal Spacing | 30 px | Gap between sprites horizontally |
| Vertical Spacing | 30 px | Gap between sprites vertically |
| Spacing Color | — | Background color of the sprite strip |
| Selection Color | — | Highlight color for selected sprites |

---

## Building from Source

**Requirements:**
- Qt 5.15 or Qt 6.x (open-source edition)
- C++11-compatible compiler (GCC, Clang, MSVC)

```bash
git clone https://github.com/jowin202/OpenSprite.git
cd OpenSprite
qmake OpenSprite.pro
make
```

Or open `OpenSprite.pro` in Qt Creator and click **Build**.

---

## macOS First-Time Launch

If macOS shows **"App is damaged and can't be opened"** when launching OpenSprite, the app is not corrupted — macOS adds a quarantine flag to unsigned apps downloaded from the internet.

**To fix this:**

1. Unpack the `.zip` or `.dmg` file.
2. Open **Terminal**.
3. Run:

```bash
xattr -d com.apple.quarantine /path/to/OpenSprite.app
```

4. Launch OpenSprite normally.

---

## License

OpenSprite is released under the [GNU General Public License v3](LICENSE).
