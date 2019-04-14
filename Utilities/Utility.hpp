/*
 * ImgUtility.hpp
 *
 *  Created on: 15 Dec 2016
 *      Author: luca
 */

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include "opencv2/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <sys/time.h>
#include <fstream>

#include "Utilities/FileSystem.hpp"

namespace cc{
	void resize(cv::Mat &img, float targetSize);
	cv::Mat ccImRead(const std::string &imgPath);
	float timeElapsed(const struct timeval &start);
	struct timeval startTimer();
	float stopTimer(const struct timeval &start, const std::string &label = std::string());
	template <typename T>
	void relja_PCA(T &U, T &lams, T &mu, T &Utmu, const T &x, size_t nPCs = 0, const bool substractMean = true);
}

template <typename T>
void cc::relja_PCA(T &U, T &lams, T &mu, T &Utmu, const T &x, size_t nPCs, const bool substractMean){

	int nPoints = x.cols;
	int nDims = x.rows;

	if(nPCs <= 0)
		nPCs = nDims;

	if(substractMean){
		cv::reduce(x,mu, 1, CV_REDUCE_AVG);
		for(int i=0; i<nPoints; i++){
			x.col(i) = x.col(i) - mu;
		}
	}
	else
		mu = cv::Mat::zeros(1, nDims, x.type());

	bool doDual;
	T X2;
	if(nDims<=nPoints){
		std::cout<<"Not dual"<<std::endl;
		doDual = false;
		cv::mulTransposed(x,X2, false);
	}
	else{
		std::cout<<"Dual"<<std::endl;
		doDual = true;
		cv::mulTransposed(x, X2, true);
	}
	X2 /= (nPoints-1);

	std::cout<<"Computing eigenvalues/vector..."<<std::endl;
	cv::eigen(X2, lams, U);

	U = U.t();


	//save only the first nPCs elements of L, which is already a vector
	if(nPCs>0 && (int) nPCs<X2.rows){
		lams(cv::Range(0, nPCs), cv::Range(0, lams.cols)).copyTo(lams);
		U(cv::Range(0,U.rows),cv::Range(0,nPCs)).copyTo(U);
	}

	if (doDual){

		cv::Mat1f diag = lams.clone();
		for(int i=0; i<diag.rows; i++)
			diag.at<float>(i,1) = diag.at<float>(i,1) > 1e-9 ? diag.at<float>(i,1) : 1e-9;
		cv::sqrt(diag, diag);
		diag = 1.0f/diag;
		diag = cv::Mat::diag(diag);
		U = x * (U * diag / std::sqrt(nPoints-1)) ;
	}

	Utmu = U.t() * mu;

}

#endif /* UTILITY_HPP_ */
