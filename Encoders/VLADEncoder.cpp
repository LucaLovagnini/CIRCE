/*
 * VLADEncoder.cpp
 *
 *  Created on: 16 Dec 2016
 *      Author: luca
 */

#include <boost/algorithm/string.hpp>

#include "VLADEncoder.hpp"
#include "Utilities/Utility.hpp"
#include <opencv2/core/core.hpp>
#include "math.h"
#include <omp.h>

#define EXP 0.3
#define OMP

namespace cc{
	VLADEncoder::VLADEncoder(vl_size numCenters, vl_size dimensions, std::string flags) : Encoder(numCenters * dimensions, dimensions), numCenters(numCenters){
		std::vector<std::string> tokens;
		boost::split(tokens, flags, boost::is_any_of("|"));
		for(size_t i=0; i<tokens.size(); i++){
			if(!tokens[i].compare("VL_VLAD_FLAG_NORMALIZE_COMPONENTS")){
				this->flags = this->flags | VL_VLAD_FLAG_NORMALIZE_COMPONENTS ;
			}
			else if(!tokens[i].compare("VL_VLAD_FLAG_SQUARE_ROOT")){
				this->flags = this->flags | VL_VLAD_FLAG_SQUARE_ROOT;
			}
			else if(!tokens[i].compare("VL_VLAD_FLAG_UNNORMALIZED"))
				this->flags = this->flags | VL_VLAD_FLAG_UNNORMALIZED;
			else if(!tokens[i].compare("VL_VLAD_FLAG_NORMALIZE_MASS"))
				this->flags = this->flags | VL_VLAD_FLAG_NORMALIZE_MASS;
			else
				throw std::runtime_error("Invalid VLAD flag "+tokens[i]);
		}
	}

	void VLADEncoder::computeLCS(cv::Mat1f &descriptors, vl_uint32 *index){

		// this function will compute the rotation matrix for LCS using the training data

		int k = kmeans->numCenters;
		int d = kmeans->dimension;
		int nDescriptors = descriptors.rows;

		// init mu which is the center
		// the size of mu is k x d
		//fix these comments
		// each col of mu is a center which has same size as a descriptor ( k cols )
		// each row of mu is a collection of element from each center ( d rows )
		cv::Mat1f mu = cv::Mat1f(k, d, (float*) kmeans->centers);

		// each row in descriptors is a descriptor
		assert((  descriptors.cols == d)
				&& " dimension of mu and data mismatch");

		// ======= step 1 init temp matrix collection ==========
		// matrixOfCenters[i].row(j) = j-th descriptor assigned to the i-th matrixOfCenters
		// each col of matrixOfCenters[i] is same size as a descriptor ( nDescriptorsOfCenter[i] cols )
		// each row of matrixOfCenters[i] is a collection of element from each computed descriptor ( d rows )
		std::vector<cv::Mat1f> matrixOfCenters(k);


		for(int i=0 ; i < nDescriptors ; i++){
			// we should test also matrixOfCenters[index[i]].push_back(descriptors.row(i));
			matrixOfCenters[index[i]].push_back((descriptors.row(i) - mu.row(index[i]))/cv::norm(descriptors.row(i) - mu.row(index[i])));
			//matrixOfCenters[index[i]].push_back(descriptors.row(i));
		}

		// ======= step 2 perform pca on the descriptors of each center =======
		for ( int i = 0 ; i < k ; i ++ ){
			// Important to aware that data organized as row
			// ATTENTION: it subtract mean automatically
			cv::PCA pca(matrixOfCenters[i], cv::Mat(), CV_PCA_DATA_AS_ROW);

			// before manually apply rotation, remember to subtract mean
			EigenvalueArrayOfRotaionMatrixs.push_back( pca.eigenvalues );
			rotationMatrices.push_back( pca.eigenvectors );
			LCSPCAs.push_back(pca);
		}
	}

