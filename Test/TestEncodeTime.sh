#!/bin/bash
# since Bash v4

for ((run = 0; run <= 3; run = run+1)); do
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 0 1 sampled 0 dataset oxford
done
