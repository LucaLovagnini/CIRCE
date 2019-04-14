/*
 * DSURFOpenCV.cpp
 *
 *  Created on: 6 Mar 2017
 *      Author: luca
 */

#include "DSURFOpenCV.hpp"

namespace cc{
	DSURFOpenCV::DSURFOpenCV(int step, double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright) : step(step){
		surf = cv::xfeatures2d::SURF::create(hessianThreshold, nOctaves, nOctaveLayers, extended, upright);
	}

	void DSURFOpenCV::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		params.push_back(std::pair<std::string,std::string>("Descriptor","DSURFOpenCV"));
		params.push_back(std::pair<std::string,std::string>("step",std::to_string(step)));
		params.push_back(std::pair<std::string,std::string>("hessianThreshold",std::to_string(surf->getHessianThreshold())));
		params.push_back(std::pair<std::string,std::string>("nOctaves",std::to_string(surf->getNOctaves())));
		params.push_back(std::pair<std::string,std::string>("nOctavesLayers",std::to_string(surf->getNOctaveLayers())));
		params.push_back(std::pair<std::string,std::string>("extended",std::to_string(surf->getExtended())));
		params.push_back(std::pair<std::string,std::string>("upright",std::to_string(surf->getUpright())));
	}

	void DSURFOpenCV::ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors){
		descriptors.release();
		int startSize = step < 8 ? 8 : step;
		std::vector<cv::KeyPoint> kps;
		for(int z=startSize; z<=startSize*5;z=z+startSize)
    		for (int i=step; i<img.rows-step; i+=step)
    			for (int j=step; j<img.cols-step; j+=step)
					kps.push_back(cv::KeyPoint(float(j), float(i), float(z)));
		surf->compute(img,kps,descriptors);
	}

}
