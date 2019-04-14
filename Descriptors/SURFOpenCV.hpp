/*
 * SURFOpenCV.hpp
 *
 *  Created on: 23 Jan 2017
 *      Author: luca
 */

#ifndef SURFOPENCV_HPP_
#define SURFOPENCV_HPP_

#include <opencv2/xfeatures2d.hpp>

#include "Descriptors/Descriptor.hpp"

namespace cc{
	class SURFOpenCV : public Descriptor{
	public:
		SURFOpenCV(double hessianThreshold = 100, int nOctaves = 4, int nOctaveLayers = 3, bool extended = true, bool upright = false );
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors);
	private:
		cv::Ptr<cv::xfeatures2d::SURF> surf;
	};
}

#endif /* SURFOPENCV_HPP_ */
