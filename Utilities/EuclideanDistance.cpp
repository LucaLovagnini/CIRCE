/*
 * EuclideanDistance.cpp
 *
 *  Created on: 8 Mar 2017
 *      Author: luca
 */

#include "EuclideanDistance.hpp"

namespace cc{
	float EuclideanDistance::compute(const cv::Mat1f &first, const cv::Mat1f &second){
		return cv::norm(first,second,cv::NORM_L2);
	}
}
