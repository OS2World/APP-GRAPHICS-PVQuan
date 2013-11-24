Persistence of Vision Quantiser Utilities version 1.6
=====================================================

SOURCE CODE  (PVQUAN16.ZIP)
===========================

Description
~~~~~~~~~~~
This archive contains the source code for a set of programs to convert
image files produced by DKB-Trace or the Persistence of Vision raytracer
into a format suitable for display on a VGA screen, or to GIF format. It
includes programs to generate a single stereoscopic 3D image from two
views of a scene, as well as programs to produce animation from multiple
ray-traced images.

QUANT (HECKBERT & OCTREE):
  Colour quantisation program which post-processes raw image files
  produced by the Persistence of Vision Ray Tracer (PoV Ray). Two
  versions can be compiled: one uses Heckbert's median-splitting
  algorithm, whereas the other uses Octree quantisation.

DISPLAY:
  Displays either 320x400 or 320x200 images on a VGA screen, either 2D,
  or stereo 3D. 3D images require Sega 3D-glasses to be connected to the
  serial port for viewing.

ANIMDAT:
  Reads an animation source file (*.VAR), and uses it to generate data
  files for the PoV ray tracer.

ANIMFLI:
  Produces a .FLI file containing a series of images to be displayed by
  a .FLI player (e.g. Autodesk's AAPLAY).


PVQUAN16.ZIP Archive contents
=============================

This archive contains subdirectory information... unzip it using

PKUNZIP -d PVQUAN16

It will create a subdirectory tree in your current directory as follows:

pvquan 커               Sample files
        쳐 quant        Heckbert and Octree quantisers
        쳐 animdat      Animation Scene file generator
        쳐 giflib       GIF encode/decode library
        쳐 flilib       FLI creation library
        쳐 anim         FLI creation source code
        쳐 vga          VGA mode-X library
        읕 display      VGA 3D display

Each directory contains source code, plus make-files to produce
executable programs via MS-DOS, OS/2, or Unix.

You may also wish to create a BIN directory off the PVQUAN directory...
the sample make-files attempt to place executable programs there.

To create your own executables:

1.      In each directory, rename the appropriate version of the
        makefile to MAKEFILE (e.g. REN MAKEDOS MAKEFILE if compiling for
        DOS).

        Because both quantisers use common code, they are contained in
        the same directory. This causes some problems in terms of naming
        the makefiles.

        In MS-DOS, rename MAKEDOS.HEC to MAKEFILE to produce HECKBERT.EXE
                   rename MAKEDOS.OCT to MAKEFILE to produce OCTREE.EXE

        In OS/2,   rename MAKEFILE.OCT to MAKEFILE to produce OCTREE.EXE
                   rename MAKEHEC.DEP  to MAKEFILE.DEP to produce HECKBERT.EXE

        In Unix,   rename MAKEUNIX.HEC to MAKEFILE to produce HECKBERT
                   rename MAKEUNIX.OCT to MAKEFILE to produce OCTREE

2.      Edit each makefile to include the correct path to your compiler,
        linker, executable location, etc.

        In MS-DOS, all source files compile and link cleanly (no
        warnings, no errors) using Turbo C++ v1.0. Other versions of
        Turbo-C should also work. Some editing will be required for
        other compilers.

        In OS/2, all source files compile and link cleanly (no warnings,
        no errors) using IBM's C Set/2 v1.0. Some editing will be
        required for other compilers.

        In Unix, all source files compile and link cleanly (no warnings,
        no errors) using GNU's gcc on a Sun IPC SPARCstation. The
        previous version compiled cleanly under SCO's cc compiler on a
        486 PC, but this version hasn't been tested with that. Some
        editing will be required for other compilers.

3.      Create FLI, GIF, and VGA libraries via MAKE. N.B. The OS/2
        version does not use libraries. Instead the .OBJ files are
        linked in directly.

4.      Create executable programs via MAKE.

File Names
----------
pvquan/
pvquan/readme                   This file
pvquan/pvquan.doc               Documentation on quantisers, etc.
pvquan/bball.var                Bouncing ball animation
pvquan/bball.pov                Scene description for bouncing ball
pvquan/pacman.var               Pacman animation description
pvquan/pacman.pov               Scene description for Pacman animation
pvquan/pacman.iff               Part of Pacman scene
pvquan/trace.sh                 Unix script to trace a scene in background
pvquan/trace1.sh                Unix script (called by trace.sh)
pvquan/anim.sh                  Unix script to create an animation
pvquan/anim1.sh                 Unix script (called by anim.sh)
pvquan/anim.cmd                 OS/2 (4OS2) script to create an animation

pvquan/quant/                   Quantisers directory
pvquan/quant/heckbert.c         Heckbert quantisation
pvquan/quant/heckbert.h
pvquan/quant/octree.c           Octree quantisation
pvquan/quant/octree.h
pvquan/quant/virt_mem.c         Virtual memory (for TurboC & Heckbert)
pvquan/quant/quant.c            Quantiser Main program
pvquan/quant/quant.h
pvquan/quant/files.c            File handling
pvquan/quant/makeunix.oct       Unix Octree makefile
pvquan/quant/makeunix.hec       Unix Heckbert makefile
pvquan/quant/makedos.hec        Turbo C Heckbert makefile
pvquan/quant/link.hec           Turbo C Heckbert TLINK commands
pvquan/quant/makedos.oct        Turbo C Octree makefile
pvquan/quant/link.oct           Turbo C Octree TLINK commands
pvquan/quant/makefile.oct       OS/2 Octree makefile (rename to makefile)
pvquan/quant/makefile.dep       OS/2 Octree make dependencies
pvquan/quant/makehec.dep        OS/2 Heckbert dependencies (rename to makefile.dep)
pvquan/quant/makefile.hec       OS/2 Heckbert makefile (don't rename)

pvquan/animdat/                 Animation scene description generator
pvquan/animdat/readme.doc       Documentation for animdat
pvquan/animdat/animdat.doc      Documentation for animdat
pvquan/animdat/common.h
pvquan/animdat/error.h
pvquan/animdat/scanner.h
pvquan/animdat/btree.h
pvquan/animdat/symtab.h
pvquan/animdat/bball.pov
pvquan/animdat/bball.var
pvquan/animdat/scanner.c
pvquan/animdat/error.c
pvquan/animdat/btree.c
pvquan/animdat/animdat.c
pvquan/animdat/symtab.c
pvquan/animdat/makeunix         Unix makefile
pvquan/animdat/makeos2          OS/2 makefile
pvquan/animdat/makefile.dep     OS/2 make dependencies
pvquan/animdat/makedos          Turbo C makefile
pvquan/animdat/link.cmd         Turbo C TLINK commands

pvquan/giflib/                  GIF encode/decode library
pvquan/giflib/gif_hash.c
pvquan/giflib/gif_hash.h
pvquan/giflib/egif_lib.c
pvquan/giflib/gif_lib.h
pvquan/giflib/dgif_lib.c
pvquan/giflib/makeunix          Unix makefile
pvquan/giflib/makeos2           OS/2 makefile
pvquan/giflib/makefile.dep      OS/2 make dependencies
pvquan/giflib/makedos           Turbo C makefile

pvquan/flilib/                  FLI encode library
pvquan/flilib/aaerr.h
pvquan/flilib/aascreen.h
pvquan/flilib/aaflisav.h
pvquan/flilib/aatypes.h
pvquan/flilib/aafii.h
pvquan/flilib/aafli.h
pvquan/flilib/str_low.c         Low-level string handling (Unix, OS/2)
pvquan/flilib/str_low.asm       Low-level string handling (Turbo C)
pvquan/flilib/str_low.h
pvquan/flilib/compfram.c
pvquan/flilib/lccomp.c
pvquan/flilib/creatfli.c
pvquan/flilib/brun.c
pvquan/flilib/copyscre.c
pvquan/flilib/flierr.c
pvquan/flilib/memscree.c
pvquan/flilib/fccomp.c
pvquan/flilib/makeunix          Unix makefile
pvquan/flilib/makeos2           OS/2 makefile
pvquan/flilib/makefile.dep      OS/2 make dependencies
pvquan/flilib/makedos           Turbo C makefile
pvquan/flilib/flilib.rsp        Turbo C TLIB commands

pvquan/anim/                    FLI generator program
pvquan/anim/animfli.c
pvquan/anim/makeunix            Unix makefile
pvquan/anim/makeos2             OS/2 makefile
pvquan/anim/makefile.dep        OS/2 make dependencies
pvquan/anim/makedos             Turbo C makefile
pvquan/anim/link.cmd            Turbo C TLINK commands

pvquan/vga/                     VGA 320x200, 320x400 screen library (Turbo C)
pvquan/vga/vga.h
pvquan/vga/vga200.c
pvquan/vga/vga400.c
pvquan/vga/vga_pal.c
pvquan/vga/vga_retr.c
pvquan/vga/makefile             Turbo C makefile

pvquan/display/                 VGA display program (Turbo C)
pvquan/display/display.c
pvquan/display/makefile         Turbo C makefile


