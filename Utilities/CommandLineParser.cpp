
/*
 * CommandLineParser.cpp
 *
 *  Created on: 28 Jan 2017
 *      Author: luca
 */

#include <iostream>
#include <typeinfo>

#include "Utilities/CommandLineParser.hpp"
#include "Encoders/VLADEncoder.hpp"
#include "Encoders/FisherVectorEncoder.hpp"
#include "Descriptors/DSIFTVLFeat.hpp"
#include "Descriptors/SIFTOpenCV.hpp"
#include "Descriptors/SIFTVLFeat.hpp"
#include "Descriptors/SURFOpenCV.hpp"
#ifndef CC_DISABLE_CUDA
#include "Descriptors/SURF_CUDAOpenCV.hpp"
#endif
#include "Descriptors/DSURFOpenCV.hpp"
#include "Descriptors/hesaff/hesaff.cpp"

namespace cc{
	//look for any string in toFind in input. If only one occurences is found, returns its position in input
	//otherwise (if more than one occurence is found, or not at all) throw runtime_error
	int findOccurences (const std::vector<std::string> &input, const std::vector<std::string> &toFind){
		int pos = -1;
	    std::string forError;
	    for(size_t i = 0 ; i<toFind.size(); i++)
	    	forError += toFind[i]+" ";
		for(size_t i = 0 ; i < input.size() ; i++)
			for(size_t j = 0 ; j < toFind.size() ; j++)
				if(!input[i].compare(toFind[j])){
					if(pos == -1) 	//first time we find something of toFind in input
						pos = i;
					else{			//we've already found something before
						std::string errorMessage("Found multiple occurences of "+forError);
						throw std::runtime_error(errorMessage);
					}
				}
		return pos;
	}

	template<typename T, typename ... Ts>
	std::shared_ptr<T> make (int argc, int pos, Ts ... args){
		const int n = sizeof ...(Ts);
		if(argc <= n+pos)
			throw std::runtime_error("Too few arguments");
		return std::shared_ptr<T>(new T(args ...));
	}


	// The public factory
	std::shared_ptr<cc::Encoder> makeEncoder(int argc, const char*argv[])
	{
	    std::vector<std::string> toFind = {"VLAD", "FisherVector"};
		std::vector<std::string> allArgs(argv, argv + argc);
		int pos = findOccurences(allArgs, toFind); //position in argv where we find the first occurence of any element in toFind
		if (pos == -1){
		    std::cout<<"Creating default VLAD"<<std::endl;
		    return std::shared_ptr<cc::VLADEncoder>(new cc::VLADEncoder);
		}
		const std::string name = argv[pos];
	    if (name == "VLAD") {
	    	vl_size numCenters = convert<vl_size>(argv[pos+1]);
	    	vl_size dimensions = convert<vl_size>(argv[pos+2]);
	    	std::string flags = argv[pos+3];
	        return make<cc::VLADEncoder, vl_size, vl_size, std::string> (argc, pos, numCenters, dimensions, flags);
	    } else if (name == "FisherVector") {
	    	vl_size numCenters = convert<vl_size>(argv[pos+1]);
	    	vl_size dimensions = convert<vl_size>(argv[pos+2]);
	    	std::string flags = argv[pos+3];
	        return make<cc::FisherVectorEncoder, vl_size, vl_size, std::string> (argc, pos, numCenters, dimensions, flags);
	    }
	    // ...
	    std::cout<<"Creating default VLAD"<<std::endl;
	    throw std::runtime_error("Not supposed to arrive here in makeEncoder");
	}

