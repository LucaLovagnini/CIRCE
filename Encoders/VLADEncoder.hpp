/*
 * VLADEncoder.hpp
 *
 *  Created on: 16 Dec 2016
 *      Author: luca
 */

#ifndef VLADENCODER_HPP_
#define VLADENCODER_HPP_

#include "Encoders/Encoder.hpp"

extern "C" {
	#include "vl/kmeans.h"
	#include "vl/vlad.h"
}

namespace cc{
	class VLADEncoder : public Encoder{
	public:
		VLADEncoder(vl_size numCenters = 16, vl_size dimensions = 128, std::string flags = "VL_VLAD_FLAG_NORMALIZE_COMPONENTS");
		void train_(cv::Mat1f &trainData);
		void encode_(cv::Mat1f &descriptors, cv::Mat1f &code);


		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void writeTrainer(const std::string &trainerFile);
		void readTrainer(const std::string &trainerFile);
		vl_size getNumCenters();
		vl_size getDimensions();
		vl_size getFlags();
		void printTrainer();
	private:

		// private function
		void computeLCS(cv::Mat1f &descriptors, vl_uint32 *index);
		// will do copy of matrix, fix later
		cv::Mat1f computeVLAD(cv::Mat1f &descriptors, vl_uint32 *index);
		float norm( cv::Mat1f mat );

		VlKMeans * kmeans = vl_kmeans_new (VL_TYPE_FLOAT,VlDistanceL2) ;
		vl_size numCenters;
		int flags;
		// store the pre-learned rotation matrix for LCS.

		// each j-th row of rotationMatrixs[i] is used to calculate the j-th component after PCA
		// if data is *subtracted* by mean and the size of data is 1 row, n col
		// just data.row(0).dot(rotationMatrixs[i].row(j)) to get the j-th component after PCA
		std::vector<cv::Mat1f> rotationMatrices;
		std::vector<cv::PCA> LCSPCAs;
		// EigenvalueArrayOfRotaionMatrixs[i] is a d rows, 1 col matrix representing d eigenvalues of i-th center
		std::vector<cv::Mat1f> EigenvalueArrayOfRotaionMatrixs;
	};
}

#endif /* VLADENCODER_HPP_ */
