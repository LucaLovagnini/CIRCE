/*
 * FisherVectorEncoder.cpp
 *
 *  Created on: 30 Dec 2016
 *      Author: luca
 */

#include "FisherVectorEncoder.hpp"
#include <boost/algorithm/string.hpp>

namespace cc{
	FisherVectorEncoder::FisherVectorEncoder(vl_size numCenters, vl_size dimensions, std::string flags) :
	Encoder(2 * numCenters * dimensions, dimensions), numCenters(numCenters){
		gmm = vl_gmm_new(VL_TYPE_FLOAT, dimensions, numCenters) ;
		kmeans = vl_kmeans_new (VL_TYPE_FLOAT, VlDistanceL2) ;
		std::vector<std::string> tokens;
		boost::split(tokens, flags, boost::is_any_of("|"));
		this->flags = 0;
		for(size_t i=0; i<tokens.size(); i++){
			if(!tokens[i].compare("VL_FISHER_FLAG_SQUARE_ROOT"))
				this->flags = this->flags | VL_FISHER_FLAG_SQUARE_ROOT;
			else if(!tokens[i].compare("VL_FISHER_FLAG_NORMALIZED"))
				this->flags = this->flags | VL_FISHER_FLAG_NORMALIZED;
			else if(!tokens[i].compare("VL_FISHER_FLAG_IMPROVED"))
				this->flags = this->flags | VL_FISHER_FLAG_IMPROVED;
			else if(!tokens[i].compare("VL_FISHER_FLAG_FAST"))
				this->flags = this->flags | VL_FISHER_FLAG_FAST;
			else
				throw std::runtime_error("Invalid FisherVector flag "+tokens[i]);
		}
	}

	void FisherVectorEncoder::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		std::string sflag;
		if(flags & VL_FISHER_FLAG_NORMALIZED)
			sflag+="NORMALIZED";
		if(flags & VL_FISHER_FLAG_SQUARE_ROOT){
			if(!sflag.empty()) sflag += "|";
			sflag+="SQUARE_ROOT";
		}
		if(flags & VL_FISHER_FLAG_IMPROVED){
			if(!sflag.empty()) sflag += "|";
			sflag+="IMPROVED";
		}
		if(flags & VL_FISHER_FLAG_FAST){
			if(!sflag.empty()) sflag += "|";
			sflag+= "FAST";
		}
		params.push_back(std::pair<std::string,std::string>("Encoder", "FisherVectorEncoder"));
		params.push_back(std::pair<std::string,std::string>("numCenters",std::to_string(numCenters)));
		params.push_back(std::pair<std::string,std::string>("dimensions",std::to_string(dimensions)));
		params.push_back(std::pair<std::string,std::string>("flags",sflag));
	}


	std::string FisherVectorEncoder::getParameters(){

		std::string sflag;
		if(flags & VL_FISHER_FLAG_NORMALIZED)
			sflag+="NORMALIZED";
		if(flags & VL_FISHER_FLAG_SQUARE_ROOT){
			if(!sflag.empty()) sflag += "|";
			sflag+="SQUARE_ROOT";
		}
		if(flags & VL_FISHER_FLAG_IMPROVED){
			if(!sflag.empty()) sflag += "|";
			sflag+="IMPROVED";
		}
		if(flags & VL_FISHER_FLAG_FAST){
			if(!sflag.empty()) sflag += "|";
			sflag+= "FAST";
		}

		return "\t FisherVector \t dimensions="+std::to_string(dimensions)+"\t numCenters="+std::to_string(numCenters)+"\t codeSize="+std::to_string(codeSize)+"\t flags="+sflag;
	}

	void FisherVectorEncoder::train_(cv::Mat1f &trainData){
		vl_kmeans_set_algorithm (kmeans, VlKMeansElkan) ;
		vl_kmeans_set_initialization(kmeans, VlKMeansPlusPlus);
		vl_kmeans_set_max_num_iterations (kmeans, 100) ;
		vl_gmm_set_initialization (gmm,VlGMMKMeans);
		vl_gmm_set_kmeans_init_object(gmm, kmeans);
		vl_gmm_cluster (gmm, trainData.data, trainData.rows);
		means = vl_gmm_get_means(gmm);
		covariances = vl_gmm_get_covariances(gmm);
		priors = vl_gmm_get_priors(gmm);
	}

	void FisherVectorEncoder::encode_ (cv::Mat1f &descriptors, cv::Mat1f &code){
		vl_fisher_encode(code.data, VL_TYPE_FLOAT,
						means, dimensions, numCenters,
						covariances,
						priors,
						descriptors.data,descriptors.rows,
						flags
						);
	}



	void FisherVectorEncoder::writeTrainer(const std::string &trainerFile){

	}
	void FisherVectorEncoder::readTrainer(const std::string &trainerFile){

	}
	void FisherVectorEncoder::printTrainer(){

	}
}
