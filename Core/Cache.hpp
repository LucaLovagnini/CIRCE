/*
 * Cache.hpp
 *
 *  Created on: 7 Mar 2017
 *      Author: luca
 */

#ifndef CORE_CACHE_HPP_
#define CORE_CACHE_HPP_

#include <iostream>
#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <limits>
#include <omp.h>

#include "Core/CCCore.hpp"
#include "Utilities/Distance.hpp"

#define CHECKHIT

namespace cc{

	/**
	 *	C = cache code type
	 *	R = back end result type
	 *	D = cache distance type (e.g. float for euclidean)
	 *	B = back end input parameters (e.g. the image filename)
	 */
	template <typename C, typename R, typename D, typename B>
	class Cache {

	typedef std::shared_ptr<cc::Distance<C,D>> DistancePtr;

	public:
		Cache(const std::function<D(C,C)> &distance, const std::function<R(B)> &backEnd, CloudCache &cc, const size_t size = 10000);
		//The user provide already the code
		R QueryCache(const C &queryCode, B &backEndInput);
		//The user provides the path to the image to encode
		R QueryImgPath (const std::string &imgPath, const B &backEndInput);
		//The backend input is the image itself
		R QueryImgPath (const std::string &imgPath);
		//The user provides the image matrix of the image to encode
		R QueryImg (cv::Mat &img, const B &backEndInput);

		void Setup(const std::vector<std::string> &imgsPaths,const std::vector<typename std::remove_const<B>::type>  &backEndInputs);
		void Setup(const std::vector<std::string> &imgsPaths);
		void Setup(const cv::Mat1f codes, const std::vector<typename std::remove_const<B>::type>  &backEndInputs);

		int getCorrectHits();
		int getnHits();
		int getCacheMisses();
		float getAvgLookupTime();
		float getAvgReadImgTime();
		float getAvgDescriptorQueryTime();
		float getAvgEncoderQueryTime();
		void PrintCache();
		struct Compare{
			Compare(D val = std::numeric_limits<D>::max(), size_t index = 0) : val(val), index(index) {}
			D val;
			size_t index;
		};
		#pragma omp declare reduction(minimum : struct Compare : omp_out = omp_in.val < omp_out.val ? omp_in : omp_out) initializer (omp_priv=Compare())
	private:
		struct CacheElem{
			CacheElem(const C &code, const R &result, std::list<size_t>::iterator listElem) : code(code), result(result), listElem(listElem) {}
			C code;
			R result;
			std::list<size_t>::iterator listElem; //pointing to corresponding element in lru0
		};
		std::function<D(C,C)> distance;
		std::function<R(B)> backEnd;
		std::vector<CacheElem> values;
		std::list<size_t> lru;
		cc::CloudCache cc;
		size_t size; 			//cache size (# of elements)
		float threshold;
		int correctHits = 0;	//number of correct hits (where the cached value is equal to
		int nHits = 0; 			//total number of cache hits
		int cacheMisses = 0;	//total number of cache miss
		float totalLookupTime = 0;
		float totalReadImgTime = 0;
	};


	template <typename C, typename R, typename D, typename B>
	Cache<C,R,D,B>::Cache(const std::function<D(C,C)> &distance, const std::function<R(B)> &backEnd, CloudCache &cc, const size_t size)
	: distance(distance), backEnd(backEnd), cc(cc), size(size), threshold(cc.getThreshold()) {
		/*if(!cc.isTrained())
			throw std::runtime_error("CloudCache is not trained! Use Train() first");*/
		if(threshold==-1)
			throw std::runtime_error("Threshold was not set! Use setThreshold() or GenerateThreshold() first");
		values.reserve(size);
		std::cout<<"CACHE SETUP: size="<<size<<" threshold="<<threshold<<std::endl;
	}

	template <typename C, typename R, typename D, typename B>
	void Cache<C,R,D,B>::PrintCache() {
		std::cout<<"LRU: ";
		for(std::list<size_t>::iterator it=lru.begin(); it != lru.end(); ++it)
			std::cout<<*it<<" ";
		std::cout<<std::endl;
		std::cout<<"VALUES: ";
		for(size_t i=0; i<values.size(); i++)
			std::cout<<"("<<values[i].result<<","<<*(values[i].listElem)<<")"<<std::endl;;
		std::cout<<std::endl;
	}