	void VLADEncoder::train_(cv::Mat1f &trainData){

		#ifdef CR
		std::cout<<"Computing descriptor PCA..."<<std::endl;
		descriptorPCA = cv::PCA (trainData, cv::Mat(), CV_PCA_DATA_AS_ROW);
		std::cout<<"Computing descriptor PCA done"<<std::endl;
		std::cout<<"Projecting training descriptors..."<<std::endl;
		trainData = descriptorPCA.project(trainData);
		std::cout<<"Projection done"<<std::endl;
		#endif
		// Use Lloyd algorithm
		vl_kmeans_set_algorithm (kmeans, VlKMeansElkan) ;
		// Initialize the cluster centers by randomly sampling the data
		vl_kmeans_set_initialization(kmeans, VlKMeansPlusPlus);
		// Run at most 100 iterations of cluster refinement using Lloyd algorithm
		vl_kmeans_set_max_num_iterations (kmeans, 100) ;
		std::cout<<"Clustering..."<<std::endl;
		vl_kmeans_cluster(kmeans, trainData.data, dimensions, trainData.rows, numCenters);
		std::cout<<"Clustering done"<<std::endl;


		#ifndef VLFEAT
			// if turn on the macro then do the LCS + Whitening.
			// WARNING: rotationMatrix is not saved in file, which is save in memory

			// after train do something to compute the rotationMatrix
			cv::Mat1f &descriptors =  trainData;
			size_t nDescriptors = descriptors.rows;
			vl_uint32 *indexes = (vl_uint32*) vl_malloc(sizeof(vl_uint32) * nDescriptors);
			float * distances = (float *)vl_malloc(sizeof(float) * descriptors.rows) ;
			vl_kmeans_quantize(kmeans,indexes,distances,descriptors.data,nDescriptors);
			// convert indexes array to assignments array,
			// which can be processed by vl_vlad_encode
			std::cout<<"Our VLAD"<<std::endl;
			#ifdef LCS
				std::cout<<"Computin LCS..."<<std::endl;
				VLADEncoder::computeLCS(descriptors, indexes);
			#endif
		#endif


	}



