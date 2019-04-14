/*
 * PaintingTest.hpp
 *
 *  Created on: 8 May 2017
 *      Author: luca
 */

#ifndef DATASETTESTS_CCDATASETTEST_HPP_
#define DATASETTESTS_CCDATASETTEST_HPP_


#include "DatasetTest.hpp"

namespace cc{
	class CCDatasetTest : public DatasetTest {
	public:
		CCDatasetTest(const std::string &gtQueryFile);
		float computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK);
		float computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir);
		void getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir);
		float getMAP();
	protected:
		std::string gtQueryFile;
	};
}


#endif /* DATASETTESTS_CCDATASETTEST_HPP_ */
