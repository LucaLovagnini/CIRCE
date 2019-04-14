/*
 * DSIFTVLFeat.cpp
 *
 *  Created on: 10 Jan 2017
 *      Author: luca
 */

#include "DSIFTVLFeat.hpp"
#include <iostream>

extern "C" {
	#include "vl/dsift.h"
}
namespace cc{
	DSIFTVLFeat::DSIFTVLFeat(int step, int binSize) : step(step), binSize(binSize) {maxBinSize = binSize; }
	DSIFTVLFeat::DSIFTVLFeat(int step, int binSize, int maxBinSize) : step(step), binSize(binSize), maxBinSize(maxBinSize) {}

	void DSIFTVLFeat::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		params.push_back(std::pair<std::string,std::string>("Descriptor","DSIFTVLFeat"));
		params.push_back(std::pair<std::string,std::string>("step",std::to_string(step)));
		params.push_back(std::pair<std::string,std::string>("binSize",std::to_string(binSize)));
		params.push_back(std::pair<std::string,std::string>("maxBinSize",std::to_string(maxBinSize)));
	}

	void DSIFTVLFeat::ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors){
		descriptors.release();

		// transform image in cv::Mat to float vector
		cv::Mat imgFloat;
		img.convertTo(imgFloat, CV_32F, 1.0/255.0);
		if(!imgFloat.isContinuous())
			throw std::runtime_error("imgFloat is not continous");

		for(int i=binSize; i<=maxBinSize; i+=2){
			VlDsiftFilter *dsift = vl_dsift_new_basic (img.rows, img.cols, step, i);
			vl_dsift_process (dsift, imgFloat.ptr<float>());
			cv::Mat scaleDescs(vl_dsift_get_keypoint_num(dsift), 128, CV_32F, (void*) vl_dsift_get_descriptors(dsift));
			descriptors.push_back(scaleDescs);
			vl_dsift_delete (dsift);
		}
	}
}