	template <typename C, typename R, typename D, typename B>
	R Cache<C,R,D,B>::QueryCache(const C &queryCode, B &backEndInput){
		std::cout<<"cache size="<<lru.size()<<std::endl;
		//PrintCache();
		Compare min;
		R result;
		auto cacheLookUpTime = cc::startTimer();
		//Find the cached element with min distance
		#pragma omp parallel
		{
			Compare localMin;
			#pragma omp	for reduction(minimum:min) schedule(dynamic,1)
			for(size_t i=0; i<values.size(); i++){
				D d = distance(queryCode, values[i].code);
				if(d < localMin.val){
					localMin.val = d;
					localMin.index = i;
				}
			}
			#pragma omp critical
			if(localMin.val < min.val)
				min = localMin;
		}
		/*
		#pragma omp parallel for reduction(minimum:min) schedule(dynamic,1)
		for(size_t i=0; i<values.size(); i++){
			D d = distance(queryCode, values[i].code);
			if(d < min.val){
				min.val = d;
				min.index = i;
			}
		}
		*/
		totalLookupTime += cc::stopTimer(cacheLookUpTime, "cache lookup");
		std::cout<<"min.val="<<min.val<<std::endl;
		//Cache hit
		if(min.val < threshold){
			CacheElem hitElem = values[min.index];
			//take the hit element to top of the queue
			if( hitElem.listElem  != lru.begin() )
				lru.splice( lru.begin(), lru, hitElem.listElem, std::next( hitElem.listElem ) );
			result = hitElem.result;
			std::cout<<"Cache hit!"<<std::endl;
			#ifdef CHECKHIT
			R backEndResult = backEnd(backEndInput);
			if(backEndResult == result)
				correctHits++;
			else
				std::cout<<"WRONG!"<<std::endl;
			#endif
			nHits++;
		}
		//cache miss
		else {
			cacheMisses++;
			result = backEnd(backEndInput);
			//Cache reached max capacity
			if(lru.size() == size){
				//last item (the one that must be removed) value is its corresponding index in values
				size_t lastIndex = lru.back();
				std::cout<<"removiing "<<lastIndex<<std::endl;
				//remove last element
				lru.pop_back();
				//insert new element in the list
				lru.push_front(lastIndex);
				//insert new element in the value vector, replacing the old one
				values[lastIndex] = CacheElem(queryCode, result, lru.begin());
			}
			else{
				//If the cache isn't full, the index in values of the new element is the last one
				lru.push_front(values.size());  //not size-1 (we still have to insert it)
				values.push_back(CacheElem(queryCode, result, lru.begin()));
			}
		}
		std::cout<<"cache size="<<lru.size()<<std::endl;
		//PrintCache();
		std::cout<<"-------------------------------------"<<std::endl;
		return result;
	}

	template <typename C, typename R, typename D, typename B>
	void Cache<C,R,D,B>::Setup(const std::vector<std::string> &imgsPaths){
		Setup(imgsPaths, imgsPaths);
	}

	template <typename C, typename R, typename D, typename B>
	void Cache<C,R,D,B>::Setup(const std::vector<std::string> &imgsPaths,const std::vector<typename std::remove_const<B>::type>  &backEndInputs){
		if(imgsPaths.size() >= size){
			std::cout<<"WARNING: the number of initial cache values ("<<values.size()<<" is bigger than cache's capacity ("<<size<<")"<<std::endl;
			std::cout<<"Only the first "<<size<<" elements will be inserted"<<std::endl;
		}
		size_t insertElements = std::min(imgsPaths.size(), size);
		for(size_t i=0; i<insertElements; i++){
			cv::Mat img = cc::ccImRead(imgsPaths[i]);
			cv::Mat1f code;
			cc.GenerateQueryCode(img, code);
			R result = backEnd(backEndInputs[i]);
			//the index in values of the new element is the last one
			lru.push_front(values.size());  //not size-1 (we still have to insert it)
			values.push_back(CacheElem(code, result, lru.begin()));
		}
		PrintCache();
	}

	template <typename C, typename R, typename D, typename B>
	void Cache<C,R,D,B>::Setup(const cv::Mat1f codes, const std::vector<typename std::remove_const<B>::type>  &backEndInputs){
		if(codes.rows > size){
			std::cout<<"WARNING: the number of initial cache values ("<<values.size()<<" is bigger than cache's capacity ("<<size<<")"<<std::endl;
			std::cout<<"Only the first "<<size<<" elements will be inserted"<<std::endl;
		}
		size_t insertElements = std::min((size_t)codes.rows, size);
		for(size_t i=0; i<insertElements; i++){
			R result = backEnd(backEndInputs[i]);
			lru.push_front(values.size());  //not size-1 (we still have to insert it)
			values.push_back(CacheElem(codes.row(i), result, lru.begin()));
		}
	}


	template <typename C, typename R, typename D, typename B>
	R Cache<C,R,D,B>::QueryImg (cv::Mat &img, const B &backEndInput){
		cv::Mat1f code;
		cc.GenerateQueryCode(img, code);
		return QueryCache(code, backEndInput);
	}

	template <typename C, typename R, typename D, typename B>
	R Cache<C,R,D,B>::QueryImgPath (const std::string &imgPath, const B &backEndInput){
		auto imgTime = cc::startTimer();
		cv::Mat img = cc::ccImRead(imgPath);
		totalReadImgTime += cc::stopTimer(imgTime, "reading image");
		return QueryImg(img, backEndInput);
	}

	template <typename C, typename R, typename D, typename B>
	R Cache<C,R,D,B>::QueryImgPath (const std::string &imgPath){
		return QueryImgPath(imgPath, imgPath);
	}

	template <typename C, typename R, typename D, typename B>
	int Cache<C,R,D,B>::getCorrectHits(){
		return correctHits;
	}

	template <typename C, typename R, typename D, typename B>
	int Cache<C,R,D,B>::getnHits(){
		return nHits;
	}

	template <typename C, typename R, typename D, typename B>
	int Cache<C,R,D,B>::getCacheMisses(){
		return cacheMisses;
	}

	template <typename C, typename R, typename D, typename B>
	float Cache<C,R,D,B>::getAvgLookupTime(){
		return totalLookupTime / (nHits + cacheMisses);
	}
	template <typename C, typename R, typename D, typename B>
	float Cache<C,R,D,B>::getAvgReadImgTime(){
		return totalReadImgTime / (nHits + cacheMisses);
	}

	template <typename C, typename R, typename D, typename B>
	float Cache<C,R,D,B>::getAvgDescriptorQueryTime(){
		return cc.getAvgDescriptorQueryTime();
	}

	template <typename C, typename R, typename D, typename B>
	float Cache<C,R,D,B>::getAvgEncoderQueryTime(){
		return cc.getAvgEncoderQueryTime();
	}


}

#endif /* CORE_CACHE_HPP_ */
