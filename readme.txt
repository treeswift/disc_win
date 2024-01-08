     DisC Decompilation Wizard
    ---------------------------

  You have downloaded the source for DisC Decompilation Wizard. This
archive contains the following files

	sample1.c
	sample2.c
	sample3.c
	sample4.c
	dv.cpp
	exe2sym.cpp
	udtypes.cpp
	tcwiz0.cpp
	variable.cpp
	tcwiz2.cpp
	tcwiz1.cpp
	strlist.cpp
	program.cpp
	mystring.cpp
	libwiz.cpp
	instinfo.cpp
	disc.cpp
	disasm.cpp
	beautify.cpp
	wizard.h
	variable.h
	udtypes.h
	tcwizard.h
	strlist.h
	program.h
	mystring.h
	libwiz.h
	instinfo.h
	global.h
	disc.h
	disasm.h
	beautify.h
	disc.dsw
	disc.dsp

  To build the executable, just open the file "disc.dsw" in Visual C++.

  Make sure you also have the TurboC version 2.01 compiler from Borland.
If you do not have this, please look at the Downloads section in the
DisC webpage at http://www.crosswinds.net/~sskr/dcompile/disc.htm.

  Before you execute DisC, compile the file "prepdisc.c" (available in
this archive itself) using TurboC and have the executable "prepdisc.exe"
in the same directory as the DisC executable.

  You can also copy the standard C libraries of TurboC (cs.lib) to the
DisC directory, so that DisC will do an automatic search for library
functions in the input executable file.

Command line
-------------
  The command line syntax is...

    disc <input_program.exe>

  The output is written to the file "_DISC.C" always.

Sample C programs
------------------
  Four sample C programs named "sample1.c".."sample4.c" are also included.
Compile each of them using TurboC and get the executables. These can then
be given to DisC for decompilation, and you can check the output.
