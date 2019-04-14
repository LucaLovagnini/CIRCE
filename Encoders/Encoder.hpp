/*
 * Encoder.hpp
 *
 *  Created on: 15 Dec 2016
 *      Author: luca
 */

#ifndef ENCODER_HPP_
#define ENCODER_HPP_

#include "opencv2/core.hpp"
#include <map>

extern "C" {
 	#include "vl/generic.h"
}

namespace cc{
	class Encoder {
	public:
		Encoder(vl_size codeSize, vl_size dimensions);
		void train(cv::Mat1f &trainDescriptors);
		void encode(cv::Mat1f &descriptors, cv::Mat1f &code);
		void encode(std::vector<cv::Mat1f> &descriptors, cv::Mat1f &codes);
		void applyLCSAndWhitening(cv::Mat1f &codeBefore, cv::Mat1f &codeAfter);
		virtual ~Encoder();
		virtual void mapParams(std::vector<std::pair<std::string,std::string>> &params) = 0;
		virtual void writeTrainer(const std::string &trainerFile) = 0;
		virtual void readTrainer(const std::string &trainerFile) = 0;
		virtual void printTrainer() = 0;
		vl_size codeSize;
		cv::PCA descriptorPCA;
	private:
		bool checkCode(cv::Mat1f &code);
		bool checkRows(cv::Mat1f &mat, int rows);
		bool checkDescriptors(cv::Mat1f &mat);
	protected:
		vl_size dimensions;
		virtual void train_(cv::Mat1f &trainDescriptors) = 0;
		virtual void encode_(cv::Mat1f &descriptors, cv::Mat1f &code) = 0;

	};
}
#endif /* ENCODER_HPP_ */

