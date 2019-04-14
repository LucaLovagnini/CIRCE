/*
 * SIFTVLFeat.hpp
 *
 *  Created on: 24 Dec 2016
 *      Author: luca
 */

#ifndef SIFTVLFEAT_HPP_
#define SIFTVLFEAT_HPP_


#include "Descriptors/Descriptor.hpp"

extern "C" {
#include <vl/generic.h>
#include <vl/sift.h>
#include <vl/mathop.h>
}

namespace cc{
	class SIFTVLFeat : public Descriptor{
	public:
		void mapParams(std::vector<std::pair<std::string,std::string>> &params);
		void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors);
	};

	typedef struct vl_sift_options
	{
		vl_sift_options(): verbose(1),
							O(-1),
							S(3),
							o_min(0),
							edge_thresh(-1),
							peak_thresh(-1),
							norm_thresh(-1),
							magnif(-1),
							window_size(-1),
							force_orientations(0),
							floatDescriptors(0){ }   // default Constructor

		int                verbose;
		int                O; //Octaves
		int                S; //Levels
		int                o_min;
		double             edge_thresh;  //-1 will use the default (as in matlab)
		double             peak_thresh;
		double             norm_thresh;
		double             magnif;
		double             window_size;
		vl_bool            force_orientations;
		vl_bool            floatDescriptors;
	} vl_sift_options;
}

#endif /* SIFTVLFEAT_HPP_ */