	std::shared_ptr<cc::Descriptor> makeDescriptor (int argc, const char*argv[])
	{
	    std::vector<std::string> toFind = {"SIFTOpenCV", "SIFTVLFeat", "DSIFTVLFeat", "SURFOpenCV", "SURF_CUDAOpenCV", "DSURFOpenCV", "PHA"};
		std::vector<std::string> allArgs(argv, argv + argc);
		int pos = findOccurences(allArgs, toFind);
		if(pos == -1){
		    std::cout<<"Creating default SIFTOpenCV"<<std::endl;
		    return std::shared_ptr<cc::SIFTOpenCV>(new cc::SIFTOpenCV);
		}
	    const std::string name = argv[pos];
	    if (name == "SIFTOpenCV") {
	    	int nFeatures = convert<int>(argv[pos+1]);
	    	int nOctaveLayers= convert<int> (argv[pos+2]);
	    	double contrastThreshold = convert<double> (argv[pos+3]);
	    	double edgeThreshold = convert<double> (argv[pos+4]);
	    	double sigma= convert<double> (argv[pos+5]);
	        return make<cc::SIFTOpenCV, int,int,double,double,double> (argc, pos, nFeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
	    } else if (name == "SIFTVLFeat") {
	        return make<cc::SIFTVLFeat>(argc,pos);
	    }
	    else if(name == "DSIFTVLFeat"){
	    	int step = convert<int> (argv[pos+1]);
	    	int binSize = convert<int> (argv[pos+2]);
	    	int maxBinSize = convert<int> (argv[pos+3]);
	    	return make<cc::DSIFTVLFeat,int,int,int>(argc,pos,step,binSize,maxBinSize);
	    }
	    else if(name == "SURFOpenCV"){
	    	double hessianThreshold = convert<double> (argv[pos+1]);
	    	int nOctaves = convert<int> (argv[pos+2]);
	    	int nOctaveLayers = convert<int> (argv[pos+3]);
	    	bool extended = convert<bool> (argv[pos+4]);
	    	bool upright = convert<bool> (argv[pos+5]);
	    	return make<cc::SURFOpenCV,double,int,int,bool,bool> (argc, pos, hessianThreshold, nOctaves, nOctaveLayers, extended, upright);
	    }
#ifndef CC_DISABLE_CUDA
	    else if(name == "SURF_CUDAOpenCV"){
	    	double hessianThreshold = convert<double> (argv[pos+1]);
	    	int nOctaves = convert<int> (argv[pos+2]);
	    	int nOctaveLayers = convert<int> (argv[pos+3]);
	    	bool extended = convert<bool> (argv[pos+4]);
	    	float keypointsRatio = convert<float> (argv[pos+5]);
	    	bool upright = convert<bool> (argv[pos+6]);
	    	return make<cc::SURF_CUDAOpenCV,double,int,int,bool,float,bool> (argc, pos, hessianThreshold, nOctaves, nOctaveLayers, extended, keypointsRatio, upright);
	    }
#endif
	    else if(name == "DSURFOpenCV"){
	    	int step = convert<int>(argv[pos+1]);
	    	double hessianThreshold = convert<double> (argv[pos+2]);
	    	int nOctaves = convert<int> (argv[pos+3]);
	    	int nOctaveLayers = convert<int> (argv[pos+4]);
	    	bool extended = convert<bool> (argv[pos+5]);
	    	bool upright = convert<bool> (argv[pos+6]);
	    	return make<cc::DSURFOpenCV, int,double,int,int,bool,bool> (argc, pos, step, hessianThreshold, nOctaves, nOctaveLayers, extended, upright);
	    }
	    else if(name == "PHA"){
	        HessianAffineParams par;
	         // copy params
	         PyramidParams p;
	         p.threshold = par.threshold;

	         AffineShapeParams ap;
	         ap.maxIterations = par.max_iter;
	         ap.patchSize = par.patch_size;
	         ap.mrSize = par.desc_factor;

	         SIFTDescriptorParams sp;
	         sp.patchSize = par.patch_size;

	         return std::shared_ptr<AffineHessianDetectorCC> (new AffineHessianDetectorCC(p, ap, sp));
	    }

	    throw std::runtime_error("Not supposed to arrive here in makeDescriptor");
	}

	dataset getDataset(int argc, const char*argv[]){
		std::vector<std::string> allArgs(argv, argv + argc);
		std::vector<std::string>::iterator it = std::find(allArgs.begin(), allArgs.end(), "dataset");
		if(it == allArgs.end())
			throw std::runtime_error("Cannot find 'dataset' keyword in arguments");
		else{
			it++;
			if((*it).compare("oxford") == 0)
				return cc::oxford;
			else if((*it).compare("holiday") == 0)
				return cc::holiday;
			else if ((*it).compare("painting") == 0)
				return cc::painting;
			else if ((*it).compare("posters") == 0)
				return cc::posters;
			else if ((*it).compare("oxfordForCache") == 0)
				return cc::oxfordForCache;
			else
				throw std::runtime_error(*it+" is an invalid dataset");
		}
	}

}
