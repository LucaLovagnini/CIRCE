/*
 * OxfordTestManager.hpp
 *
 *  Created on: 20 Dec 2016
 *      Author: luca
 */

#ifndef OXFORDTESTMANAGER_HPP_
#define OXFORDTESTMANAGER_HPP_

#include "DatasetTest.hpp"

namespace cc{
	class OxfordTestManager : public DatasetTest {
	public:
		float computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK);
		float computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir);
		void getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir);
	};
}

#endif /* OXFORDTESTMANAGER_HPP_ */