	void VLADEncoder::mapParams(std::vector<std::pair<std::string,std::string>> &params){
		std::string sflag;
		if(flags & VL_VLAD_FLAG_NORMALIZE_COMPONENTS)
			sflag+="NORMALIZE_COMPONENTS";
		if(flags & VL_VLAD_FLAG_SQUARE_ROOT){
			if(!sflag.empty()) sflag += "|";
			sflag+="SQUARE_ROOT";
		}
		if(flags & VL_VLAD_FLAG_UNNORMALIZED){
			if(!sflag.empty()) sflag += "|";
			sflag+="UNNORMALIZED";
		}
		if(flags & VL_VLAD_FLAG_NORMALIZE_MASS){
			if(!sflag.empty()) sflag += "|";
			sflag+= "NORMALIZE_MASS";
		}
		params.push_back(std::pair<std::string,std::string>("Encoder","VLAD"));
		params.push_back(std::pair<std::string,std::string>("numCenters", std::to_string(numCenters)));
		params.push_back(std::pair<std::string,std::string>("dimensions", std::to_string(dimensions)));
		params.push_back(std::pair<std::string,std::string>("flags",sflag));
	}

#pragma omp declare reduction(vec_mat1f_plus : std::vector<cv::Mat1f> : \
std::transform(omp_out.begin(), omp_out.end(), omp_in.begin(), omp_out.begin(), std::plus<cv::Mat1f>()))


	cv::Mat1f VLADEncoder::computeVLAD(cv::Mat1f &descriptors, vl_uint32 *index){

		int k = kmeans->numCenters;
		int d = kmeans->dimension;
		int nDescriptors = descriptors.rows;

		//result.at<float>(i,j) = (i*d+j)-th component of the final VLAD code
		//we will reshape it from (k,d) to (1,k*d) at the end
		#ifdef OMP
			cv::Mat1f result(k,d);
		#else
			cv::Mat1f result = cv::Mat1f::zeros(k, d);
			omp_set_num_threads(1);
			cv::setNumThreads(1);
		#endif
		// centersMat.at<float>(i,j) = j-th element of the i-th center
		cv::Mat1f centersMat = cv::Mat1f(k, d, (float*) kmeans->centers);

		// descriptors.row(i) = i-th descriptor of the image to encode
		assert((  descriptors.cols == d  )
				&& " dimension of mu and data mismatch");

		// ========== Apply RN+LCS (from "Revisiting the VLAD Image Representation") ==========
		// 1. L2-normalize each residual vector
		// 2. Project it using the learnt rotation matrix
		#ifdef OMP
			std::vector<cv::Mat1f> mat(k);
			for(int i=0; i<k; i++)
				mat[i] = cv::Mat1f::zeros(1,d);
			#pragma omp parallel reduction(vec_mat1f_plus : mat)
			{
				mat = std::vector<cv::Mat1f>(k);
				for(int i=0; i<k; i++)
					mat[i] = cv::Mat1f::zeros(1,d);
				#pragma omp for
				for(int i=0; i<nDescriptors; i++){
					cv::Mat1f residual = descriptors.row(i) - centersMat.row(index[i]);
					#ifdef RN
					residual = residual/cv::norm(residual);
					#endif
					mat[index[i]] += residual;
				}
			}
			#ifdef LCS
			#pragma omp parallel for
			for (int i=0; i<k; i++)
				LCSPCAs[i].project(mat[i].row(0)).copyTo(result.row(i));
			#else
				mat[i].row(0).copyTo(result.row(i));
			#endif
		#else
			for(int i=0 ; i < nDescriptors ; i++){
				cv::Mat1f residual = descriptors.row(i) - centersMat.row(index[i]);
				#ifdef RN
				residual = residual/cv::norm(residual);
				#endif
				result.row(index[i]) += residual;
			}
			#ifdef LCS
			for(int i=0; i<k; i++)
				LCSPCAs[i].project(result.row(i)).copyTo(result.row(i));
			#endif
		#endif
		// ========== Signed Square Root (from "All About VLAD") ==========
		// result.at<float>(i,j) =  sign(result.at<float>(i,j)) * (result.at<float>(i,j))^0.5
		#ifdef SSR
		float *rp = result.ptr<float>(0);
		#ifdef OMP
		#pragma omp parallel for simd
		#endif
		for (int i=0; i<result.cols * result.rows; i++)
			if(rp[i]>=0)
				rp[i] = std::pow(rp[i],EXP);
			else
				rp[i] = - std::pow(-rp[i],EXP);
		#endif
		result = result.reshape(1,1); //not it's a (1,k*d)
		assert(( result.cols == k*d) && "result dimension is wrong");
		result = result/cv::norm(result);
		#ifndef OMP
		omp_set_num_threads(0);
		cv::setNumThreads(0);
		#endif
		return result;
	}

	void VLADEncoder::encode_(cv::Mat1f &descriptors, cv::Mat1f &code){
		size_t nDescriptors = descriptors.rows;
		vl_uint32 *indexes = (vl_uint32*) vl_malloc(sizeof(vl_uint32) * nDescriptors);
		float * distances = (float *)vl_malloc(sizeof(float) * descriptors.rows) ;
		vl_kmeans_quantize(kmeans,indexes,distances,descriptors.data,nDescriptors);
		// convert indexes array to assignments array,
		// which can be processed by vl_vlad_encode
		float *assignments = (float*) vl_malloc(sizeof(float) * nDescriptors * numCenters);
		memset(assignments, 0, sizeof(float) * nDescriptors * numCenters);
		for(size_t i = 0; i < nDescriptors; i++) {
		  assignments[i * numCenters + indexes[i]] = 1.;
		}
		#ifndef VLFEAT

					code = computeVLAD(descriptors, indexes);
		#else
					vl_vlad_encode (code.data, VL_TYPE_FLOAT,
									vl_kmeans_get_centers(kmeans), dimensions, numCenters,
									descriptors.data, nDescriptors,
									assignments,
									flags) ;
		#endif
		free(assignments);
		free(distances);
		free(indexes);
	}






	void VLADEncoder::writeTrainer(const std::string &trainerFile){
		cc::WriteData(&numCenters, sizeof(numCenters), trainerFile);
		cc::WriteData(&flags, sizeof(flags), trainerFile);
		cc::WriteData(&kmeans->algorithm, sizeof(kmeans->algorithm), trainerFile);
		cc::WriteData(&kmeans->dataType, sizeof(kmeans->dataType), trainerFile);
		cc::WriteData(&kmeans->dimension, sizeof(kmeans->dimension), trainerFile);
		cc::WriteData(&kmeans->energy, sizeof(kmeans->energy), trainerFile);
		cc::WriteData(&kmeans->initialization, sizeof(kmeans->initialization), trainerFile);
		cc::WriteData(&kmeans->maxNumComparisons, sizeof(kmeans->maxNumComparisons), trainerFile);
		cc::WriteData(&kmeans->maxNumIterations, sizeof(kmeans->maxNumIterations), trainerFile);
		cc::WriteData(&kmeans->minEnergyVariation, sizeof(kmeans->minEnergyVariation), trainerFile);
		cc::WriteData(&kmeans->numCenters, sizeof(kmeans->numCenters), trainerFile);
		cc::WriteData(&kmeans->numRepetitions, sizeof(kmeans->numRepetitions), trainerFile);
		cc::WriteData(&kmeans->numTrees, sizeof(kmeans->numTrees), trainerFile);
		cc::WriteData(&kmeans->verbosity, sizeof(kmeans->verbosity), trainerFile);
		vl_size datasize = vl_get_type_size(kmeans->dataType) * kmeans->numCenters * kmeans->numCenters ;
		cc::WriteData<void>(kmeans->centerDistances, datasize, trainerFile);
		datasize = vl_get_type_size(kmeans->dataType) * kmeans->dimension * kmeans->numCenters;
		cc::WriteData(kmeans->centers, datasize, trainerFile);
		#ifdef LCS
		std::vector<cv::Mat> PCAMats;
		PCAMats.reserve(LCSPCAs.size()*3);
		//write
		for(size_t i=0; i<LCSPCAs.size(); i++){
			PCAMats.push_back(LCSPCAs[i].eigenvectors);
			PCAMats.push_back(LCSPCAs[i].eigenvalues);
			PCAMats.push_back(LCSPCAs[i].mean);
		}
		cc::WriteMatVect(trainerFile, PCAMats);
		#endif
	}
	void VLADEncoder::readTrainer(const std::string &trainerFile){
		size_t offset = 0;
		cc::ReadData<vl_size>(&numCenters, sizeof(vl_size), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<int>(&flags, sizeof(int), trainerFile, offset);
		offset+=sizeof(int);
		cc::ReadData<VlKMeansAlgorithm> (&kmeans->algorithm, sizeof(kmeans->algorithm), trainerFile, offset);
		offset+=sizeof(VlKMeansAlgorithm);
		cc::ReadData<vl_type>(&kmeans->dataType, sizeof(kmeans->dataType), trainerFile, offset);
		offset+=sizeof(vl_type);
		cc::ReadData<vl_size>(&kmeans->dimension, sizeof(kmeans->dimension), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<double>(&kmeans->energy, sizeof(kmeans->energy), trainerFile, offset);
		offset+=sizeof(double);
		cc::ReadData<VlKMeansInitialization>(&kmeans->initialization, sizeof(kmeans->initialization), trainerFile, offset);
		offset+=sizeof(VlKMeansInitialization);
		cc::ReadData<vl_size>(&kmeans->maxNumComparisons, sizeof(kmeans->maxNumComparisons), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<vl_size>(&kmeans->maxNumIterations, sizeof(kmeans->maxNumComparisons), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<double>(&kmeans->minEnergyVariation, sizeof(kmeans->minEnergyVariation), trainerFile, offset);
		offset+=sizeof(double);
		cc::ReadData<vl_size>(&kmeans->numCenters, sizeof(kmeans->numCenters), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<vl_size>(&kmeans->numRepetitions, sizeof(kmeans->numRepetitions), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<vl_size>(&kmeans->numTrees, sizeof(kmeans->numTrees), trainerFile, offset);
		offset+=sizeof(vl_size);
		cc::ReadData<int>(&kmeans->verbosity, sizeof(kmeans->verbosity), trainerFile, offset);
		offset+=sizeof(int);
		vl_size datasize = vl_get_type_size(kmeans->dataType) * kmeans->numCenters * kmeans->numCenters ;
		kmeans->centerDistances = vl_realloc(kmeans->centerDistances, datasize);
		cc::ReadData<void>(kmeans->centerDistances, datasize, trainerFile, offset);
		offset+=datasize;
		datasize = vl_get_type_size(kmeans->dataType) * kmeans->dimension * kmeans->numCenters;
		kmeans->centers = vl_realloc(kmeans->centers, datasize);
		cc::ReadData(kmeans->centers, datasize, trainerFile, offset);
		offset+=datasize;
		//printTrainer();
		#ifdef LCS
		std::vector<cv::Mat> PCAMats;
		cc::ReadMatVect(trainerFile, PCAMats, offset);
		LCSPCAs.resize(kmeans->numCenters);
		for(size_t i=0; i<ceil(PCAMats.size()/3); i++){
			LCSPCAs[i] = cv::PCA();
			LCSPCAs[i].eigenvectors = PCAMats[i*3].clone();
			LCSPCAs[i].eigenvalues 	= PCAMats[i*3+1].clone();
			LCSPCAs[i].mean 		= PCAMats[i*3+2].clone();
		}
		#endif
	}

	void VLADEncoder::printTrainer(){
		std::cout<<"numCenters="<<numCenters<<std::endl<<flags<<std::endl<<kmeans->dataType<<std::endl<<
				kmeans->dimension<<std::endl<<kmeans->energy<<std::endl<<kmeans->initialization<<std::endl<<
				kmeans->maxNumComparisons<<std::endl<<kmeans->maxNumIterations<<std::endl<<kmeans->minEnergyVariation<<std::endl<<
				kmeans->numCenters<<std::endl<<kmeans->numRepetitions<<std::endl<<kmeans->numTrees<<std::endl<<kmeans->verbosity<<std::endl;
		float *p = (float*) kmeans->centerDistances;
		for(size_t i=0;i<kmeans->numCenters * kmeans->numCenters;i++)
			std::cout<<"centerDistances["<<i<<"]="<<p[i]<<std::endl;
		p = (float*) kmeans->centers;
		for(size_t i=0;i<kmeans->dimension * kmeans->numCenters;i++)
			std::cout<<"centers["<<i<<"]="<<p[i]<<std::endl;
	}

}
