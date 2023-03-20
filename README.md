# Arguru Aodix

[Aodix](https://web.archive.org/web/20070819041559/http://www.aodix.com/pageaodixv4.html) was a music sequencer program for Windows developed by Arguru Software. It had a unique interface combining features of [trackers](https://en.wikipedia.org/wiki/Music_tracker) and [sequencers](https://en.wikipedia.org/wiki/Music_sequencer), as well as a modular wiring system for VSTs.

The developer of Aodix, [Juan Antonio Arguelles Rius ("Arguru")](https://en.wikipedia.org/wiki/Juan_Antonio_Arguelles_Rius), passed away in 2007. The final version v4.2.0.1 was released as freeware, however the source code was not published at the time.

I received permission from Zafer Kantar, who worked with Arguru on Aodix, to publish the code on GitHub. That code is available in the [archive repository](https://github.com/arguru-archive/aodix). This is my fork of the source code release, which I'll use for making additional fixes and improvements based on my work with the [executable patch](https://github.com/vanjac/aodix-repair/).

## Building

Aodix can be built using Visual Studio 2003 (run on Windows XP for best results). Instead of MSVC, Aodix was originally compiled with the [Intel C++ Compiler](https://en.wikipedia.org/wiki/Intel_C%2B%2B_Compiler). I wasn't able to find the correct version of this compiler used by Arguru; I used version 7.1.029 for the reconstruction, which produced similar but not identical binaries. MSVC can still be used as a substitute.

This repository does not include the [ASIO SDK](https://www.steinberg.net/developers/) (version 2.0) or the VST SDK (version 2.3, no longer available), since they are proprietary.
