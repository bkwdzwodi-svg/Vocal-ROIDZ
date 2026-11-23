# Vocal-ROIDZ

**VOCAL ROIDS** — the all-in-one vocal production workstation, designed by BKWDZWODI (BKWDZ STUDIO).

One plugin. One workflow. One finished vocal. Sixteen modules, each on its own page with its own
live graph and syringe controls, plus a genre and signature preset bank.

## Get a Windows VST3 without building anything

**Direct download:** [VocalROIDZ-Windows-VST3.zip](https://github.com/bkwdzwodi-svg/Vocal-ROIDZ/releases/latest/download/VocalROIDZ-Windows-VST3.zip)

That zip contains the `Vocal ROIDZ.vst3` bundle plus `INSTALL.txt`. Copy the `.vst3` folder into
`C:\Program Files\Common Files\VST3` and rescan plugins in your DAW.

The `Build Windows VST3` GitHub Actions workflow rebuilds it and refreshes that download on every
push to `main`; the same zip is also attached to each run:

1. Open the **Actions** tab, pick **Build Windows VST3**, then **Run workflow**.
2. When the run finishes, download the **VocalROIDZ-Windows** artifact.
3. Unzip it and copy the `Vocal ROIDZ.vst3` folder into `C:\Program Files\Common Files\VST3`
   (or any folder your DAW scans), then rescan plugins.

Your DAW will list it as **Vocal ROIDZ** by BKWDZ. The `.vst3` is a folder, not a single file:
keep `Contents` inside it when you copy it.

## The rack

| # | Module | What it does |
|---|--------|--------------|
| 01 | **Autotune Pro** | Monophonic pitch tracking locked to key + scale (12 keys, 12 scales), retune speed, humanize deadband, correction depth and hard tune |
| 02 | **Analog EQ** | High pass, low pass, low shelf, parametric bell, high shelf and air band with a live spectrum |
| 03 | **Compressor X** | VCA, FET, Optical and Tube detectors, soft knee, parallel mix, honest gain-reduction meter |
| 04 | **Noise Gate Pro** | Threshold, attack, hold, release and range with a gate-open display |
| 05 | **Saturation Engine** | Tube, Tape, Vintage, Modern, Warm and Aggressive drive with warmth, density and mix |
| 06 | **Vocal Doubler** | Two drifting short delays with humanize and pitch-drift variation |
| 07 | **Harmony Engine** | Pitch-shifted 3rd, 5th, octave up and octave down, mixed over the dry take |
| 08 | **Reverb Space** | Room, Hall, Plate, Arena, Ambient, Dream and Cathedral with pre-delay, tone, modulation and width |
| 09 | **Delay Lab** | Analog, Tape, Ping Pong and Ducking modes with tone and width |
| 10 | **Hyper Stutter FX** | Six slice modes: Micro Chop, Machine Gun, Hyperpop, Reverse Chaos, Freeze, Glitch Storm |
| 11 | **Auto Chop-N-Screw** | Classic Screw, Texas Heavy, BKWDZ WODI, Dark Atmosphere, Trance Screw, Chaos Screw with pitch drop, slowdown, phrase repeats and tape drift |
| 12 | **Stereo Imager** | Width, mono bass and balance with a goniometer |
| 13 | **De-Esser** | Split-band sibilance control with a sibilance graph |
| 14 | **Noise Removal** | Hum notches, breath-band reduction and click repair |
| 15 | **Creative FX Lab** | Reverse, tape stop, vinyl stop, megaphone, telephone, robot, bit crusher and granular echo |
| 16 | **Master Finish** | Punch, warmth, air, glue, loudness trim, ceiling limiter, LUFS and correlation meters |

Signal order: Autotune → Harmony → Cleanup → Gate → EQ → Compressor → Saturation → De-Esser →
Chop-N-Screw → Stutter → Doubler → Stereo → Creative FX → Delay → Reverb → Master.

Every module has a POWER switch, and every control is a draggable glass syringe: drag up to draw
more juice, drag down to push it out, double-click to reset, mouse wheel to nudge. Values are
host-automatable and saved with the project.

## Presets

Factory presets are grouped by style and are also exposed as host programs, so your DAW's
preset menu shows the same list.

**Genre Presets** — Melodic Trap, Dark Trap, Emo Rap, Rage, Hyperpop, Drill, Alternative Hip-Hop,
Singing Vocals, Pop Vocals, Podcast Voice, Broadcast Voice.

**Signature Presets** — BKWDZ Signature, Dream Vocal, Dark Atmosphere, Arena Vocal, Heavy Auto-Tune,
Natural Auto-Tune, Texas Screw Vocal, Hyper Stutter Trap.

Save your own with **SAVE**; they are stored in the `VocalROIDZ/Presets` folder inside your user
application data directory and appear under **My Presets**.

## Build

Requires CMake 3.22+ and a C++17 compiler. JUCE 8.0.4 is fetched automatically.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

To build against an existing JUCE checkout instead of downloading it:

```bash
cmake -S . -B build -DJUCE_PATH=/path/to/JUCE
```

Artifacts land in `build/VocalROIDZ_artefacts/`: VST3, AU (macOS) and a Standalone app.
Set `COPY_PLUGIN_AFTER_BUILD TRUE` to install into your system plugin folder automatically.

## Notes

- Autotune is monophonic, like a vocal take. Chords and stacked harmonies confuse any single-voice
  tracker, so print the tuned lead before stacking.
- AAX is not built here (it needs Avid's SDK).
- The DSP is 64-bit-float end to end; oversampling is not used, so the saturation and clipper are
  tuned to stay clean at normal vocal levels.
