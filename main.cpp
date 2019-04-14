/*
 * main.cpp
 *
 *  Created on: 5 Dec 2016
 *      Author: luca
 */



#include <iostream>
#include <sstream>
#include <memory>
#include <typeinfo>
#include <cxxabi.h>

#include <cstdlib>


#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdlib>
#include <functional>
#include <omp.h>
#include <unistd.h>

#include "Core/CCCore.hpp"
#include "Core/Cache.hpp"
#include "Utilities/CommandLineParser.hpp"
#include "DatasetTests/DatasetTest.hpp"
#include "DatasetTests/OxfordTestManager.hpp"
#include "DatasetTests/HolidayTest.hpp"
#include "Descriptors/SIFTOpenCV.hpp"
#include "Descriptors/DSIFTVLFeat.hpp"
#include "Descriptors/SURFOpenCV.hpp"
#include "Descriptors/SIFTVLFeat.hpp"
#include "Encoders/VLADEncoder.hpp"
#include "Encoders/FisherVectorEncoder.hpp"
#include "Utilities/Utility.hpp"

#include "python2.7/Python.h"
#include "DatasetTests/CCDatasetTest.hpp"

#define VERBOSE 1 //toDo
#define EXP 0.3
#define NQUERIES 1500

std::string getImageTopic(const std::string &imgPath){
	  size_t lastSlash = imgPath.find_last_of("/");
	  size_t beforeLastSlash = imgPath.substr(0,lastSlash-1).find_last_of("/");
	  return imgPath.substr(beforeLastSlash+1, lastSlash-beforeLastSlash-1);
}

float euclDist (const cv::Mat1f &first, const cv::Mat1f &second){
	float dist = 0;
	const float *firstp = first.ptr<float>(0);
	const float *secondp = second.ptr<float>(0);
	#pragma omp simd reduction(+:dist)
	for(size_t i=0; i<first.cols; i++){
		float diff = firstp[i]-secondp[i];
		dist += diff*diff;
	}
	return std::sqrt(dist);
}


int main (int argc, const char *argv[]){
	try{

		int nThreads = omp_get_max_threads();
		int codeSize = 128;
		size_t cacheSize = 10000;
		std::vector<std::string> args(argv, argv+argc);
		for (size_t i = 1; i < args.size(); ++i) {
			  if (args[i] == "nThreads")
					nThreads = atoi(argv[i+1]);
			  else if (args[i] == "codeSize")
					codeSize = atoi(argv[i+1]);
			  else if (args[i] == "cacheSize")
				    cacheSize = atoi(argv[i+1]);
		}

		std::cout<<"nThreads = "<<nThreads<<" codeDim="<<codeSize<<" cacheSize="<<cacheSize<<std::endl;
		omp_set_num_threads(nThreads);
		cc::CloudCache cc (argc, argv, "");
		cc.setThreshold(1.2);
		//distance function used by the cache
		std::function<float(const cv::Mat1f&, const cv::Mat1f&)> euclideanDistance =
				[](const cv::Mat1f &first, const cv::Mat1f &second){return cv::norm(first, second);};
		//cache declaration
		std::cout<<"Creating cache"<<std::endl;
		cc::Cache<cv::Mat1f, std::string, float, const std::string> cache(euclideanDistance, getImageTopic, cc, cacheSize);
		std::cout<<"Cache created"<<std::endl;
		//create a random matrix
		cv::Mat1f mat(cacheSize, codeSize); // Or: Mat mat(2, 4, CV_64FC1);
		float low = 0;
		float high = 2.0;
		 srand (0);
		 cv::theRNG().state = cv::getTickCount() ;
		randu(mat, cv::Scalar(low), cv::Scalar(high));
		std::vector<std::string> backEndInputs(cacheSize, "Hello there!");
		cache.Setup(mat, backEndInputs);
		for(int i=0; i<100;i++){
			cv::Mat1f query(1, codeSize);
			randu(query, cv::Scalar(low), cv::Scalar(high));
			cache.QueryCache(query, "Hello there!");
		}
		//write to ods file
		//get home directory
		struct passwd *pw = getpwuid(getuid());
		const std::string 	homeDir(pw->pw_dir);
		const std::string 	output_path =  homeDir + "/Documents/CloudCacheOutput";

		std::vector<std::string> extraLabels = {"nThreadsnThreads", "codeSize", "cacheSize","avgLookUpTime"};
		cc.WriteOds(output_path+"/stats.ods", extraLabels, nThreads, codeSize, cacheSize, cache.getAvgLookupTime());
	}
	catch (const std::exception &e)
	{
		std::cerr<< "error: "<<e.what() <<std::endl;
		return EXIT_FAILURE;
	}
	std::cout<<"Bye bye!"<<std::endl;
	return 0;
}
