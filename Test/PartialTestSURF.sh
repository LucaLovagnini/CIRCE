#!/bin/bash
# since Bash v4

startHessian=100
maxHessian=600

startOctaves=3
maxOctaves=6

startLayers=2
maxLayers=5

OUTPUT="$HOME/Documents/CloudCacheOutput"

for ((layers = 4; layers <= maxLayers; layers = layers+1)); do
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 5 $layers 0 1 sampled 0 dataset oxford
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 5 $layers 0 1 sampled 0 dataset holiday
done


for ((octaves = 6; octaves <= maxOctaves; octaves = octaves+1)); do
for ((layers = $startLayers; layers <= maxLayers; layers = layers+1)); do
	
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 $octaves $layers 0 1 sampled 0 dataset oxford
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 $octaves $layers 0 1 sampled 0 dataset holiday
done
done


for ((hessian = 500; hessian <= maxHessian; hessian = hessian+100)); do
for ((octaves = $startOctaves; octaves <= maxOctaves; octaves = octaves+1)); do
for ((layers = $startLayers; layers <= maxLayers; layers = layers+1)); do
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV $hessian $octaves $layers 0 1 sampled 0 dataset oxford
	rm "$OUTPUT"/*.bin
	../make/CloudCache resizeDim 500  OMP 0 VLAD 64 64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV $hessian $octaves $layers 0 1 sampled 0 dataset holiday
done
done
done

