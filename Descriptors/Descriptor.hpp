/*
 * Descriptor.hpp
 *
 *  Created on: 13 Dec 2016
 *      Author: luca
 */

#ifndef DESCRIPTOR_HPP_
#define DESCRIPTOR_HPP_

#include <opencv2/core.hpp>

namespace cc{
	class Descriptor{
	public:
		void ComputeDescriptors_(cv::Mat &img, cv::Mat1f &descriptors);
		virtual void ComputeDescriptors(const std::string &fileName, const std::string &imgExt, cv::Mat1f &descriptor);
		virtual void ComputeDescriptors(const std::string &dirPath, const std::string &imgExt, std::vector<cv::Mat1f> &descriptors);
		double getOnlyDescriptor();
		void setResizeDim(const size_t resizeDim);
		void setSamples (const size_t samples);
		void setOMP(const bool omp);
		virtual ~Descriptor();
		virtual void mapParams(std::vector<std::pair<std::string,std::string>> &params) = 0;
		int processedImages = 0;
		double onlyDescriptor = 0;
	protected:
		virtual void ComputeDescriptors(const cv::Mat &img, cv::Mat1f &descriptors) = 0;
	private:
		void ComputeDescriptorsRange(const std::vector<std::string> &files, std::vector<cv::Mat1f> &descriptors, const int start, const int finish, size_t errors);
		size_t resizeDim = 0;	//comput full-size image
		int samples = 0;
		bool omp = true;
		double onlyImage=0;
	};
}

#endif /* DESCRIPTOR_HPP_ */
