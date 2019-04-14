/*
 * HolidayTest.hpp
 *
 *  Created on: 3 Mar 2017
 *      Author: luca
 */

#ifndef HOLIDAYTEST_HPP_
#define HOLIDAYTEST_HPP_


#include <vector>
#include <set>
#include <boost/filesystem.hpp>

#include "opencv2/highgui/highgui.hpp"

#include "DatasetTest.hpp"

namespace fs = ::boost::filesystem;

namespace cc{
	class HolidayTest : public DatasetTest {
	public:
		float computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK);
		float computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir);
		void getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir);
	private:
		std::set<std::string> junk_set, pos_set;
	};
}


#endif /* HOLIDAYTEST_HPP_ */
