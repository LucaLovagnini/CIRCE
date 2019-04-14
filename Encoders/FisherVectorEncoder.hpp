/*
 * FisherVectorEncoder.hpp
 *
 *  Created on: 30 Dec 2016
 *      Author: luca
 */

#ifndef FISHERVECTORENCODER_HPP_
#define FISHERVECTORENCODER_HPP_

#include "Encoders/Encoder.hpp"

extern "C" {
	#include <vl/generic.h>
	#include <vl/gmm.h>
	#include <vl/fisher.h>
}

namespace cc{
	class FisherVectorEncoder : public Encoder {
	public:
		FisherVectorEncoder (vl_size numCenters = 16, vl_size dimensions = 128, std::string flags = "VL_FISHER_FLAG_IMPROVED");
		void train_ (cv::Mat1f &trainData);
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void encode_ (cv::Mat1f &descriptors, cv::Mat1f &code);
		void writeTrainer(const std::string &trainerFile);
		void readTrainer(const std::string &trainerFile);
		std::string getParameters();
		void printTrainer();
	private:
		vl_size numCenters;
		int flags;
		VlKMeans *kmeans;
		VlGMM *gmm;
		const void *covariances = NULL, *means= NULL, *priors= NULL;
	};
}

#endif /* FISHERVECTORENCODER_HPP_ */
