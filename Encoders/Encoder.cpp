/*
 * Encoder.cpp
 *
 *  Created on: 16 Dec 2016
 *      Author: luca
 */

#include <iostream>

#include "Encoders/Encoder.hpp"

#include "Utilities/Utility.hpp"

namespace cc{
	Encoder::Encoder(vl_size codeSize, vl_size dimensions) : codeSize(codeSize), dimensions(dimensions){}

	/**
	 * Encode only one image into one vector matrix
	 */
	void Encoder::encode(cv::Mat1f &descriptors, cv::Mat1f &code){
		//code must be a continous vector in codeSize dimensions
		checkCode(code);
		checkRows(code,1);
		//if no descriptor detected, return a 0s vector
		if(descriptors.empty())
			code = cv::Mat1f::zeros(1,code.cols);
		else{
			//checking continuity and number of cols
			checkDescriptors(descriptors);
			encode_(descriptors, code);
		}
	}

	/**
	 * Include a set of images into a matrix (one row per image)
	 */
	void Encoder::encode(std::vector<cv::Mat1f> &descriptors, cv::Mat1f &codes){
		codes.release();
		codes.create(descriptors.size(), codeSize);

		//codes must be a continuous matrix in descriptors.size() X codeSize
		checkCode(codes);
		checkRows(codes,descriptors.size());
		for(size_t i = 0 ; i < descriptors.size() ; i++){
			cv::Mat1f code;
			//if no descriptors, then 0 line
			if(descriptors[i].empty())
				code = cv::Mat1f::zeros(1,codes.cols);
			else{
				//checking continuity and number of cols
				checkDescriptors(descriptors[i]);
				code.create(1, codes.cols);
				encode_(descriptors[i], code);
			}
			code.row(0).copyTo(codes.row(i));
		}
	}



	void Encoder::train(cv::Mat1f &trainDescriptors){
		if(!checkDescriptors(trainDescriptors))
			return;
		train_(trainDescriptors);
	}

	bool Encoder::checkCode(cv::Mat1f &code){
		if((unsigned int) code.cols != codeSize || !code.isContinuous()){
			throw std::runtime_error("checkCode: codeSize="+std::to_string(codeSize)+" code.cols="+std::to_string(code.cols)+" code.isContinuous()="+std::to_string(code.isContinuous()));
			return false;
		}
		return true;
	}

	bool Encoder::checkRows(cv::Mat1f &mat, int rows){
		if( mat.rows != rows){
			throw std::runtime_error("checkRows: mat.rows="+std::to_string(mat.rows)+" rows="+std::to_string(rows));
			return false;
		}
		return true;
	}

	bool Encoder::checkDescriptors(cv::Mat1f &descriptors){
		if(!descriptors.isContinuous() || (unsigned int) descriptors.cols != dimensions){
			throw std::runtime_error("checkDescriptors: descriptors.isContinuous()="+std::to_string(descriptors.isContinuous())+" descriptors.cols="+std::to_string(descriptors.cols)+" descDimensions="+std::to_string(dimensions));
			return false;
		}
		return true;
	}

	Encoder::~Encoder(){}
}
