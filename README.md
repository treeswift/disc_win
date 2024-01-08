# DisC Decompilation Wizard
The famous Borland C++ decompiler (Windows version).

## Original version

The original code of DisC (as still available at various online tool archives)
is checked in by the following commit (hereafter referred to as Original Commit):

```
dbf18ee008f0692890fadb54d73cd82906e0ce36
```

and tagged `1.0.0` (`git checkout 1.0.0`), hereafter referred to as Original Tag.

The original README [can be found here](readme.txt).

## Copyright

The following copyright notice is attached, and must stay attached to the code:
```
/*---------------------------------------------------------------------------*/
// DisC Decompilation Wizard
// written by
// Satish Kumar S
// satish@miel.mot.com
//
// Copyright 1999-2001 Satish Kumar S
//
// Permission is granted to use this software for research purposes as
// long as this notice stays attached to this software.
/*---------------------------------------------------------------------------*/
```
Abide by it, and be a good digital citizen.

## Porter's notice

Time flies, standards evolve. In order to make the venerable tool buildable with modern
compilers (such as MSVC, G++ or CLang) and usable in modern environments (our reference one
is currently Windows 10 for Intel/AMD x64 CPUs), changes to original source code, as well as additions
in the form of newly required metadata files, may need to be introduced. Such changes can
be differentiated from the original code by **Satish Kumar S** by comparing the working copy
state to the Original Commit (or, less strictly, to the Original Tag).

Consistent with our mission of digital preservation, we hereby state that (a) we RESERVE NO RIGHTS
pertinent to such changes and additions and (b) release all of the aforementioned changes and additions
into the public domain by applying the terms of the [Unlicense License](https://unlicense.org/UNLICENSE).

# Porting history

## Changelog

* The VC 6.0 project and solution (DSP+DSW) have been converted to MSVC 2019's SLN and VCXPROJ.

## Current state

Honestly, we just began.
