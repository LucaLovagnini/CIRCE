/*
 * Descriptor.cpp
 *
 *  Created on: 14 Dec 2016
 *      Author: luca
 */

#include <fstream>
#include <omp.h>
#include <random>
#include <numeric>

#include "opencv2/imgcodecs.hpp"

#include "Descriptors/Descriptor.hpp"
#include "Utilities/FileSystem.hpp"
#include "Utilities/Utility.hpp"

namespace fs = boost::filesystem;

namespace cc{

	void Descriptor::ComputeDescriptors_(cv::Mat &img, cv::Mat1f &descriptors){
		if(resizeDim>0)
			cc::resize(img, resizeDim);
		auto startDesc = cc::startTimer();
		ComputeDescriptors(img,descriptors);
		double descriptorTime = cc::timeElapsed(startDesc);
		#pragma omp atomic
		onlyDescriptor += descriptorTime;
		#pragma omp atomic
		processedImages++;
	}

	void Descriptor::ComputeDescriptorsRange(const std::vector<std::string> &files, std::vector<cv::Mat1f> &descriptors, const int start, const int finish, size_t errors){
		descriptors.clear();
		int nextPerc = 10, percStep = 10;
		float range = finish-start;
		for(int i=start, c=0 ; i<finish ; i++, c++){
			cv::Mat img;
			cv::Mat1f imgDescriptors;
			auto startIm = cc::startTimer();
			//std::cout<<i<<": "<<files[i]<<std::endl;
			img = ccImRead(files[i]);
			#pragma omp atomic
			onlyImage += cc::stopTimer(startIm);
			if(!img.data)
				errors++;
			ComputeDescriptors_(img,imgDescriptors);
			if(imgDescriptors.rows==0)
				std::cout<<i<<" "<<files[i]<<" has 0 descriptors"<<std::endl;
			if(samples > 0 && samples < imgDescriptors.rows){
				cv::Mat1f sampledDescripotrs;
				std::vector<int> v(imgDescriptors.rows);
				std::iota (std::begin(v), std::end(v), 0); //fill v with 0 ... imgDescriptors.rows
				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(v.begin(), v.end(), g);
				for(int j=0 ; j<samples; j++){
					sampledDescripotrs.push_back(imgDescriptors.row(v[j]));
				}
				descriptors.push_back(sampledDescripotrs);
			}
			else
				descriptors.push_back(imgDescriptors); //copy of a smart pointer, not expensive
			float perc = 100*c/range;
			if( perc >= nextPerc){
				if(omp){
					#pragma omp critical
					std::cout<<"Thread "<<omp_get_thread_num()<<" "<<perc<<"%"<<std::endl;
				}
				else
					std::cout<<"Thread "<<omp_get_thread_num()<<" "<<perc<<"%"<<std::endl;
				nextPerc += percStep;
			}
		}
		std::cout<<"onlyImage="<<onlyImage<<std::endl;
		std::cout<<"onlyDescriptor="<<onlyDescriptor<<std::endl;
		std::cout<<"Thread "<<omp_get_thread_num()<<": "<<start<<" "<<finish<<" finished!"<<std::endl;
	}

	void Descriptor::ComputeDescriptors(const std::string &fileName, const std::string &imgExt, cv::Mat1f &descriptor){
		cv::Mat img = ccImRead(fileName);
		if(!img.data)
			throw std::runtime_error("Error reading the image "+fileName);
		ComputeDescriptors_(img,descriptor);
	}

	void Descriptor::ComputeDescriptors(const std::string &dirPath, const std::string &imgExt, std::vector<cv::Mat1f> &descriptors){

		if(!fs::exists(dirPath) || !fs::is_directory(dirPath))
			throw std::runtime_error(dirPath+" doesn't exists or it is not a directory");

		std::vector<std::string> files;
		cc::getAll(dirPath,imgExt,files);
		size_t errors = 0;
		if(omp){
			#pragma omp parallel
			{
				//need distinct (not shared) matrices
				std::vector<cv::Mat1f> myDescriptors;
				cv::Mat1f imgDescriptors;
				cv::Mat img;
				const int nthreads = omp_get_num_threads();
				const int ithread = omp_get_thread_num();
				const int start = ithread*files.size()/nthreads;
				const int finish = (ithread+1)*files.size()/nthreads;
				ComputeDescriptorsRange(files, myDescriptors, start, finish, errors);
				#pragma omp for ordered schedule(static)
				for(int i=0 ; i<omp_get_num_threads() ; i++){
					#pragma omp ordered
					{
						for(size_t i=0 ; i<myDescriptors.size() ; i++){
							descriptors.push_back(myDescriptors[i]);
						}
					}

				}
			}
		}
		else{
			std::cout<<"Computing descriptors..."<<std::endl;
			ComputeDescriptorsRange(files, descriptors, 0, files.size(), errors);
		}
		if(errors)
			throw std::runtime_error("Something wrong in openmp!");
	}

	void Descriptor::setResizeDim(const size_t resizeDim){
		this->resizeDim = resizeDim;
	}

	void Descriptor::setSamples(const size_t samples){
		this->samples = samples;
	}

	void Descriptor::setOMP(const bool omp){
		this->omp = omp;
	}

	double Descriptor::getOnlyDescriptor(){
		return onlyDescriptor;
	}

	Descriptor::~Descriptor(){}
}
