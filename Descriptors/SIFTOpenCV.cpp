/*
 * OpencvSIFT.cpp
 *
 *  Created on: 15 Dec 2016
 *      Author: luca
 */


#include "SIFTOpenCV.hpp"

namespace cc{
	SIFTOpenCV::SIFTOpenCV(int nFeatures, int nOctaveLayers, double contrastThreshold, double edgeThreshold, double sigma)
	: nFeatures(nFeatures), nOctaveLayers(nOctaveLayers), contrastThreshold(contrastThreshold), edgeThreshold(edgeThreshold), sigma(sigma){
		sift = cv::xfeatures2d::SIFT::create(nFeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
	}

	void SIFTOpenCV::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		params.push_back(std::pair<std::string,std::string>("Descriptor","SIFTOpenCV"));
		params.push_back(std::pair<std::string,std::string>("nFeatures", std::to_string(nFeatures)));
		params.push_back(std::pair<std::string,std::string>("nOctaveLayers", std::to_string(nOctaveLayers)));
		params.push_back(std::pair<std::string,std::string>("contrastThreshold", std::to_string(contrastThreshold)));
		params.push_back(std::pair<std::string,std::string>("edgeThreshold", std::to_string(edgeThreshold)));
		params.push_back(std::pair<std::string,std::string>("sigma", std::to_string(sigma)));
	}

	void SIFTOpenCV::ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors){
		descriptors.release();
		std::vector<cv::KeyPoint> pts;
		sift->detectAndCompute(img, cv::Mat(), pts, descriptors);
		//Apply SIFTRoot
		//L1 normalization of each descriptor
		if(pts.size()){
			// For each row
			for (int i = 0; i < descriptors.rows; ++i) {
			  // Perform L1 normalization
			  cv::normalize(descriptors.row(i), descriptors.row(i), 1.0, 0.0, cv::NORM_L1);
			}
			// Perform sqrt on the whole descriptor matrix
			cv::sqrt(descriptors, descriptors);
		}
	}
}
