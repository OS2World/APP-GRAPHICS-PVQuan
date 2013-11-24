: Usage:   anim.sh filename

: expects filename.anm and filename.dat to be present
: creates then deletes filename_xxx.dat, filename_xxx.tga, filename_0.xxx
:                         where xxx is in the range 0..??
: creates filename.fli

animdat $1
i=0
echo $1_$i.pov
while test -r $1_$i.pov
do
	nice povray -i$1_$i.pov -o$1_$i.tga /usr/export/home/kai/f_vander/Pov/$2.def
	times
	rm $1_$i.pov
	i="`expr $i + 1`"
done
echo Raytracing finished
times
nice heckbert -t=1 -i=1 -s=0 -n=$i $1
rm $1*.tga
animfli $1 $1
rm $1.[0-9]*
mv $1.fli /usr/export/home/kai/f_vander/Down
times
