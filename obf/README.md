# ithare::obf
C++ Code+Data Obfuscation Library with Build-Time Polymorphism

**Primary Target Audience**: Primarily, the library is intended for **multiplayer game developers concerned about bots**. Use of the library to protect programs from piracy, while potentially possible (especially if speaking about the programs with online protection), is not among the goals (and while it might improve things, IMO anti-piracy fight will be still next-to-hopeless; in contrast - anti-bot fight DOES have a fighting chance). **ithare::obf aims to withstand the-best-available reverse-engineering tools such as IDA Pro's decompiler** (see also [first results](#first-results) below). 

**Secondary Target Audience**: C++ compiler writers (and those willing to learn new ways to abuse the compiler &lt;wink /&gt;). Apparently, ithare::obf tends to generate LOTS of ugly code (and can generate even-uglier-code if desired &lt;wink /&gt;); as such, it puts optimizer under significant stress (with almost no efforts to generate testing code, as generated code is supposed to be functionally-equivalent to the original one). In particular, even without aiming to test the compiler, I already ran into a code-generation bug with one of the intrinsics, and one case of compile-in-Release-mode taking atrociously long times (plus who-knows-how-many INTERNAL COMPILER ERRORs induced by my own mistakes). 

**The Big Idea**: Automatically generated obfuscation code+data code while leaving the source (mostly) readable, with obfuscation only *declared* in the source. Obfuscation code is heavily randomized depending on ITHARE_OBF_SEED macro, so changing it causes a major reshuffling even if the source code is 100% the same. 

&nbsp;

**Rationale**: See http://ithare.com/bot-fighting-201-declarative-datacode-obfuscation-with-build-time-polymorphism-in-c/ and http://ithare.com/bot-fighting-201-part-2-obfuscating-literals/ 

**Implementation**: IMO - it is a rather interesting exercise in C++-provided code generation. We have a template class obf_injection<>, with OBFCYCLES (meaning "CPU cycles we're allowed to burn") being one of the parameters, and OBFSEED being another one; obf_injection<> randomly chooses which of obf_injection_version<>'s to use - and obf_injection_version<> does some version-specific obfuscation stuff, and instantiates another obf_injection<> but with less CYCLES left. This template recursion goes on until CYCLES goes to zero. That's pretty much it, the rest is about some basic maths and some basic template+constexpr C++17 programming.
* NB: while formally, it is probably possible to implement the same thing under C++11 (and maybe even with C++03) - without proper support for constexprs I wouldn't be able to write it, so **C++17 is THE MUST** (="I am not going to support anything lower than that"). 

<a name="first-results"></a>
**First Results**: http://ithare.com/bot-fighting-201-part-3-ithareobf-an-open-source-datasource-randomized-obfuscation-library

&nbsp;

**Requirements**: v0.01 - Visual Studio 2017 with /std:c++latest (and /cgthreads1 recommended as a workaround for a suspected bug in MSVC linker); for v0.02 support for Clang (with -std=c++1z) is being added

**Status**: v0.01 is PRE-ALPHA (actually - more like proof of concept). No known bugs, but testing was very limited, and probably there are still MANY issues (mostly with refusing to compile). 

**Current Work**: v0.02 - a LOT of structural improvements coming (currently being developed in [develop branch](https://github.com/ITHare/obf/tree/develop), though still WIP and may occasionally ), hopefully to become a viable "alpha". LOTS of improvements, such as added support for operations without complete restore of the value (proof of concept DONE, to be extended), support for real injections (not just bijections) - DONE, support for Clang/Mac (DONE), automated randomized testing (1st version DONE), a way to debug compile-time failures (TODO), significantly improved PRNGs (with an option to use 128-bit keys and real compile-time encryption) - DONE, support for different variations of the same injection (DONE), "official" way to add your own injections (TODO), and probably something-else-I-forgot-about.

**How to Help**: Until v0.02 is out - there isn't much to delegate, but **if you're interested in the library - PLEASE leave a comment** on http://ithare.com/bot-fighting-201-part-3-ithareobf-an-open-source-datasource-randomized-obfuscation-library encouraging me to work on ithare::obf harder (I am serious, there is no point in doing things if there is little interest in using them). After v0.02 is out, ideas and implementations of new injections and various generated trickery would be possible (and certainly VERY welcome; the whole point of this project is in having LOTS of different blocks-to-build-randomized-code-from). 
