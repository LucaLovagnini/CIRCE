#!/bin/bash
# since Bash v4

startExp=0.1
maxExp=1

MAKEDIR="$HOME/Dropbox/HKUST/CloudCache/cloudcache/CloudCache/make"
OUTPUT="$HOME/Documents/CloudCacheOutput"

exp=0.1
incr=0.1

make clean -C "$MAKEDIR"
for ((i = 0; i<9; i=i+1)); do
	exp=$(echo "scale=2;$exp+$incr"|bc)
	COMP="-DRN -DSSR -DEXP=$exp"
	rm "$MAKEDIR"/VLADEncoder.o "$MAKEDIR"/main.o "$OUTPUT"/*.bin
	make LCS="$COMP" -C "$MAKEDIR"
#	../make/CloudCache resizeDim 500 OMP 0 VLAD 64  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 dataset holiday
#	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500 OMP 0 VLAD 64  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 dataset oxford
done

COMP="-DRN -DSSR"
rm "$MAKEDIR"/VLADEncoder.o "$MAKEDIR"/main.o "$OUTPUT"/*.bin
make LCS="$COMP" -C "$MAKEDIR"

