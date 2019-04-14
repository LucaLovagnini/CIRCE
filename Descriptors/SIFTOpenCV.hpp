/*
 * OpenCVSIFT.hpp
 *
 *  Created on: 15 Dec 2016
 *      Author: luca
 */

#ifndef SIFTOPENCV_HPP_
#define SIFTOPENCV_HPP_

#include "opencv2/xfeatures2d.hpp"

#include "Descriptors/Descriptor.hpp"

namespace cc{
	class SIFTOpenCV : public Descriptor{
	public:
		SIFTOpenCV(int nFeatures=0, int nOctaveLayers=3, double contrastThreshold=0.04, double edgeThreshold=10, double sigma=1.6);
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors);
	private:
		cv::Ptr<cv::xfeatures2d::SIFT> sift;
		int nFeatures, nOctaveLayers;
		double contrastThreshold, edgeThreshold, sigma;
	};
}

#endif /* SIFTOPENCV_HPP_ */
