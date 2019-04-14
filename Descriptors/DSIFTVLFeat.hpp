/*
 * DSIFT.hpp
 *
 *  Created on: 10 Jan 2017
 *      Author: luca
 */

#ifndef DSIFTVLFEAT_HPP_
#define DSIFTVLFEAT_HPP_

#include "Descriptors/Descriptor.hpp"

namespace cc{
	class DSIFTVLFeat : public Descriptor{
	public:
		DSIFTVLFeat(int step, int binSize);
		DSIFTVLFeat(int step, int binSize, int maxBinSize);
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors);
	private:
		int step, binSize, maxBinSize;
	};
}

#endif /* DSIFTVLFEAT_HPP_ */
