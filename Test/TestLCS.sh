#!/bin/bash
# since Bash v4


MAKEDIR="/home/luca/Dropbox/HKUST/CloudCache/cloudcache/CloudCache/make"
OUTPUT="/home/luca/Documents/CloudCacheOutput"

C=0

make clean -C "$MAKEDIR"

for ((VLFEAT = 0; VLFEAT <= 1; VLFEAT = VLFEAT+1));do
	if [ $VLFEAT -ne 1 ] 
	then
		for ((LCS = 0; LCS <= 1; LCS = LCS+1));do
		for ((RN = 0; RN <= 1; RN = RN+1));do
		for ((SSR = 0; SSR <= 1; SSR = SSR+1));do
		for ((INTRANORM = 0; INTRANORM <= 1; INTRANORM = INTRANORM+1));do
		for ((L2NORM = 0; L2NORM <= 1; L2NORM = L2NORM+1));do
			if [ $LCS -eq 1 ]
			then
				COMP="$COMP -DLCS"
			fi
			if [ $RN -eq 1 ]
			then
				COMP="$COMP -DRN"
			fi
			if [ $SSR -eq 1 ]
			then
				COMP="$COMP -DSSR"
			fi
			if [ $INTRANORM -eq 1 ]
			then
				COMP="$COMP -DINTRANORM"
			fi
			if [ $L2NORM -eq 1 ]
			then
				COMP="$COMP -DL2NORM"
			fi
			C=$((C+1))
			echo "$C comp= $COMP"
			rm "$MAKEDIR"/VLADEncoder.o "$MAKEDIR"/main.o "$OUTPUT"/*.bin
			make LCS="$COMP" -C "$MAKEDIR"
			"$MAKEDIR"/CloudCache resizeDim 500  OMP 0 VLAD 256  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 
			COMP=""
		done
		done
		done
		done
		done		
	else
			rm "$MAKEDIR"/VLADEncoder.o "$MAKEDIR"/main.o "$OUTPUT"/*.bin
			make LCS="-DVLFEAT" -C "$MAKEDIR"
			"$MAKEDIR"/CloudCache resizeDim 500  OMP 0 VLAD 256  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 
	fi
done
