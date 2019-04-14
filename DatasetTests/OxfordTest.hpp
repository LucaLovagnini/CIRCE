/*
 * OxfordTest.hpp
 *
 *  Created on: 8 Dec 2016
 *      Author: luca
 */


#ifndef OXFORDTEST_HPP_
#define OXFORDTEST_HPP_

#include <vector>
#include <set>

#include <boost/filesystem.hpp>

namespace fs = ::boost::filesystem;

class OxfordTest {
public:
	bool setDirectory (fs::path root);
	bool nextQuery(std::string &nextQuery);
	void computeAp (std::vector<std::string> &ranked_list);
	float getMAP();
	float getGoodRatio();
	std::set<std::string> getPosSet();
private:
	fs::directory_iterator itr;
	std::set<std::string> junk_set, pos_set;
	std::set<std::string> good_set, ok_set;
	float totalAP = 0;
	int nQueries = 0;
	// increased every time the closest image belongs to ok or good set
	int good = 0;
};
#endif /* FILESYSTEM_HPP_ */
