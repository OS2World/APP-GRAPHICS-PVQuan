: Usage:   trace1.sh povfile paramfile

: expects filename.anm and filename.dat to be present
: creates then deletes filename_0.tga
: creates filename.fli

nice povray -i$1.pov -o$1_0.tga /usr/export/home/kai/f_vander/Pov/$2.def
nice heckbert -t=2 -i=1 -w=320 -h=200 $1
rm $1_0.tga							
times
