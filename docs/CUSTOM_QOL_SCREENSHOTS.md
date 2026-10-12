# Custom QoL screenshots

These screenshots illustrate Sol1vaN's custom27 build. They show the combined custom implementation; individual upstream PRs contain only their stated changes.

## Two map legend layouts

With **Mouse aim enabled**, the modern keyboard/mouse layout groups mouse rotation and wheel zoom, then shows **Move U/D** and **Move L/R** with the effective movement bindings. Paired direction prompts alternate smoothly, including left/right mouse-button highlights.

![Modern keyboard/mouse map legend](images/custom-qol/map-modern-keyboard-mouse.png)

With **Mouse aim disabled**, the classic layout preserves the action organization of the original Nintendo GameCube controller/stick scheme, also applicable to other gamepads: **Rotate V/H**, **Zoom in/out**, and **Move U/D/L/R**. In this screenshot those original stick actions are mapped to keyboard/mouse inputs, so it shows their assigned key/button glyphs rather than a gamepad-only glyph set.

![Classic stick-scheme map legend, showing mapped keyboard/mouse prompts](images/custom-qol/map-original-stick-layout.png)

Both layouts retain the game's frame, visual style and existing prompt artwork, with supplementary glyphs where necessary (such as Menu). They adapt the instructions to the control scheme rather than replacing the map's visual design.

## Mouse settings and keyboard Walk/Run

This screen shows six-decimal high-DPI sensitivity (0.000040 rad/px), map zoom sensitivity, translation-axis inversion, walking speed and Always run. Invert map axis changes translation only, not map rotation. The displayed checkboxes and values are personal settings, not a statement of the defaults.

![Mouse sensitivity, map settings and Walk/Run controls](images/custom-qol/mouse-and-walk-settings.png)

The Walk action is remappable alongside the movement actions:

![Walk binding in the remapping menu](images/custom-qol/walk-binding.png)

## Weapon sway and viewmodel adjustment

The custom UI offers None, Stick Sway and Inertia Sway, a strength control, and viewmodel adjustment with automatic/manual modes, a separate viewmodel FOV, three position controls and Reset.

![Custom weapon sway and viewmodel adjustment UI](images/custom-qol/weapon-sway-viewmodel-settings.png)

The **Viewmodel Position / weapon viewmodel adjustment concept is referenced from [PrimeHack](https://github.com/shiiion/dolphin)**, particularly its automatic/manual adjustment and left/right, forward/backward and up/down controls. The image below is the PrimeHack reference used for this feature; it is not a screenshot of MetroidPrimePort. The custom port implementation adds its own UI and reset behavior.

![PrimeHack reference: Viewmodel Position controls](images/custom-qol/primehack-viewmodel-reference.png)

Screenshots supplied by Sol1vaN. PrimeHack reference UI credited to the PrimeHack project; game artwork remains attributable to its original creators.
