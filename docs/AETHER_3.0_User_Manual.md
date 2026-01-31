# AETHER 3.0 — User Manual

**Neuro-Bass Workstation & Multiband Distortion**

*Version 3.0 • Antigravity / Æther Labs*

---

*This manual describes AETHER 3.0 in full: what it is, why it was made, how to install and use it, every control, signal flow, presets, tips, technical specs, and future plans. Screenshot placeholders are marked as **[SCREENSHOT: description]** — replace with actual images for your final PDF or web version.*

---

## Table of Contents

- [Before You Begin](#before-you-begin)
1. [Introduction](#1-introduction)
2. [What Is AETHER?](#2-what-is-aether)
3. [Why AETHER Was Made](#3-why-aether-was-made)
4. [Who It’s For & Why You Should Care](#4-who-its-for--why-you-should-care)
5. [System Requirements & Installation](#5-system-requirements--installation)
6. [Quick Start](#6-quick-start)
7. [Interface Overview](#7-interface-overview)
8. [Signal Flow & Architecture](#8-signal-flow--architecture)
9. [Controls Reference](#9-controls-reference)
10. [Factory Presets](#10-factory-presets)
11. [Tips & Techniques](#11-tips--techniques)
    - [11.1 Understanding the Orb and Visuals](#111-understanding-the-orb-and-visuals)
    - [11.2 Workflow Suggestions](#112-workflow-suggestions)
12. [Technical Specifications](#12-technical-specifications)
13. [Roadmap & Future Plans](#13-roadmap--future-plans)
14. [Troubleshooting](#14-troubleshooting)
- [Appendix A: Parameter Quick Reference](#appendix-a-parameter-quick-reference)
- [Appendix B: Glossary](#appendix-b-glossary)
15. [Credits & Support](#15-credits--support)

---

## Before You Begin

This manual is written for **AETHER 3.0** and assumes you have the plugin installed (or are about to install it) in a compatible DAW or as a standalone application. No prior experience with AETHER is required. If you are new to multiband processing or distortion plugins, reading **Section 2 (What Is AETHER?)** and **Section 8 (Signal Flow)** first will help. For a fast start, go straight to **Section 6 (Quick Start)** and **Section 10 (Factory Presets)**.

**How to use this manual**

- **Screenshot placeholders** are written as **[SCREENSHOT: description]**. Replace these with actual screenshots of your plugin window for a polished PDF or web manual. Descriptions indicate what each image should show.
- **Page count:** When exported to PDF with normal margins and font size (e.g. 11–12 pt body), this manual is designed to be **20+ pages** including space for screenshots and headings.
- **Version:** Content refers to AETHER 3.0. If you use a different build (e.g. beta or custom), some features (e.g. Space, Scramble UI) may differ; check your build notes.

---

## 1. Introduction

AETHER 3.0 is a professional **multiband distortion and saturation plugin** built for electronic music production, sound design, and mixing. It combines a **split-band architecture** (sub + highs), **bipolar distortion** (separate algorithms for positive and negative waveform halves), **morphing filters** (including vowel/formant mode), **feedback resonator**, **noise texture**, and **stereo width** in one instrument. The result is a single plugin that can go from subtle warmth to full “reactor” saturation, reese basses, metallic resonances, and vocal-like filtering—without leaving your DAW.

This manual explains everything: the idea behind AETHER, how to get it running, how to use every part of the interface, how the audio is processed, and how to get the most out of it. Whether you’re a producer, sound designer, or engineer, you’ll find the detail you need here.

**[SCREENSHOT: Full AETHER 3.0 interface — main window with all sections visible (header, orb, distortion, filter, feedback, deck, scope).]**

---

## 2. What Is AETHER?

AETHER is an **audio effect plugin** (VST3, AU, Standalone) that runs inside your DAW (Ableton Live, Logic, Bitwig, etc.) or as a standalone app. It is **not** a synth: you feed it audio (drums, bass, synths, vocals, full mixes), and it distorts, filters, and reshapes that audio in real time.

**In one sentence:** AETHER is a multiband distortion workstation with separate low and high processing, bipolar saturation, morphing/vowel filters, feedback resonance, and stereo tools—designed for bass music, DnB, techno, and modern electronic production.

**Core concepts:**

- **Multiband:** Your signal is split into **sub (low)** and **highs**. The sub stays clean and punchy; the highs go through distortion, filter, and feedback. Then they’re summed back. This keeps low-end solid while you destroy the top end.
- **Bipolar distortion:** The plugin can use **different saturation algorithms** for the positive and negative parts of the waveform. That asymmetry creates richer harmonics and more character than a single algorithm.
- **Stages:** The high band can be processed **1–12 times** in a row. More stages = thicker, more broken-up saturation (the “reactor”).
- **Morph & Vowel:** The filter can **morph** between low-pass, band-pass, and high-pass, or switch into **Vowel** mode and sweep through formant vowels (A, E, I, O, U) for talking/growling tones.
- **Feedback resonator:** A tuned delay line with feedback creates metallic, Karplus-style, or screaming resonances—great for DnB and industrial textures.
- **Noise, squeeze, width:** Optional **noise** (white/pink/crackle), **squeeze** (OTT-style dynamics), and **stereo width** let you add grit, punch, and space.

**[SCREENSHOT: Central Orb and Transfer Curve visualizer — show the orb and the small transfer graph above it.]**

---

## 3. Why AETHER Was Made

AETHER was created to fill a gap: **one plugin** that can do **serious multiband distortion and tonal shaping** without stacking multiple devices or fighting phase and level issues.

**Design goals:**

1. **Phase-coherent multiband.** The crossover is a **4th-order Linkwitz–Riley** (24 dB/oct, phase-matched). Sub and highs are recombined without the phase smearing and level dips you often get with random filter chains.
2. **Bipolar character.** Real analog and “broken” digital units often treat positive and negative peaks differently. AETHER does this explicitly: you choose one algorithm for positive and one for negative, so you get asymmetric grit and movement by design.
3. **Controlled destruction.** Drive, stages, fold, filter, resonance, and feedback give many ways to push the sound from “warm” to “reactor meltdown,” while the sub and dry/wet keep the mix usable.
4. **Electronic-music workflow.** Presets and controls are named and laid out for bass music, DnB, techno, and sound design—reese, growl, plasma, vowel grit, etc.—so you can get to a sound fast.
5. **Visual feedback.** The central **Orb**, **Transfer** curve, **Reactor Tank**, and **scope** reflect drive, morph, stages, and signal level so you can see what the plugin is doing.

**[SCREENSHOT: Transfer visualizer close-up — bipolar curve with drive and fold visible.]**

---

## 4. Who It’s For & Why You Should Care

**Producers & beatmakers**  
You want one go-to distortion for basses, drums, and full mixes. AETHER gives you sub-safe multiband saturation, reese-style detune (via feedback + filter), and presets that get you in the ballpark fast.

**Sound designers**  
You need extreme control: bipolar algorithms, 12 stages, vowel filters, resonance, noise, and width. AETHER is built for designing signature tones—metallic, vocal, broken, or clean—without leaving the box.

**Mix engineers**  
You want saturation and character that don’t trash the low end. The crossover and sub level let you add grit and weight while keeping the bottom tight and mono.

**Why care?**

- **One plugin, many jobs:** warmth, crunch, reese, growl, vowel, plasma, radio, width—all in one place.
- **Transparent when you want it:** dry/wet and sub level let you blend in just enough saturation.
- **Presets as starting points:** 13 factory presets cover classic use cases; you tweak from there.
- **DAW-friendly:** VST3 and AU for Mac/Windows; works in Ableton, Logic, Bitwig, Studio One, etc.

**[SCREENSHOT: Factory Presets dropdown open — list of preset names.]**

---

## 5. System Requirements & Installation

**Supported systems**

- **macOS:** 10.14 (Mojave) or later (Intel and Apple Silicon).
- **Windows:** 10 or later (64-bit).
- **Formats:** VST3, AU (Mac), Standalone.

**Hosts**

- Any DAW that loads VST3 or AU: Ableton Live, Logic Pro, Bitwig Studio, Studio One, Reaper, FL Studio, etc.

**Installation**

1. **Build from source (developer):**  
   Open the project in CMake, configure for your OS, and build. The built plugin and/or standalone app will be in the build output folder (e.g. `Aether_artefacts`). Copy the plugin component to your system’s VST3/AU folder if needed.

2. **Installer / pre-built (if provided):**  
   Run the installer and follow the steps. It will typically install:
   - **VST3:** `C:\Program Files\Common Files\VST3\` (Windows) or `/Library/Audio/Plug-Ins/VST3/` (Mac)
   - **AU:** `/Library/Audio/Plug-Ins/Components/` (Mac)
   - **Standalone:** Applications folder or chosen location.

3. **Rescan plugins in your DAW** so it picks up AETHER.

**[SCREENSHOT: DAW plugin browser with “AETHER 3.0” or “Antigravity” visible in the list.]**

---

## 6. Quick Start

**Get sound in 30 seconds**

1. Insert AETHER on an **audio track** (or return) that has signal (e.g. a bass or drum bus).
2. Leave **Dry/Wet** at 100% and **GAIN** at 0 dB to hear the full effect.
3. Choose a **Factory Preset** (e.g. “INIT / REESE” or “TECHSTEP GROWL”) from the top-right dropdown.
4. Play your track and adjust **Drive** and **Stages** to taste; use **Cutoff** and **Morph** to shape tone.
5. Use **Dry/Wet** to blend effect with dry, and **GAIN** to match level.

**First moves**

- **More grit:** Increase **Drive** and/or **Stages** (reactor tank).
- **Darker/brighter:** Move **Cutoff** left (dark) or right (bright).
- **Vowel / talking:** Click the **MORPH** button so it switches to **VOWEL**, then move the **Morph** knob to sweep A→E→I→O→U.
- **Metallic resonance:** Raise **Feedback** and set **Time** (ms); try low **Time** for comb-like tones.
- **Sub level:** Use **Sub** and **X-OVER** to keep the low end solid; **Width** only affects the processed high band.

**[SCREENSHOT: Plugin on a track in Ableton Live — device title “AETHER 3.0” and signal flowing.]**

---

## 7. Interface Overview

The AETHER window is divided into clear areas. Here’s a top-to-bottom, left-to-right tour.

**[SCREENSHOT: Full interface with numbered or labeled regions: 1=Header, 2=Orb, 3=Distortion, 4=Filter, 5=Feedback, 6=Deck, 7=Scope.]**

### 7.1 Header (Top Bar)

- **Logo (left):** Animated particle orb and branding. No functional controls.
- **Algorithm selectors (left-center):** Two dropdowns for **Positive** and **Negative** distortion algorithms (None, SoftClip, HardClip, SineFold, etc.). These define the bipolar character.
- **Transfer curve (center):** Small graph of the distortion transfer (input → output). Updates with drive and algorithms.
- **Factory Presets (right):** Dropdown to load factory presets (INIT / REESE, CRUSHED LIQUID, TECHSTEP GROWL, etc.).
- **Help (?) (right):** Toggles detailed tooltips on/off for all controls.

**[SCREENSHOT: Header area only — logo, two algo dropdowns, preset menu, help button.]**

### 7.2 Central Orb

- Large **glowing sphere** in the middle of the UI. It **reacts** to **Drive** and **Morph** (and level). Purely visual—no clicks required. Gives a quick read of how “hot” and how “morphed” the sound is.

**[SCREENSHOT: Central Orb — default state and with high drive.]**

### 7.3 Distortion Section (Top-Left)

- **DRIVE:** Input gain into the distortion (pre saturation). More = harder clip and more harmonics.
- **FOLD:** Sine wavefolder amount. Bends peaks instead of hard clipping; adds hollow, synthy harmonics.
- **Noise row:** **NOISE** (level), **WIDTH** (stereo spread of noise), and **Type** dropdown (White / Pink / Crackle).
- **12-Stage Reactor (right):** Vertical “tank” showing **1–12 stages**. Drag or click to set how many times the high band is re-processed. More stages = denser, more broken saturation.

**[SCREENSHOT: Distortion section — Drive, Fold, Noise knobs and reactor tank.]**

### 7.4 Filter Section (Top-Right)

- **CUTOFF:** Filter frequency (≈80 Hz–20 kHz). Lower = darker; higher = brighter.
- **RESONANCE:** Emphasis around cutoff. Low = smooth; high = peaky, whistling, or growling (especially in Vowel mode).
- **MORPH knob:** In **MORPH** mode: sweeps filter shape (LP → BP → HP). In **VOWEL** mode: sweeps formant vowels A→E→I→O→U.
- **MORPH / VOWEL button:** Toggles between **Morph** (filter shapes) and **Vowel** (formant) mode.

**[SCREENSHOT: Filter section — Cutoff, Resonance, Morph knob, MORPH/VOWEL button.]**

### 7.5 Feedback / Resonator (Bottom-Left)

- **FEEDBACK:** Amount of signal fed back into the delay line. Low = subtle body; high = metallic ring or scream.
- **TIME:** Delay time in ms. Short = comb-like; long = stretched metallic echoes.
- **SPACE:** Intended for diffusion/space in the feedback path (implementation may vary by version).

**[SCREENSHOT: Feedback section — Feedback, Time, Space knobs.]**

### 7.6 Deck / Global (Bottom Center-Right)

- **SUB:** Level of the clean, mono sub band (after crossover). Keeps low end present.
- **X-OVER:** Crossover frequency (≈60–300 Hz). Defines where sub stops and highs begin.
- **SQUEEZE:** OTT-style upward/downward compression; adds punch and grit.
- **WIDTH:** Stereo width of the **processed** high band only. Sub stays mono.
- **GAIN:** Output level (≈−24 dB to +24 dB).
- **DRY/WET:** Blend between dry (original) and wet (fully processed). 0% = bypass; 100% = full effect.

**[SCREENSHOT: Deck row — Sub, X-OVER, Squeeze, Width, Gain, Dry/Wet knobs.]**

### 7.7 Scope (Bottom)

- **Waveform display** of the output (or processed) signal. Confirms that the plugin is receiving and outputting audio.

**[SCREENSHOT: Bottom scope with waveform.]**

### 7.8 Tooltips (Help Mode)

- With **Help (?)** turned **on**, hovering over any knob or control shows a **tooltip** with a plain-language description of what that control does. Tooltips use the same dark, on-brand styling as the rest of the interface. Turn Help **off** if you prefer a cleaner cursor.

**[SCREENSHOT: Tooltip visible on hover over one knob.]**

---

## 8. Signal Flow & Architecture

Understanding the order of operations helps you get the sound you want.

**Block diagram (simplified):**

```
Input (stereo)
    → Noise injection (optional)
    → Crossover (Linkwitz–Riley 24 dB/oct)
    → Low band  → Sub processor (mono, gentle sat) → [Sub out]
    → High band → 4× oversample
                  → Fold (wavefolder)
                  → Bipolar distortion (Pos/Neg algos, 1–12 stages)
                  → Filter (Morph or Vowel)
                  → Resonator (feedback delay)
                  → Dimension (width)
                  → Squeeze (OTT-style)
                  → [High out]
    → Sum: Sub + High
    → Dry/Wet mix
    → Output gain
    → Output
```

**Takeaways:**

- **Noise** is added **before** the split, so both sub and highs can carry it; the heavy processing is on the high band.
- **Sub** is only: crossover → sub processor (mono + light saturation) → level. No distortion, filter, or feedback on the sub.
- **High band** is oversampled 4×, then: fold → distortion (with stages) → filter → resonator → width → squeeze.
- **Dry/Wet** and **Output** are at the very end.

**[SCREENSHOT: Optional — hand-drawn or diagram of signal flow (can be a simple box diagram).]**

---

## 9. Controls Reference

This section goes through **every** control in detail: what it does, typical range, and how it interacts with the rest.

### 9.1 Drive

- **What it does:** Sets input gain **before** the wavefolder and distortion. Higher = more level into the nonlinear stages = more saturation and harmonics.
- **Range:** 0–100% (0.0–1.0).
- **Typical use:** Start around 40–60% for gentle warmth; push 70–100% for heavy grit. Watch the Orb and transfer curve.
- **Interaction:** More drive makes **Stages** and **Fold** more noticeable; the filter and resonator then shape that saturated signal.

**[SCREENSHOT: Drive knob at 0%, 50%, 100% with short caption.]**

### 9.2 Fold (Wavefolder)

- **What it does:** Applies a **sine wavefolder**: instead of hard clipping, peaks are “folded” back, adding odd/even harmonics and a hollow, synthy character.
- **Range:** 0–100%.
- **Typical use:** 0% = no folding; 30–60% = clear “talking” or reese-like tone; high = very broken, synthetic.
- **Interaction:** Works in series with Drive and Stages; often used with Morph or Vowel for vocal-like movement.

### 9.3 Stages (1–12)

- **What it does:** Re-processes the high band through the distortion **1 to 12 times** in a row. Each pass adds more nonlinearity. The “Reactor Tank” bar shows the current value.
- **Range:** 1–12 (integer).
- **Typical use:** 1–2 = light saturation; 4–8 = thick, broken; 12 = maximum density. Start low and increase until it feels right.
- **Interaction:** More stages amplify the effect of **Drive** and **Fold**; the filter and resonator then shape that dense signal.

**[SCREENSHOT: Reactor Tank at 1, 6, and 12 stages.]**

### 9.4 Positive / Negative Algorithm

- **What it does:** Chooses the **saturation algorithm** for the **positive** and **negative** halves of the waveform. Different combinations give asymmetric harmonics.
- **Options:** None, SoftClip, HardClip, SineFold, TriangleWarp, BitCrush, Rectify, Tanh, SoftFold, Chebyshev.
- **Typical use:** Try e.g. SoftClip (positive) + SineFold (negative), or HardClip + Rectify for aggressive asymmetry. Presets show suggested pairs.
- **Interaction:** Defines the “color” of the distortion that Stages and Drive then multiply.

**[SCREENSHOT: Algorithm dropdowns open with one algorithm selected.]**

### 9.5 Noise Level, Width, Type

- **Noise Level:** Amount of noise added to the signal before processing. Distortion then “grabs” the noise for extra grit and sizzle.
- **Noise Width:** Stereo spread of the noise (0 = mono, 1 = wide).
- **Type:** White (flat spectrum), Pink (warmer), Crackle (granular-style impulses).
- **Typical use:** Low level (10–30%) for texture; higher for lo-fi or noise-heavy designs. Width and type shape the character.

### 9.6 Cutoff

- **What it does:** Sets the **center frequency** of the main filter (≈80 Hz–20 kHz). In Morph mode it’s the pivot for LP/BP/HP; in Vowel mode it scales the formant frequencies.
- **Range:** 80 Hz – 20 kHz (log-style).
- **Typical use:** Lower for dark, muffled tones; higher for bright, open. Sweep for movement.

### 9.7 Resonance

- **What it does:** Increases gain **around** the cutoff (Q). Low = smooth; high = peaky, ringing, or self-oscillation.
- **Range:** 0–100%.
- **Typical use:** Keep low (10–30%) for smooth tone; raise for whistles, growls, or vowel emphasis. Use carefully—very high can be harsh.

### 9.8 Morph (Knob)

- **What it does:**  
  - **MORPH mode:** Sweeps filter type from Low-Pass (0) → Band-Pass (0.5) → High-Pass (1).  
  - **VOWEL mode:** Sweeps formant vowels A → E → I → O → U.
- **Range:** 0–100%.
- **Typical use:** Sweep slowly for filter movement; in Vowel mode, small moves give distinct vowel steps.

### 9.9 MORPH / VOWEL Button

- **What it does:** Switches the filter between **Morph** (LP/BP/HP shapes) and **Vowel** (formant A–E–I–O–U).
- **Typical use:** Click to VOWEL when you want talking/growling tones; click back to MORPH for classic filter sweeps.

### 9.10 Feedback

- **What it does:** Sends part of the **processed** signal back into a delay line (resonator). More = more sustain, metallic ring, or screaming resonance.
- **Range:** 0–110% (slightly over 1.0 for self-oscillation).
- **Typical use:** Start at 0; add 20–40% for body; push 70%+ for metallic or screaming tones. Use with **Time** to tune the character.

### 9.11 Time

- **What it does:** Delay length of the resonator in **milliseconds**. Short = comb-like, metallic; long = stretched echoes.
- **Range:** ~0.1–500 ms.
- **Typical use:** 5–20 ms for comb/grit; 20–100 ms for tuned metallic; 100+ ms for long tails. Works with **Feedback** and (in engine) **Scramble/Plasma** for movement.

### 9.12 Space

- **What it does:** Intended to add diffusion/space to the feedback path (e.g. small-room or tank-like resonance). Implementation may vary; see your build.
- **Range:** 0–100%.

### 9.13 Sub

- **What it does:** **Level** of the **low band** (sub) after the crossover. Sub is mono-summed and lightly saturated; this knob only changes its level.
- **Range:** 0–200% (0.0–2.0).
- **Typical use:** 100% = balanced; >100% for more weight; <100% if you want the effect to thin the low end.

### 9.14 X-OVER (Crossover Frequency)

- **What it does:** **Frequency** where the signal is split into sub and highs (Linkwitz–Riley, 24 dB/oct). Below = sub; above = highs (distortion path).
- **Range:** ~60–300 Hz.
- **Typical use:** 80–120 Hz for typical bass; lower to send more bass into the effect; higher to keep more low-mids clean.

### 9.15 Squeeze

- **What it does:** OTT-style upward/downward compression: brings up quiet details and squashes peaks. Adds punch and grit.
- **Range:** 0–100%.
- **Typical use:** 30–60% for punch; higher for aggressive squash. Works on the high band only.

### 9.16 Width

- **What it does:** **Stereo width** of the **processed high band** (mid-side or similar). Sub remains mono.
- **Range:** 0–150% (0 = mono, 1.5 = very wide).
- **Typical use:** 0 for mono compatibility; 50–100% for width; higher for dramatic stereo.

### 9.17 GAIN (Output)

- **What it does:** **Output level** after dry/wet mix. Compensate for level change when the effect is on.
- **Range:** −24 dB to +24 dB.
- **Typical use:** Set so the track level matches when the plugin is bypassed vs engaged.

### 9.18 DRY/WET (Mix)

- **What it does:** Blend between **dry** (unaltered input) and **wet** (fully processed). 0% = bypass; 100% = full effect.
- **Range:** 0–100%.
- **Typical use:** 100% for full effect; 50–80% for parallel-style blend; 0% to A/B.

**[SCREENSHOT: Full control layout with one or two knobs highlighted and a short caption.]**

---

## 10. Factory Presets

AETHER ships with **13 factory presets** as starting points. Load from the **Factory Presets** dropdown (top-right).

| Preset name          | Short description                                      |
|----------------------|--------------------------------------------------------|
| INIT / REESE         | Mild drive, 2 stages, flat filter; good starting reese.|
| CRUSHED LIQUID       | Softer drive, 1 stage, gentle morph; liquid character. |
| TECHSTEP GROWL       | Heavy drive, 8 stages, high resonance; techstep growl.  |
| METALLIC TALKER      | Strong resonance, vowel area; metallic “talking” filter.|
| FURNACE BLAST        | Max drive, 12 stages, bright; full saturation.         |
| VOWEL GRIT AEU       | Vowel-style formant with grit; A–E–U zone.            |
| SUBMERGED            | Low drive, high sub; clean low end, light top.        |
| PLASMA FEEDBACK      | High feedback, high resonance; plasma-style resonance. |
| BROKEN RADIO         | Full drive, 1 stage, open filter; lo-fi radio.        |
| NEURONAL WIDTH       | Moderate drive, width and morph; wide, moving.        |
| HARSH SINEFOLD       | Strong fold, 6 stages; harsh sine-fold character.      |
| DARK MATTER          | Low cutoff, high morph, long feedback; dark, spacey.   |
| ULTIMA REAKTOR       | Near-max drive/stages/resonance; “ultimate” reactor.   |

**Note:** Presets set Drive, Stages, Cutoff, Res, Morph, Feedback, Time, Mix, Sub, Squeeze, and Positive/Negative algorithms. They do **not** store Noise, Width, X-OVER, or Output; adjust those per project.

**[SCREENSHOT: Preset list in dropdown; optional second screenshot of one preset loaded with key knobs visible.]**

---

## 11. Tips & Techniques

**Reese bass**  
Start from **INIT / REESE**. Slight detune and movement come from **Feedback** + **Time** + **Resonance**. Use **Morph** or **Vowel** for slow movement. Keep **Sub** and **X-OVER** so the bottom stays solid.

**Growl / vowel**  
Switch to **VOWEL** mode. Set **Resonance** high (70–90%). Sweep **Morph** for A–E–I–O–U. **Cutoff** scales the formants (lower = darker vowel). **Drive** and **Stages** add grit.

**Plasma / unstable**  
Use **PLASMA FEEDBACK** or high **Feedback**, medium **Time**, and (if your build exposes it) **Scramble/Plasma** for LFO-style modulation. **Resonance** and **Cutoff** tune the instability.

**Parallel saturation**  
Set **Dry/Wet** to 50–70% so the dry signal stays present. Increase **Drive** and **Stages** on the wet side for punch without losing clarity.

**Sub-safe heavy distortion**  
Keep **X-OVER** at 80–120 Hz and **Sub** at 100% or higher. Crank **Drive** and **Stages** on the highs; the sub stays clean and punchy.

**Stereo width**  
Use **Width** on the high band only; keep **Sub** mono for compatibility. Try **NEURONAL WIDTH** and adjust to taste.

**[SCREENSHOT: Example — Vowel mode with Resonance high and Morph mid-sweep.]**

---

## 11.1 Understanding the Orb and Visuals

The **central Orb** is a real-time visualization that reacts to your **Drive** and **Morph** (and input level). It does not affect the sound; it gives you a quick read of how “hot” and how “morphed” the signal is. When Drive is low, the orb is calmer; when Drive and level are high, it becomes more active and bright. The **Transfer** curve (small graph near the algorithm selectors) shows the distortion transfer function: input level on the horizontal axis, output on the vertical. As you change Drive, Fold, or algorithms, the curve updates. Use it to see how hard you’re pushing the nonlinearity. The **Reactor Tank** (vertical bar with 1–12 segments) shows the current **Stages** value; filled segments indicate how many times the signal is re-processed. The **scope** at the bottom displays the output waveform so you can confirm signal flow and level.

**[SCREENSHOT: Orb at low drive vs high drive; optional: Transfer curve and Reactor Tank annotated.]**

---

## 11.2 Workflow Suggestions

- **Sound design:** Start from INIT or a preset close to what you want. Tweak **algorithms** first (Positive/Negative), then **Drive** and **Stages**, then **Filter** (Cutoff, Res, Morph/Vowel). Add **Feedback** and **Noise** last for movement and texture.
- **Mixing:** Use **Dry/Wet** at 30–70% for parallel saturation. Set **X-OVER** and **Sub** so the low end of the mix stays clean. Use **GAIN** to match level when the effect is on/off.
- **Live or performance:** Map **Drive**, **Morph**, **Cutoff**, or **Dry/Wet** to a controller. The Orb and scope give visual feedback so you can perform without staring at numbers.

---

## 12. Technical Specifications

- **Plugin formats:** VST3, AU (Mac), Standalone.
- **Operating systems:** macOS 10.14+, Windows 10+ (64-bit).
- **Sample rate:** Host-dependent (typically 44.1–192 kHz).
- **Block size:** Host-dependent.
- **Oversampling:** 4× on the high band (distortion path).
- **Crossover:** Linkwitz–Riley 4th order (24 dB/oct), phase-matched.
- **Filter:** TPT (topology-preserving) SVF; Morph (LP/BP/HP) and Formant (5 vowels) modes.
- **Resonator:** Tuned delay line with feedback; modulatable time (e.g. via LFO/Plasma in engine).
- **Latency:** Depends on oversampling and host; typically low. Check your DAW’s reported plugin delay.
- **Presets:** 13 factory presets; state is saved with the host project (parameter automation and preset handling are host-dependent).

---

## 13. Roadmap & Future Plans

*The following reflects possible directions; priorities may change.*

- **More algorithms:** Additional bipolar distortion types and possibly user-orderable stages.
- **Space / diffusion:** Full implementation of the **Space** control (e.g. allpass/diffusion in the feedback path).
- **Scramble/Plasma UI:** Expose **Scramble (Plasma)** in the interface if not already present, for modulated filter/feedback.
- **Preset management:** Save/load user presets, import/export, and possibly preset categories.
- **Visuals:** More optional visualizations (spectrum, phase, or extra orb modes).
- **Performance:** Optional quality/CPU modes (e.g. 2× vs 4× oversampling) for weaker machines.
- **Documentation:** Video tutorials and more preset packs.

We want AETHER to stay a single, focused multiband distortion workstation while growing in depth and usability.

---

## 14. Troubleshooting

**No sound when I enable the plugin**  
- Check **Dry/Wet** (0% = only dry; try 100%).  
- Check **GAIN** (not at −24 dB).  
- Confirm the track has input and is not muted.  
- In **Vowel** mode, very high **Resonance** and wrong **Cutoff** can make the formant very quiet—lower Resonance or switch to **Morph** to test.

**Vowel mode is very quiet**  
- Vowel (formant) filter is very selective; we added gain compensation. If it’s still too quiet, raise **GAIN** or **Drive** slightly, or lower **Resonance** so more signal passes.

**Plugin not found in my DAW**  
- Install VST3/AU to the correct folders and **rescan** plugins in your DAW. On Mac, grant the DAW “Full Disk Access” if it can’t see the Components folder.

**Crackling or dropouts**  
- Reduce **Stages** (e.g. 1–4) or lower oversampling in a future build. Close other heavy plugins or increase buffer size.

**Sub is too quiet or too loud**  
- Adjust **Sub** level and **X-OVER**. Higher **X-OVER** sends more low-mids into the effect; lower keeps more true sub in the clean path.

**Preset doesn’t sound like the name**  
- Presets don’t store Noise, Width, X-OVER, or Output. Set those manually. Also check your source material and level.

---

## Appendix A: Parameter Quick Reference

| Parameter      | Range / Type   | Default (approx) | Main use                          |
|----------------|----------------|------------------|-----------------------------------|
| Drive          | 0–1            | 0.5              | Input gain into distortion        |
| Fold           | 0–1            | 0                | Sine wavefolder amount            |
| Stages         | 1–12           | 1                | Number of distortion passes       |
| Positive Algo   | Choice         | SoftClip         | Saturation for positive half      |
| Negative Algo  | Choice         | SoftClip         | Saturation for negative half      |
| Cutoff         | 80 Hz–20 kHz   | 20 kHz           | Filter frequency                  |
| Resonance      | 0–1            | 0.2              | Filter Q                          |
| Morph          | 0–1            | 0                | Filter shape or vowel             |
| Filter Mode    | Morph / Vowel   | Morph            | Filter type                       |
| Feedback       | 0–1.1          | 0                | Resonator feedback amount         |
| Time           | 0.1–500 ms     | 20 ms            | Resonator delay time              |
| Space          | 0–1            | —                | Feedback diffusion/space         |
| Sub            | 0–2            | 1                | Sub band level                    |
| X-OVER         | 60–300 Hz      | 150 Hz           | Crossover frequency               |
| Squeeze        | 0–1            | 0.4              | OTT-style compression             |
| Width          | 0–1.5          | 0                | Stereo width (high band)          |
| Output (GAIN)  | −24 to +24 dB  | 0 dB             | Output level                      |
| Dry/Wet        | 0–1            | 1                | Mix dry/wet                       |
| Noise Level    | 0–1            | 0                | Noise injection amount            |
| Noise Width    | 0–1            | 1                | Noise stereo width                |
| Noise Type     | White/Pink/Crackle | White        | Noise spectrum                    |

---

## Appendix B: Glossary

- **Bipolar distortion:** Using different saturation algorithms for the positive and negative parts of the waveform to create asymmetric harmonics.
- **Crossover:** A filter that splits the signal into low (sub) and high bands. AETHER uses a Linkwitz–Riley 4th-order crossover.
- **Dry/Wet:** The blend between the unprocessed (dry) and fully processed (wet) signal.
- **Formant / Vowel:** Filter mode that emphasizes vowel-like resonances (A, E, I, O, U) for talking or growling tones.
- **Morph (filter):** Sweeping between low-pass, band-pass, and high-pass filter types.
- **Multiband:** Processing different frequency bands separately (here: sub vs highs).
- **OTT:** “Over the top” style upward/downward compression; AETHER’s Squeeze control is in this spirit.
- **Reactor / Stages:** Re-processing the signal through the distortion multiple times (1–12) for denser saturation.
- **Resonator:** A tuned delay line with feedback; in AETHER it creates metallic or screaming resonances.
- **Sub:** The low band below the crossover; kept relatively clean and mono in AETHER.
- **Transfer curve:** A graph of input level vs output level for the distortion; shows how hard the signal is being driven.
- **Wavefolder:** Nonlinearity that “folds” peaks (e.g. with a sine) instead of hard clipping, adding harmonics.

---

## 15. Credits & Support

**AETHER 3.0**  
- **Design & development:** Antigravity / Æther Labs.  
- **Architecture:** Multiband crossover, bipolar distortion, TPT filter, resonator, dimension, noise, squeeze.  
- **Framework:** JUCE.  
- **Manual:** This document; screenshot placeholders are for you to replace with actual images.

**Support**  
- For bugs, feature ideas, or presets: use your project’s issue tracker or contact channel (e.g. GitHub, Discord, or email if provided).  
- When reporting bugs, please include: OS, host, AETHER version, and steps to reproduce.

**Thanks**  
- Everyone who tests, uses, and gives feedback on AETHER. Your input shapes what comes next.

---

*End of manual. Replace every **[SCREENSHOT: …]** with a real screenshot for your final 20+ page PDF or web export. For PDF, use a Markdown-to-PDF tool (e.g. Pandoc, Typora, or a static site generator) and ensure images are high resolution for print.*
