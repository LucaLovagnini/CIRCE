/*
 * PaintingTest.cpp
 *
 *  Created on: 8 May 2017
 *      Author: luca
 */

#include "CCDatasetTest.hpp"

#include <string>

#include "Utilities/FileSystem.hpp"

namespace cc{

	CCDatasetTest::CCDatasetTest(const std::string &gtQueryFile) : gtQueryFile(gtQueryFile){}


	float CCDatasetTest::computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK){
		size_t k;
		if(!cutK)
			k = (size_t) codes.rows < 10000 ? (size_t) codes.rows : 10000;
		else
			k = cutK;
		float map = 0;
		std::vector<queryResults> queries;
		std::string trueGtQueryFile = gtQueryFile;//we need to use ALWAYS "queries.dat" (good quality images), so we save the old value temporarily
		gtQueryFile = "queries.dat";//use always the good images for computing map
		getQueriesResults(queries, dataDir, gtDir);
		gtQueryFile = trueGtQueryFile;//then set to the old values
		std::vector<std::string> imagesFiles;
		//it's important to use getAll since codes follow the same order
		//first line is relative to holiday_files[0] and so on
		cc::getAll(dataDir, ".jpg", imagesFiles);
		for(size_t i=0; i<queries.size(); i++){
			//get query position using getAll order
			auto findIter = find(imagesFiles.begin(), imagesFiles.end(), queries[i].query);
			if(findIter == imagesFiles.end())
				throw std::runtime_error("query "+queries[i].query+" file not found");
			size_t pos = findIter - imagesFiles.begin();
			//get query code (which follow getAll order)
			cv::Mat1f queryCode = codes.row(pos);
			//get the top-k elements
			std::vector<std::string> topkTest;
			std::vector<float> topkDistance;
			Topk(topkTest , topkDistance, imagesFiles, queryCode, codes, k);
			float correct = 0;
			float ap = 0;
			for(size_t j=0; j<topkTest.size(); j++){
				//if topkTest[j] belongs to the true positives, increment the number of correct images
				if(std::find(queries[i].truePositives.begin(), queries[i].truePositives.end(), topkTest[j]) != queries[i].truePositives.end()){
					ap += ++correct / j; //notice that topkTest[0] is always the query itself
				}
			}
			if(cutK)
				map += ap / (cutK-1);//-1 because topkTest[0] is the query itself
			else
				map += ap / queries[i].truePositives.size();
		}
		map /= queries.size();
		return map;
	}

	float CCDatasetTest::computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir){
		return computeMap(codes, dataDir, gtDir, 2);
	}

	void CCDatasetTest::getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir){
		std::ifstream imagesIfs (gtDir+"/images.dat");
		std::ifstream queriesFs (gtDir+"/"+gtQueryFile);
		std::vector<std::string> imagesFiles; //all images in the dataset
		std::string image;
		while(std::getline(imagesIfs, image))
			imagesFiles.push_back(image);
		while(std::getline(queriesFs, image)){
			queryResults q;
			std::string query_and_dir = image; // e.g. nascita_di_venere-sandro_botticelli/IMG_8852.jpg
			q.query = dataDir+"/"+query_and_dir; //absolute path to the query file
			size_t slash_pos = query_and_dir.find_first_of("/");
			if(slash_pos==std::string::npos)
				throw std::runtime_error("query "+query_and_dir+" can't find /");
			std::string dir = query_and_dir.substr(0,slash_pos); //e.g.  nascita_di_venere-sandro_botticelli/
			//the query results are all the files in the query's directory (except itself)
			for(size_t i=0; i< imagesFiles.size(); i++){
				//file from another directory or the query itself
				//e.g. annunciazione-leonardo_da_vinci/IMG_20170112_163657.jpg
				if(imagesFiles[i].compare(0, dir.size(), dir) != 0 || imagesFiles[i].compare(query_and_dir) == 0)
					continue;
				q.truePositives.push_back(dataDir+"/"+imagesFiles[i]);
			}
			queries.push_back(q);
		}
	}
}


