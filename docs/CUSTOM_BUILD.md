# Metroid Prime Port — Sol1vaN custom28 (Windows x86_64)

Custom build based on upstream v0.23.0. The executable reports 0.23.0-custom28.
This is Sol1vaN's personal fork build, maintained independently of the upstream release.

## Run

Extract the entire folder, place your own supported Metroid Prime disc image beside metroid_prime_port.exe, and launch the executable. The Visual C++ runtime is required. The package contains no disc image, saves or personal settings.

## Included custom features

High-DPI mouse sensitivity with six decimal places; configurable port hotkeys; continuous brightness; optional Stick Sway and Inertia Sway with separate strength; automatic/manual viewmodel position and FOV with Reset; keyboard Walk/Run with gradual transitions and neutral-input fix; movement-aware bobbing and footsteps; aspect-correct electrical visor effects; smooth map rotation, wheel zoom, translation inversion and unchanged-pitch step timing; two map legend layouts with effective bindings and animated native-style glyphs.

Includes upstream v0.23 features: photo mode, quick resume, room-aware save states, save conversion/import/export, later disc revisions, revision compatibility, prompt/menu fixes, widescreen HUD fixes and Remastered improvements.

## Integration choices

Photo mode uses F8 if free, otherwise the next available key (typically F7 when F8 already loads a state). Remap it in Controls > Remap > Port.

Custom visual viewmodel adjustment and upstream arm cannon offsets are alternative position modes; they never stack. Disable Enable viewmodel adjustment to expose the upstream offsets. Reset restores both.

## Validation

Windows x86_64 RelWithDebInfo compilation succeeded. 79/79 automated tests passed. Smoke/input-injection driver is disabled. The Walk fix was manually validated in custom27; the complete v0.23 integration still needs further manual gameplay coverage.

Executable SHA256: bc5815d5b9701277c96de4dfb0fc05f539d9a7a9113a63911ee7ab910eb99eb6

Upstream: https://github.com/Odrannnn/MetroidPrimePort
Fork: https://github.com/Sol1vaN/MetroidPrimePort
Viewmodel adjustment concept referenced from PrimeHack: https://github.com/shiiion/dolphin
Developed with Codex assistance and user playtesting. See LICENSE, NOTICE and licenses/ for attribution and terms.
