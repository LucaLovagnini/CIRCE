/*
 * SURFOpenCV.cpp
 *
 *  Created on: 23 Jan 2017
 *      Author: luca
 */


#include "SURFOpenCV.hpp"
#include <sys/time.h>
#include "Utilities/Utility.hpp"

namespace cc{
	SURFOpenCV::SURFOpenCV(double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright){
		surf = cv::xfeatures2d::SURF::create(hessianThreshold, nOctaves, nOctaveLayers, extended, upright);
	}

	void SURFOpenCV::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		params.push_back(std::pair<std::string,std::string>("Descriptor","SURFOpenCV"));
		params.push_back(std::pair<std::string,std::string>("hessianThreshold",std::to_string(surf->getHessianThreshold())));
		params.push_back(std::pair<std::string,std::string>("nOctaves",std::to_string(surf->getNOctaves())));
		params.push_back(std::pair<std::string,std::string>("nOctavesLayers",std::to_string(surf->getNOctaveLayers())));
		params.push_back(std::pair<std::string,std::string>("extended",std::to_string(surf->getExtended())));
		params.push_back(std::pair<std::string,std::string>("upright",std::to_string(surf->getUpright())));
	}

	template <typename T> int sgn(T val) {
	    return (T(0) < val) - (val < T(0));
	}

	void SURFOpenCV::ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors){
		descriptors.release();
		std::vector<cv::KeyPoint> pts;
		surf->detectAndCompute(img, cv::Mat(), pts, descriptors);
		/*******************************************
		//Apply RootSURF
		//L1 normalization of each descriptor
		if(pts.size()){
			// For each row
			for (int i = 0; i < descriptors.rows; ++i) {
				// L1 normalization
				descriptors.row(i)/= cv::norm(descriptors.row(i), cv::NORM_L1);
				for (int j = 0; j<descriptors.cols; j++) {
					//Signed square rooting
					float val = descriptors.at<float>(i,j);
					if(val>0)
						descriptors.at<float>(i,j) = std::sqrt(val);
					if(val<0)
						descriptors.at<float>(i,j) = - std::sqrt(-val);
				}
				//descriptors.row(i)/= cv::norm(descriptors.row(i));
			}
		}

		********************************************/
	}
}
