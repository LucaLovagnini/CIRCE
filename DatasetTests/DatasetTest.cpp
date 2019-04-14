/*
 * DatasetTest.cpp
 *
 *  Created on: 4 Mar 2017
 *      Author: luca
 */

#include <iostream>
#include <queue>

#include "opencv2/highgui/highgui.hpp"
#include "DatasetTest.hpp"
#include "Utilities/Utility.hpp"
#include "Utilities/FileSystem.hpp"

namespace cc{
	void DatasetTest::Topk(std::vector<std::string> &topkTest , std::vector<float> &topkDistance,
					const std::vector<std::string> files, const cv::Mat1f &queryCode, const cv::Mat1f &codes,
					size_t k, const std::string ext){
		k = k <= (size_t) codes.rows ? k : (size_t) codes.rows;
		topkTest.clear();
		topkDistance.clear();
		topkTest.resize(k,std::string());
		topkDistance.resize(k,0);

		//---------------------------EUCLIDEAN DISTANCES---------------------------
		std::vector<double> distances(codes.rows,0);
		for(int i=0;i<codes.rows;i++){
			if(countNonZero(codes.row(i)))
				distances[i] = norm(queryCode,codes.row(i),cv::NORM_L2);
			else{
				//if all values are 0, then the descriptor was empty (look Encoder::encode)
				distances[i] = std::numeric_limits<float>::max();
			}
			//std::cout<<"distances["<<i<<"]= "<<distances[i]<<std::endl;
		}
		//---------------------------TOP-K---------------------------
		std::priority_queue<std::pair<double, size_t>> queue;
		for (size_t i = 0; i < distances.size(); i++) {
			if(queue.size()<k)
				queue.push(std::pair<double, size_t>(distances[i], i));
			else if(queue.top().first>distances[i]){//the top element is the one with the biggest distance
					queue.pop();
					queue.push(std::pair<double, size_t>(distances[i], i));
			}
		}
		for(size_t i=0;i<k;i++){ //don't use queue.size() !
			topkTest[k-1-i] = files[queue.top().second];
			topkDistance[k-1-i] = queue.top().first;
			queue.pop();
		}
	}

	void DatasetTest::queryImage(const cv::Mat1f &codes, const std::string &dataDir, const std::string &queryImg, const std::string &ext){
		std::vector<std::string> files;
		cc::getAll(dataDir, ext, files);
		cv::Mat1f queryCode;
		for(size_t i=0 ; i<files.size() ; i++){
			if(files[i] == dataDir+"/"+queryImg){
				queryCode = codes.row(i);
				break;
			}
			if(i==files.size()-1){
				std::cerr<<"Didn't find "<<dataDir+"/"+queryImg<<std::endl;
				return;
			}
		}
		size_t k = 10; // number of indices we need
		std::vector<std::string> topkTest(k);
		std::vector<float> topkDistance(k);
		Topk(topkTest, topkDistance, files, queryCode, codes, k, ext);
		for (size_t i=0; i<topkTest.size(); i++){
			std::cout<<"Opening "<<topkTest[i]<<std::endl;
			cv::Mat image = ccImRead(topkTest[i]);
			cv::namedWindow( topkTest[i], cv::WINDOW_AUTOSIZE );// Create a window for display.
			cv::imshow( topkTest[i], image );                   // Show our image inside it.
		}
		std::cout<<"waiting for entering key..."<<std::endl;
		cv::waitKey(0);
		std::cout<<"Gotcha!"<<std::endl;
		cv::destroyAllWindows();
		std::cout<<"DESTROYED!"<<std::endl;
	}


	DatasetTest::~DatasetTest(){}

}
