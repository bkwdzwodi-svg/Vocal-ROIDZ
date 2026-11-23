# Vocal-ROIDZ

**VOCAL ROIDS** — the all-in-one vocal production workstation, designed by BKWDZWODI (BKWDZ STUDIO).

One plugin. One workflow. One finished vocal. Eighteen modules, each on its own themed page with
its own live graph and syringe controls, an FX rack that toggles every creative effect in one place,
and a genre and signature preset bank.

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

## Pitch correction does not change the voice

Autotune Pro corrects to the selected key and scale and nothing else:

- It snaps to the nearest in-scale note **in the singer's own octave**, so changing key or scale can
  never transpose the take, drop it an octave or make it sound deeper or higher. The tuner is
  limited to a one-semitone nudge by design.
- **Formant Preserve** is on by default and uses pitch-synchronous overlap-add: the pitch moves, the
  vocal-tract formants (and with them the singer's tone and character) stay put.
- Retune speed, humanize deadband, correction depth and Hard Tune shape how tightly it tracks.
- All creative pitching lives in the separate **PITCH FX** module, which is off until you switch it
  on: ±24 semitones of pitch, ±12 semitones of formant shift, and a dry/wet mix.

## The rack

| # | Module | What it does |
|---|--------|--------------|
| 01 | **Autotune Pro** | Scale-locked correction with formant preserve, retune speed, humanize, depth and hard tune |
| 02 | **Pitch FX** | Creative pitch up/down (±2 octaves), formant shift and mix — entirely separate from the tuner |
| 03 | **Analog EQ** | Draggable 6-band EQ graph: high/low pass, low shelf, bell with Q, high shelf and air, with the exact DSP response drawn live |
| 04 | **Compressor X** | VCA, FET, Optical and Tube detectors, soft knee, parallel mix, transfer curve with live point, in/out meters and a gain-reduction history graph |
| 05 | **Noise Gate Pro** | Threshold, attack, hold, release and range with a gate-open display |
| 06 | **Saturation Engine** | Tube, Tape, Vintage, Modern, Warm and Aggressive drive with warmth, density and mix |
| 07 | **Vocal Doubler** | Two drifting short delays with humanize and pitch-drift variation |
| 08 | **Harmony Engine** | Pitch-shifted 3rd, 5th, octave up and octave down, mixed over the dry take |
| 09 | **Reverb Space** | Room, Hall, Plate, Arena, Ambient, Dream and Cathedral with pre-delay, tone, modulation and width |
| 10 | **Delay Lab** | Analog, Tape, Ping Pong and Ducking modes with tone and width |
| 11 | **Regular Stutter** | Clean, repeating slices on a timer: trigger period, slice length, repeat count and mix |
| 12 | **Hyper Stutter FX** | Six slice modes: Micro Chop, Machine Gun, Hyperpop, Reverse Chaos, Freeze, Glitch Storm |
| 13 | **Auto Chop-N-Screw** | Classic Screw, Texas Heavy, BKWDZ WODI, Dark Atmosphere, Trance Screw, Chaos Screw with pitch drop, slowdown, phrase repeats and tape drift |
| 14 | **Stereo Imager** | Width, mono bass and balance with a goniometer |
| 15 | **De-Esser** | Split-band sibilance control with a sibilance graph |
| 16 | **Noise Removal** | Hum notches, breath-band reduction and click repair |
| 17 | **Creative FX Lab** | Reverse, tape stop, vinyl stop, megaphone, telephone, robot, bit crusher and granular echo |
| 18 | **Master Finish** | Punch, warmth, air, glue, loudness trim, ceiling limiter, LUFS and correlation meters |

Signal order: Autotune → Pitch FX → Harmony → Cleanup → Gate → EQ → Compressor → Saturation →
De-Esser → Regular Stutter → Chop-N-Screw → Hyper Stutter → Doubler → Stereo → Creative FX →
Delay → Reverb → Master.

## FX rack

The last page in the module rail is the **FX rack**: one card per creative effect with its own ON/OFF
switch — Pitch FX, Harmony Engine, Vocal Doubler, Regular Stutter, Hyper Stutter, Chop-N-Screw,
Creative FX Lab, Delay Lab and Reverb Space. Click a card to jump straight to that module's page for
the full controls.

Every module has a POWER switch, and every control is a draggable glass syringe: drag up to draw
more juice, drag down to push it out, double-click to reset, mouse wheel to nudge. Values are
host-automatable and saved with the project.

The EQ graph works like a hardware EQ display: drag a node horizontally for frequency and vertically
for gain, and roll the wheel over the bell for its Q. The compressor graph draws the real transfer
curve, shades the compressed region and shows input, output and gain reduction live.

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
- Formant-preserving correction works on pitch periods, so it adds a fixed ~22 ms of latency while
  Autotune Pro is enabled. The plugin does not report that latency to the host yet.
- AAX is not built here (it needs Avid's SDK).
- The DSP is 64-bit-float end to end; oversampling is not used, so the saturation and clipper are
  tuned to stay clean at normal vocal levels.
