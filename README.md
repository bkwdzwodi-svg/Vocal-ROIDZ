# Vocal-ROIDZ

All-in-one vocal plugin created by BKWDZ wodi.

Signal chain: **High-pass → Compressor (+ makeup) → De-esser → Delay → Output gain**, with a
syringe-style meter rack for input, compression, sibilance and output levels.

## Get a Windows VST3 without building anything

The `Build Windows VST3` GitHub Actions workflow builds a Release Windows VST3 and attaches
the zipped plugin to each run:

1. Open the **Actions** tab, pick **Build Windows VST3**, then **Run workflow**.
2. When the run finishes, download the **VocalROIDZ-Windows** artifact.
3. Unzip it and copy the `Vocal ROIDZ.vst3` folder into `C:\Program Files\Common Files\VST3`
   (or any folder your DAW scans), then rescan plugins.

Your DAW will list it as **Vocal ROIDZ** by BKWDZ. The vst3 is a folder, not a single file:
keep `Contents` inside it when you copy it.

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

## Presets

Factory presets are grouped by style and are also exposed as host programs:

- **Lead Vocals** — Clean Lead, Pop Lead, Rock Lead, R&B Lead, Country Lead, Gospel Lead
- **Rap and Ad-libs** — Rap Lead, Trap Ad-lib, Big Echo Ad-lib
- **Backgrounds and Choir** — Smooth Backgrounds, Soft Choir Stack
- **Broadcast and Voice** — Podcast, Voiceover, Whisper / ASMR
- **Creative FX** — EDM Topline, Lo-Fi Tape, Live Stage

Save your own with **Save Preset**; they are stored in the `VocalROIDZ/Presets` folder inside
your user application data directory and appear under **My Presets**.
