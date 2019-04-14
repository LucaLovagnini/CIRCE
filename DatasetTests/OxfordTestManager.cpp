/*
 * OxfordTestManager.cpp
 *
 *  Created on: 20 Dec 2016
 *      Author: luca
 */

//#include <iostream>

#include <queue>

#include "OxfordTest.hpp"
#include "OxfordTestManager.hpp"
#include "Utilities/FileSystem.hpp"

namespace cc{
	float OxfordTestManager::computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK){
		//The MAP is computed on the whole code list (or most part of it, say 10k codes)
		//http://stackoverflow.com/questions/40906671/confusion-about-mean-average-precision/41369415#41369415
		std::string ext = ".jpg";
		size_t k = codes.rows < 10000 ? (size_t) codes.rows : 10000;
		std::vector<std::string> files;
		cc::getAll(dataDir, ext, files);
		OxfordTest ot;
		ot.setDirectory(gtDir);
		std::string query;
		int c=0;
		while(ot.nextQuery(query)){
			//std::cout<<query<<std::endl;
			cv::Mat1f queryCode;
			for(int i=0;i<codes.rows;i++){
				if(!files[i].compare(dataDir+"/"+query+ext)){
					queryCode = codes.row(i);
					break;
				}
				if( i == codes.rows-1)
					throw std::runtime_error("Didn't find "+dataDir+"/"+query+ext);
			}
			std::vector<std::string> topkTest(k);
			std::vector<float> topkDistance(k);
			Topk(topkTest, topkDistance, files, queryCode, codes, k);
			//std::cout<<topkDistance[1]<<std::endl;
			//Topk returns the absolute path to the files (extension included)
			//We need only the file name (without extension)
			for(size_t i=0; i<topkTest.size(); i++){
				topkTest[i] = topkTest[i].substr(topkTest[i].find_last_of("/\\")+1);//get file name
				topkTest[i] = topkTest[i].substr(0,topkTest[i].length() - ext.length());//removing extension
			}
			//std::cout<<"Top-k:"<<std::endl;
			//for(size_t i=0 ; i<k ; i++)
			//	std::cout<<i<<": "<<topkTest[i]<<"\t"<<topkDistance[i]<<std::endl;
			//return;
			ot.computeAp(topkTest);
			c++;
		}
		std::cout<<"Oxford test finished!"<<std::endl;
		std::cout<<"goodRatio="<<ot.getGoodRatio()<<std::endl;
		return ot.getMAP();
	}

	float OxfordTestManager::computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir){
		throw std::runtime_error("computMrr not supported for oxford dataset!");
	}

	void OxfordTestManager::getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir){
		OxfordTest ot;
		ot.setDirectory(gtDir);
		std::string query;
		while(ot.nextQuery(query)){
			queryResults q;
			q.query = dataDir+"/"+query+".jpg";
			std::set<std::string> pos_set = ot.getPosSet();
			for(std::set<std::string>::iterator it = pos_set.begin(); it != pos_set.end(); ++it)
				q.truePositives.push_back(dataDir+"/"+*it+".jpg");
			queries.push_back(q);
		}
	}

}
