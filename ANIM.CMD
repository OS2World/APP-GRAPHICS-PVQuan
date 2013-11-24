rem 4OS/2 command file to generate an animation file
: Usage:   anim filename

: expects filename.var and filename.pov to be present in the current directory
: expects animdat.exe, povray.exe, heckbert.exe, animfli.exe to be in a directory in PATH
: creates then deletes filename_xxx.pov, filename_xxx.tga, filename.xxx
:                         where xxx is in the range 0..??
: creates filename.fli

echo off
path %path%;e:\graphics
if not exist %1_0.pov animdat %1
*set i=0
:LOOP
   echo %1_%i.pov
   if not exist %1_%i.pov goto QUANTISE
   if %@FILESIZE[%1_%i.tga,b] GT 192000 goto SKIP
   povray -i%1_%i.pov -ld:\graphics\ray\pov\include +a -q9 +ft -w320 -h200 -c -v +d
:SKIP
   if %@FILESIZE[%1_%i.tga,b] GT 19200 del %1_%i.pov
   *set i=%@EVAL[%i + 1]
   goto LOOP

:QUANTISE
rem octree -t=1 -i=1 -n=%i %1
heckbert -t=1 -i=1 -s=0 -n=%i %1
if exist %1.0 del %1*.tga
animfli %1 %1
if exist %1.fli del %1.[0-9]*

