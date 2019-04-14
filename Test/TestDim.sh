#!/bin/bash
# since Bash v4

startDim=400
maxDim=1000

OUTPUT="$HOME/Documents/CloudCacheOutput"

for ((dim = $startDim; dim <= maxDim; dim = dim+100)); do
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim $dim  OMP 0 VLAD 64  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 dataset holiday
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim $dim  OMP 0 VLAD 64  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 dataset oxford
done
rm "$OUTPUT/*.bin"
../make/CloudCache resizeDim 0  OMP 0 VLAD 256  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0
