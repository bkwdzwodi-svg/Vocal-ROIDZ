# Vocal-ROIDZ

**VOCAL ROIDS** — the all-in-one vocal production workstation, designed by BKWDZWODI (BKWDZ STUDIO).

One plugin. One workflow. One finished vocal. Nine sections, each on its own themed page with its
own live graph and syringe controls, a scrollable FX rack for everything creative, and a
level-matched preset bank mixed like finished records.

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

## The nine sections

| # | Section | What it does |
|---|---------|--------------|
| 01 | **Autotune** | Classic hard tune, always snapped: scale-locked correction with retune, humanize and correction depth |
| 02 | **Pitch** | Creative transposition ±2 octaves, formant shift and mix — entirely separate from the tuner |
| 03 | **EQ** | Seven-band graph with live spectrum: high/low pass, low shelf, two bells with Q, high shelf and air, drawn from the real DSP response |
| 04 | **Compressor** | Clean peak+RMS program-dependent detector in VCA, FET, Optical and Tube feels, with transfer curve, meters and gain-reduction history |
| 05 | **Saturation** | Tube, Tape, Vintage, Modern, Warm and Aggressive drive with warmth, density and mix |
| 06 | **Reverb** | Room, Hall, Plate, Arena, Ambient, Dream and Cathedral with pre-delay, tone, modulation and width |
| 07 | **Limit** | Punch, warmth, air, glue, loudness trim and a ceiling limiter, with loudness and correlation meters |
| 08 | **Gate** | Threshold, attack, hold, release and range with a gate-open display |
| 09 | **FX Rack** | Everything else, each with its own power switch and controls |

### The FX rack

| Effect | What it does |
|--------|--------------|
| **Harmony** | Pitch-shifted 3rd, 5th, octave up and octave down over the dry take |
| **Noise Removal** | Hum notches, breath-band reduction and click repair |
| **De-Esser** | Split-band sibilance control with a sibilance graph |
| **Regular Stutter** | Clean, repeating slices on a timer: trigger period, slice length, repeats, mix |
| **Hyper Stutter** | Micro Chop, Machine Gun, Hyperpop, Reverse Chaos, Freeze, Glitch Storm |
| **Chop-N-Screw** | Classic Screw, Texas Heavy, BKWDZ WODI, Dark Atmosphere, Trance Screw, Chaos Screw with pitch drop, slowdown, phrase repeats and tape drift |
| **Doubler** | Two drifting short delays with humanize and pitch-drift variation |
| **Convolver** | Real impulse-response reverb: Room, Plate, Hall and Church with pre-delay, tone, width and mix |
| **Cassette** | Wow, flutter, drive, tape tone and hiss for a worn-tape vocal |
| **DJ Spin** | Timed backspins and brake stops: rate, length, backspin depth and mix |
| **Stereo** | Width, mono bass and balance with a goniometer |
| **Delay** | Analog, Tape, Ping Pong and Ducking modes with tone and width |

Signal order: Autotune → Pitch → Harmony → Cleanup → Gate → EQ → Compressor → Saturation →
De-Esser → Regular Stutter → Hyper Stutter → Chop-N-Screw → Doubler → Convolver → Cassette →
DJ Spin → Stereo → Delay → Reverb → Limit.

Every section has a POWER switch, and every control is a draggable glass syringe: drag up to draw
more juice, drag down to push it out, double-click to reset, mouse wheel to nudge. Values are
host-automatable and saved with the project.

The EQ graph works like a hardware EQ display: the live spectrum sits behind a seven-band curve,
with a labelled dB and frequency grid. Drag a node horizontally for frequency and vertically for
gain, and roll the wheel over either bell for its Q. The compressor graph draws the real transfer
curve, shades the compressed region and shows attack/release, input, output and gain reduction live.

## Pitch correction does not change the voice

- Autotune snaps to the nearest in-scale note **in the singer's own octave**, so changing key or
  scale can never transpose the take or make it sound deeper. The tuner is limited to a
  one-semitone nudge by design.
- Correction is always **classic hard tune**: there is no formant-preserve switch and no mode
  switch to get wrong. The tuner uses a tight grain snap, so the note lands and stays there.
- Creative pitch (pitch, formant shift, octaves) lives in the Pitch section and the FX rack only.

## Presets

Factory presets are grouped by style and are exposed as host programs, so your DAW's preset menu
shows the same list. They are level-matched so switching never jumps in volume.

**Genre Presets** — Melodic Trap, Trap Tune, Dark Trap, Emo Rap, Rage, Hyperpop, Drill,
Alternative Hip-Hop, Singing Vocals, Pop Vocals, R&B Smooth, Podcast Voice, Broadcast Voice,
Voiceover Clean.

**Signature Presets** — BKWDZ Signature, Dream Vocal, Dark Atmosphere, Arena Vocal, Hard Tune,
Natural Correction, Screwed & Chopped, Doubled Stack, Stutter Trap, Lo-Fi Bedroom.

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
