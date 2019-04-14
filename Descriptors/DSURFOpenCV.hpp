/*
 * DSURFOpenCV.hpp
 *
 *  Created on: 6 Mar 2017
 *      Author: luca
 */

#ifndef DSURFOPENCV_HPP_
#define DSURFOPENCV_HPP_

#include "Descriptors/Descriptor.hpp"
#include "opencv2/xfeatures2d.hpp"


namespace cc{
	class DSURFOpenCV : public Descriptor {
	public:
		DSURFOpenCV (int step, double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright);
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors);
	private:
		cv::Ptr<cv::xfeatures2d::SURF> surf;
		int step;
	};
}

#endif /* DSURFOPENCV_HPP_ */
