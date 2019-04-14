/*
 * OxfordTest.cpp
 *
 *  Created on: 8 Dec 2016
 *      Author: luca
 */

#include "OxfordTest.hpp"

#include <iostream>
#include <fstream>

#include <boost/range/iterator_range.hpp>

#define VERBOSE 0

std::vector<std::string>
load_list(const std::string& fname)
{
  std::vector<std::string> ret;
  std::ifstream fobj(fname.c_str(), std::ifstream::in);
  /*if (!good)
	  std::cerr << "File " << fname << " not found!\n"; exit(-1);*/
  std::string line;
  while (getline(fobj, line))
	  ret.push_back(line);
  return ret;
}

template<class T>
std::set<T> vector_to_set(const std::vector<T>& vec) {
	return std::set<T>(vec.begin(), vec.end());
}


bool OxfordTest::setDirectory(fs::path root){
	if(fs::is_directory(root)){
		itr = fs::directory_iterator(root);
		return true;
	}
	else
		return false;
}

bool OxfordTest::nextQuery(std::string &nextQuery){
	//std::cout<<"Reading next query"<<std::endl;
	while(true) {
		nextQuery.clear();
		if(itr == fs::directory_iterator())	{
			//std::cout<<"No more files!"<<std::endl;
			return false;
		}
		else {
			std::string nextFile = (*itr++).path().string();
			if(nextFile.find("query.txt") != std::string::npos){
				std::string base = nextFile.substr(0,nextFile.length()-9); // something_query.txt -> something
				good_set = vector_to_set( load_list(base + "good.txt") );
				ok_set = vector_to_set( load_list(base + "ok.txt") );
				junk_set = vector_to_set( load_list(base + "junk.txt") );
				pos_set.insert(good_set.begin(), good_set.end());
				pos_set.insert(ok_set.begin(), ok_set.end());
				std::ifstream ifs(nextFile);
				std::getline(ifs,nextQuery);
				nextQuery = nextQuery.substr(5,nextQuery.find(' ')-5);
				if(VERBOSE)
					std::cout<<"query: "<<nextQuery<<std::endl;
				return true;
			}
		}
	}
}

void OxfordTest::computeAp(std::vector<std::string> &ranked_list){
	  float old_recall = 0.0;
	  float old_precision = 1.0;
	  float ap = 0.0;

	  size_t intersect_size = 0;
	  size_t i = 0;
	  size_t j = 0;
	  for ( ; i<ranked_list.size(); ++i) {
		if(VERBOSE){
			  std::cout<<"ranked_list: "<<ranked_list[i]<<std::endl;
			  std::cout<<"pos_set: ";
			  for(std::set<std::string>::iterator it = pos_set.begin() ; it != pos_set.end() ; ++it)
				  std::cout<<*it<<" ";
			  std::cout<<std::endl;
			  std::cout<<"intersect: "<<pos_set.count(ranked_list[i])<<std::endl;
			  if(!pos_set.count(ranked_list[i]))
				  std::cin.get();
		}
	    if (junk_set.count(ranked_list[i])) continue;
	    if (pos_set.count(ranked_list[i])){
	    	if(i==1){
	    		good++;
	    	}
	    	intersect_size++;
	    }

	    float recall = intersect_size / (float)pos_set.size();
	    float precision = intersect_size / (j + 1.0);

	    ap += (recall - old_recall)*((old_precision + precision)/2.0);

	    old_recall = recall;
	    old_precision = precision;
	    j++;
	  }
	  totalAP += ap;
	  nQueries++;
	  junk_set.clear();
	  pos_set.clear();
}

float OxfordTest::getMAP() {
	return totalAP/nQueries;
}

float OxfordTest::getGoodRatio(){
	return (float) good/nQueries;
}

std::set<std::string> OxfordTest::getPosSet(){
	std::set<std::string> tp; //true positives
	tp.insert(good_set.begin(), good_set.end());
	tp.insert(ok_set.begin(), ok_set.end());
	return tp;
}
