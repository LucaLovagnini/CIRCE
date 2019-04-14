/*
 * DatasetTest.hpp
 *
 *  Created on: 4 Mar 2017
 *      Author: luca
 */

#ifndef DATASETTEST_HPP_
#define DATASETTEST_HPP_

#include "opencv2/imgcodecs.hpp"

namespace cc{

	struct Rates{
		float tp=0, tn=0, fp=0, fn=0, queries=0;
	};

	struct queryResults{
						std::string query;
						std::vector<std::string> truePositives;
	};

	class DatasetTest{
	public:
		virtual float computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK = 0) = 0;
		virtual float computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir) = 0;
		static void getDataset(char *argv);
		void queryImage(const cv::Mat1f &codes, const std::string &dataDir, const std::string &queryImg, const std::string &ext = ".jpg");
		void Topk(	std::vector<std::string> &topkTest , std::vector<float> &topkDistance,
					const std::vector<std::string> files, const cv::Mat1f &queryCode, const cv::Mat1f &codes,
					size_t k, const std::string ext = ".jpg");
		virtual ~DatasetTest();
		virtual void getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir) = 0;
	};
}

#endif /* DATASETTEST_HPP_ */
