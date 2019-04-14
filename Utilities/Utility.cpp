/*
 * ImgUtility.cpp
 *
 *  Created on: 15 Dec 2016
 *      Author: luca
 */

#include "Utilities/Utility.hpp"
#include "opencv2/imgcodecs.hpp"

namespace cc{
	void resize(cv::Mat &img, float targetSize){
		float max = std::max(img.rows,img.cols);
		if(max>targetSize){
			float ratio = targetSize/max;
			cv::resize(img,img, cv::Size((int)(img.cols*ratio),(int)(img.rows*ratio)),0,0,cv::INTER_LINEAR);
		}
	}

	cv::Mat ccImRead(const std::string &imgPath){
		return cv::imread(imgPath);
	}

	float timeElapsed(const struct timeval &start){
		struct timeval end;
		float mtime, seconds, useconds;

		gettimeofday(&end, NULL);

		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;

		mtime = seconds + useconds/1000000.0;

		return mtime;

	}

	struct timeval startTimer(){
		struct timeval start;
		gettimeofday(&start, NULL);
		return start;
	}
	float stopTimer(const struct timeval &start, const std::string &label){
		float time = timeElapsed(start);
		if(!label.empty())
			std::cout<<label<<" time "<<time<<" seconds"<<std::endl;
		return time;
	}
}
