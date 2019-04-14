/*
 * Euclidean.hpp
 *
 *  Created on: 8 Mar 2017
 *      Author: luca
 */

#ifndef UTILITIES_EUCLIDEANDISTANCE_HPP_
#define UTILITIES_EUCLIDEANDISTANCE_HPP_

#include "opencv2/core/core.hpp"

#include "Distance.hpp"

namespace cc{
	class EuclideanDistance : public Distance<cv::Mat1f, float>{
		float compute(const cv::Mat1f &first, const cv::Mat1f &second);
	};
}


#endif /* UTILITIES_EUCLIDEANDISTANCE_HPP_ */
