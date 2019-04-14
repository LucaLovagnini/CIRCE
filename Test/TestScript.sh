#!/bin/bash
# since Bash v4

startNumCenters=16
maxCenters=256

startDim=100
maxDim=800

for ((sampled = 0; sampled<=50; sampled=sampled+50));do

#SurfOpenCV VLAD
for ((upright = 0; upright <= 1; upright = upright+1)); do
for ((extended = 0; extended<=1; extended = extended+1));do
for ((omp = 0; omp <= 1; omp = omp+1)); do
	rm "$HOME""/Documents/CloudCacheOutput/DescTrain.bin"
	for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
		rm "$HOME""/Documents/CloudCacheOutput/Codes.bin"
		if [ $extended -eq 0 ];then
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP "$omp" VLAD "$centers"  64 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 "$extended" "$upright"
		else
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP "$omp" VLAD "$centers"  128 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" SURFOpenCV 400 4 3 "$extended" "$upright"
		fi
	     #delete VLAD codes
	done
done
done
done

#SurfOpenCV FisherVector
for ((upright = 0; upright <= 1; upright = upright+1)); do
for ((extended = 0; extended<=1; extended = extended+1));do
for ((omp = 0; omp <= 1; omp = omp+1)); do
	rm "$HOME""/Documents/CloudCacheOutput/DescTrain.bin"
	for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
		rm "$HOME""/Documents/CloudCacheOutput/Codes.bin"
		if [ $extended -eq 0 ];then
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP $omp FisherVector $centers  64 "VL_FISHER_FLAG_IMPROVED" SURFOpenCV 400 4 3 $extended $upright
		else
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP $omp FisherVector $centers  128 "VL_FISHER_FLAG_IMPROVED" SURFOpenCV 400 4 3 $extended $upright
		fi
	     #delete VLAD codes
	done
done
done
done

#SIFTOpenCV VLAD
for ((omp = 0; omp <= 1; omp = omp+1)); do
	rm "$HOME""/Documents/CloudCacheOutput/DescTrain.bin"
	for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
		rm "$HOME""/Documents/CloudCacheOutput/Codes.bin"
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP $omp VLAD $centers  128 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS"
	     #delete VLAD codes
	done
done

#SIFTOpenCV FisherVector
for ((omp = 0; omp <= 1; omp = omp+1)); do
	rm "$HOME""/Documents/CloudCacheOutput/DescTrain.bin"
	for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
		rm "$HOME""/Documents/CloudCacheOutput/Codes.bin"
		"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP $omp FisherVector $centers  128 "VL_FISHER_FLAG_IMPROVED"
	     #delete VLAD codes
	done
done

done
#DSIFT VLAD
for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
	"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP 1 VLAD $centers  128 "VL_VLAD_FLAG_SQUARE_ROOT|VL_VLAD_FLAG_NORMALIZE_COMPONENTS" DSIFTVLFeat 5 4 10
done
#DSIFT FisherVector
for ((centers = $startNumCenters; centers <= maxCenters; centers = centers * 2)); do
	"$CLOUD_CACHE"/make/CloudCache resizeDim 0  OMP 1 FisherVector $centers  128 "VL_FISHER_FLAG_IMPROVED" DSIFTVLFeat 5 4 10
done

