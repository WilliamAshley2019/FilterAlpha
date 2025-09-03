FilterAlpha

FilterAlpha is a TB-303-style 4-pole diode-ladder filter VST3 plug-in for Windows.
Each build is organized into its own subfolder (e.g., FilterAlphaThree) for clarity and versioning.
Processes stereo audio in real-time; no MIDI required.

ALPHA SOFTWARE NOTICE:
These are alpha versions provided for testing and experimentation. Minimal testing has been done, and stability is not guaranteed. Use at your own risk.

Features:
- An attempt at an authentic 24 dB/oct TB-303 resonant low-pass ladder
- Additional modes: LP 24 dB, LP 18 dB, LP 12 dB, HP 12 dB, Flat bypass
- Pre- and post-filter saturation with ±24 dB "Drive"
- High-pass feedback path with independent cutoff and amount
- Double-precision internal processing, zero-latency
- Fully-automatable parameters

Quick Start:
1. Copy the .vst3 file from the repository subfolder (e.g., FilterAlphaThree)  %COMMONPROGRAMFILES%\VST3
   2. Re-scan your DAW and load FilterAlpha on any stereo track

Note: Since these are alpha builds each is being given its own folder with new featuers only added in newer versions. Any fixes to this version will be to address bugs
or improve things like CPU efficency or compatibility issues.

Building from Source:

Prerequisites:
- CMake ≥ 3.22
- C++20 compiler (This build using Visual Studio 2022 V17)
- JUCE 8.0.7 
- VST3 SDK 3.7.11 


Parameter Reference:

Parameter       Range        Description
---------       -----        -----------
Cutoff          20–20 kHz    Corner frequency
Resonance       0–100 %      Emphasis amount
Drive           -24 … +24 dB Pre-filter gain
Mode            6 choices    Filter topology
Feedback HP     20–20 kHz    High-pass in feedback loop
Feedback Amp    0–100 %      Amount of feedback
Automation Mode On/Off       Smoother parameter smoothing for automation

License & 3rd-Party Notices:
- This project is released under the GPL-3.0: https://www.gnu.org/licenses/gpl-3.0.html
- JUCE: Built with JUCE 8.0.7. Licensing terms (GPL v3 or commercial) apply.
  License file included at: modules/JUCE/LICENSE.md
  More info: https://juce.com/
- VST3 SDK: © Steinberg. Redistribution must comply with the VST3 License.
  License file included at: modules/vst3sdk/LICENSE.txt
  More info: https://developer.steinberg.help/display/VST/License
- TB-303 Reference: Filter inspired / modeled after the Roland TB-303 diode ladder.
  Roland is not affiliated with this project; "TB-303" is descriptive only.
Use of TeeBee303 is in part due to trying to adapting TeeBee 303 free source solutions to some issues
faced early into the build. 


Acknowledgements:
- JUCE community for the excellent framework: https://juce.com/
 I will link back to jc303 because it was helpful to learn from in solving a few early issues in FilterAlphaOne and FilterAlphaTwo 
https://github.com/midilab/jc303

Support / Bugs:
Open an issue on the GitHub tracker: https://github.com/WilliamAshley2019/FilterAlpha/issues

Because I have the intent to continue to work on new versions of this plugin adding more functionality  I will have each version as a seperate folder they are being based off
the same code just developed a little further. I consider FilterAlphaTHree to have met the intial goals so that is why it is being upped.
