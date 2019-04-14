/*
 * Setup.cpp
 *
 *  Created on: 27 Jan 2017
 *      Author: luca
 */

#include "Utilities/FileSystem.hpp"
#include "CCCore.hpp"
#include "Utilities/Utility.hpp"
#include "Utilities/CommandLineParser.hpp"
#include "Encoders/VLADEncoder.hpp"
#include "DatasetTests/DatasetTest.hpp"

#include <Eigen/SVD>
#include <Eigen/Core>
#include <Eigen/Dense>
#include "opencv2/core/eigen.hpp"

#define APPLY_LCS_AND_WHITENING 1 //toDo


namespace cc{

	CloudCache::CloudCache(	const int argc, const char *argv[],
				const std::string &binDir,
				const int PCA,
				const bool VERBOSE,
				const std::string &ext)
	: binDir(binDir), ext(ext), VERBOSE(VERBOSE),
	  encoder(makeEncoder(argc,argv)), descriptor(makeDescriptor(argc,argv)){
		descDataFile = binDir+"/DescData.bin";
		descTrainFile = binDir+"/DescTrain.bin";
		codesFile = binDir+"/Codes.bin";
		trainerFile = binDir+"/Trainer.bin";
		thresholdFile = binDir+"/Threshold.bin";
		std::vector<std::pair<std::string, std::string>> descParams, encParams;
		bool omp = true;
		unsigned int resizeDim = 0;
		std::vector<std::string> allArgs(argv, argv + argc);
		getParam(allArgs,"OMP",omp);
		getParam(allArgs,"resizeDim",resizeDim);
		getParam(allArgs,"sampled",sampled);
		getParam(allArgs, "PCA", PCAdim);
		descriptor->setResizeDim(resizeDim);
		descriptor->setOMP(omp);
		descriptor->setSamples(sampled); //by default 0 (keep all the descriptors)
		descriptor->mapParams(descParams);
		encoder->mapParams(encParams);
		if(VERBOSE){
			std::cout<<"\t resizeDim="<<resizeDim<<std::endl;
			std::cout<<"\t omp="<<omp<<std::endl;
			std::cout<<"\t sampled="<<sampled<<std::endl;
			for(const std::pair<std::string,std::string> i : encParams)
				std::cout<<"\t "<<i.first<<"="<<i.second;
			std::cout<<std::endl;
			for(const std::pair<std::string,std::string> i : descParams)
				std::cout<<"\t "<<i.first<<"="<<i.second;
			std::cout<<std::endl;
		}
		params.push_back(std::pair<std::string,std::string>("resizeDim",std::to_string(resizeDim)));
		params.push_back(std::pair<std::string,std::string>("PCAdim",std::to_string(PCAdim)));
		params.push_back(std::pair<std::string,std::string>("omp",std::to_string(omp)));
		params.push_back(std::pair<std::string,std::string>("sampled",std::to_string(sampled)));
		params.reserve(params.size()+encParams.size()+descParams.size());
		params.insert(params.end(),encParams.begin(),encParams.end());
		params.insert(params.end(),descParams.begin(),descParams.end());
	}

	void CloudCache::AddTimesToParams(){
		params.push_back(std::pair<std::string,std::string>("descriptorTime",std::to_string(descriptorTime)));
		params.push_back(std::pair<std::string,std::string>("trainingTime",std::to_string(trainingTime)));
	}

	void CloudCache::ComputeDescriptors(const std::string &binDescFile,
										const std::string &imgDir,
										std::vector<cv::Mat1f> &descMats){
		if(!cc::FileExists(binDescFile.c_str())){
			auto startCompare = cc::startTimer();
			descriptor->ComputeDescriptors(imgDir,ext,descMats);
			descriptorTime = cc::stopTimer(startCompare, "descriptor");
			std::cout<<"Writing descriptors..."<<std::endl;
			cc::WriteMatVect(binDescFile, descMats);
		}
		//Read descriptors
		else {
			std::cout<<"Reading descriptors..."<<std::endl;
			cc::ReadMatVect(binDescFile, descMats);
		}
	}

	void CloudCache::Train(const std::string &trainDir){
		std::vector<cv::Mat1f> descMats; //descMats[i]=descriptor matrix of the i-th image
		//If the trainer file doesn't exists, train CloudCache
		//IMPORTANT: remember always to delete the Trainer file if the training dataset has changed
		//The trainer is used later to create new VLAD codes for query images
		if 	(!cc::FileExists(trainerFile.c_str() )) {
			//Compute Descriptors
			std::cout<<"Creating training descriptors..."<<std::endl;
			ComputeDescriptors(descTrainFile, trainDir, descMats);
			cv::Mat1f descBigMat; //it contains all the descriptors from all training images
			auto startTrain = cc::startTimer();
			for(size_t i=0;i<descMats.size();i++)
				//http://stackoverflow.com/questions/38508161/pushing-one-cvmat-into-another-is-going-to-copy-it
				//cv::Mat is similar to a smart pointer, copying it isn't expensive
				descBigMat.push_back(descMats[i]);
			std::cout<<"Training ="<<descBigMat.rows<<" descriptors from "<<descMats.size()<<" images..."<<std::endl;
			encoder->train(descBigMat);
			#ifdef CR
			for(size_t i=0;i<descMats.size();i++){
				if(!descMats[i].empty())
					descMats[i] = encoder->descriptorPCA.project(descMats[i]);
			}
			#endif
			trainingTime = cc::stopTimer(startTrain,"Training");
			std::cout<<"Writing Trainer..."<<std::endl;
			encoder->writeTrainer(trainerFile);
			std::cout<<"Adding times results...."<<std::endl;
			AddTimesToParams();
		}else{
			std::cout<<"Reading Trainer..."<<std::endl;
			encoder->readTrainer(trainerFile);
		}
		trained = true;
	}

	void CloudCache::GenerateCodes(const std::string &dataDir, cv::Mat1f &codes){
		//if the code already exists, read it
		if( !cc::FileExists(codesFile.c_str()) ){
			//CloudCache must be trained in order to generate codes
			if(!trained)
				throw std::runtime_error("CloudCache is not trained! Call Train() first");
			std::vector<cv::Mat1f> descMats; //descMats[i]=descriptor matrix of the i-th image
			ComputeDescriptors(descDataFile, dataDir, descMats);
			int nDesc=0;
			for(size_t i=0; i<descMats.size(); i++)
				nDesc += descMats[i].rows;
			std::cout<<"Computed "<<nDesc<<" descriptors from "<<descMats.size()<<" data images..."<<std::endl;
			//Compute codes
			std::cout<<"Encoding..."<<std::endl;
			auto start = cc::startTimer();
			encoder->encode(descMats,codes);
			encodingTime = cc::stopTimer(start,"encoding");
			if(PCAdim > 0 ){
				if(!PCADone)
					TrainPca(codes);
				std::cout<<"before PCA codes rows="<<codes.rows<<" codes.cols="<<codes.cols<<std::endl;
				DoPca(codes);
				std::cout<<"after PCA codes rows="<<codes.rows<<" codes.cols="<<codes.cols<<std::endl;
			}
			cc::WriteMat(codesFile, codes);
		}
		else{
			std::cout<<"Reading codes..."<<std::endl;
			cc::ReadMat(codesFile, codes);
			if(PCAdim > 0){
				if(!cc::FileExists((binDir+"/vladPCA.bin").c_str()))
					throw ("We have codes file, PCAdim="+std::to_string(PCAdim)+", but vladPCA.bin doesn't exists!");
				//used only to read pca files, no actually training
				TrainPca(codes);
			}
		}
	}

	void CloudCache::GenerateThreshold(const std::string &dataDir, const cv::Mat1f &codes, const std::string &gtDir, const std::unique_ptr<cc::DatasetTest> &test,
			const float minThreshold, const float maxThreshold, const float thresholdStep){
		if 	(!cc::FileExists(thresholdFile.c_str() )) {
			std::vector<queryResults> queries;
			std::vector<std::string> dataFiles; //dataset files
			//get the list of all dataset files
			cc::getAll(dataDir, ".jpg", dataFiles);
			//get all the true positives for the given dataset
			test->getQueriesResults(queries, dataDir, gtDir);
			Rates bestRates;

			float bestOptDist = std::numeric_limits<float>::max(); //closest distance to the optimal point (see below)
			float bestYouden  = std::numeric_limits<float>::min();
			float youdenThreshold = -1;
			//test different threshold values
			for(float threshold = minThreshold; threshold<maxThreshold; threshold+=thresholdStep){
				Rates rates;//Rates for the given threshold
				for(size_t i=0; i<queries.size(); i++){
					std::string query = queries[i].query;
					//check if query exists in the dataset
					cv::Mat1f queryCode;
					for(size_t j=0; j<dataFiles.size(); j++){
						//if query is equal to the given dataset file
						if(! dataFiles[j].compare(query)){
							queryCode = codes.row(j);
							break;
						}
					}
					size_t queryIndex = std::find(dataFiles.begin(), dataFiles.end(), query) - dataFiles.begin();
					//error: we didn't find the query file
					if(queryIndex >= dataFiles.size())
						throw std::runtime_error("Query "+query+" not found");
					//0 codes are for matrices with 0 descriptors, skip it
					if(cv::countNonZero(codes.row(queryIndex))<1)
						continue;
					std::vector<std::string> topkTest;
					std::vector<float> topkDistance;
					test->Topk(topkTest , topkDistance, dataFiles, queryCode, codes, size_t(10));
					//compute the distance between the query and all the other codes and classify them as true/false positives/negatives
					for(int j=0; j<topkTest.size(); j++){
						//if distance is 0 then it's the query itself (don't consider it)
						if(topkDistance[j] == 0)
							continue;
						if(std::find(queries[i].truePositives.begin(), queries[i].truePositives.end(), topkTest[j]) !=
								queries[i].truePositives.end()){
							//true positive
							if(topkDistance[j] <= threshold)
								rates.tp++;
							//false negative
							else
								rates.fn++;
						}
						else{
							//false positive
							if(topkDistance[j] <= threshold)
								rates.fp++;
							//true negative
							else
								rates.tn++;
						}
					}
				}
				rates.queries = queries.size();
				float positives = rates.tp+rates.fn;
				float negatives = rates.fp + rates.tn;
				float TPR = rates.tp == 0 ? 0 : rates.tp / positives;	//True Positive Rate, i.e. recall
				float FPR = rates.fp == 0 ? 0 : rates.fp / negatives;	//False Positive Rate, i.e. 1-specifity
				float specifity = negatives == 0 ? 1 : rates.tn / negatives;
				//distance from the optimal point (0,1) in the ROC space
				float optDist = std::sqrt(std::pow(FPR,2)+std::pow(1-TPR, 2));
				// compute the Youden Index as alternative
				float youdenIndex = TPR + specifity;
				std::cout<<"threshold="<<threshold<<" tp="<<rates.tp<<" tn="<<rates.tn<<" fp="<<rates.fp<<" fn="<<rates.fn<<" FPR="<<FPR<<" TPR="<<TPR<<
						" optDist="<<optDist<<" 1-specifity="<<1-specifity<<" youdenIndex="<<youdenIndex<<std::endl;
				if(optDist < bestOptDist){
					std::cout<<"threshold updated optDist="<<optDist<<" bestOptDist="<<bestOptDist<<std::endl;
					bestOptDist = optDist;
					cacheHitThreshold = threshold;
					bestRates = rates;
				}
				if(youdenIndex > bestYouden){
					std::cout<<"Youden updated youdenIndex="<<youdenIndex<<" bestYouden="<<bestYouden<<std::endl;
					bestYouden = youdenIndex;
				}

			}
			std::cout<<"cacheHitThreshold="<<cacheHitThreshold<<" bestOptDist="<<bestOptDist<<" tp="<<bestRates.tp<<
					" tn="<<bestRates.tn<<" fp="<<bestRates.fp<<" fn="<<bestRates.fn<<std::endl;
			std::cout<<"Writing threshold to file..."<<std::endl;
			std::ofstream out;
			out.open(thresholdFile, std::ios::out | std::ios::binary);
			out.write(reinterpret_cast<const char*>(&cacheHitThreshold), sizeof(float));
		}
		else{
			std::cout<<"Reading threshold from file..."<<std::endl;
			std::ifstream in;
			in.open(thresholdFile, std::ios::out | std::ios::binary);
			in.read(reinterpret_cast<char*>(&cacheHitThreshold), sizeof(float));
			std::cout<<"cacheHitThreshold="<<cacheHitThreshold<<std::endl;
		}
	}


	void Whitening(const cv::Mat &eigenValues, cv::Mat1f &projectedCodes, const int dim){
		if(projectedCodes.cols != dim)
			throw std::runtime_error("Codes are not projected");
		cv::Mat sqrtEigen;
		cv::sqrt(eigenValues, sqrtEigen);
		cv::Mat::diag(cv::Mat(1.0f/(sqrtEigen+1e-09)));
		projectedCodes = projectedCodes * cv::Mat::diag(cv::Mat(1.0f/(sqrtEigen+1e-09)));
		if(projectedCodes.rows > 1){
			#pragma omp parallel for
			for(int i=0; i<projectedCodes.rows; i++)
				cv::normalize(projectedCodes.row(i), projectedCodes.row(i));
		}
		else
			cv::normalize(projectedCodes.row(0), projectedCodes.row(0));
	}

	void CloudCache::TrainPca(const cv::Mat1f &codes){
		std::cout<<"codes rows="<<codes.rows<<" codes.cols="<<codes.cols<<std::endl;
		std::string pcaFile = binDir+"/vladPCA.bin";
		if(!cc::FileExists(pcaFile.c_str())){
			std::cout<<"Computing VLAD PCA..."<<std::endl;
			pca = cv::PCA(codes, cv::Mat1f(), CV_PCA_DATA_AS_ROW, PCAdim);
			std::vector<cv::Mat> pcaMats;
			pcaMats.push_back(pca.eigenvalues);
			pcaMats.push_back(pca.eigenvectors);
			pcaMats.push_back(pca.mean);
			cc::WriteMatVect(pcaFile, pcaMats);
		}
		else{
			std::cout<<"Reading VLAD PCA..."<<std::endl;
			pca = cv::PCA();
			std::vector<cv::Mat> pcaMats;
			cc::ReadMatVect(pcaFile, pcaMats);
			pca.eigenvalues = pcaMats[0];
			pca.eigenvectors = pcaMats[1];
			pca.mean = pcaMats[2];
		}
		PCADone = true;
	}

	void CloudCache::DoPca(cv::Mat1f &codes){
		if(PCADone){
			codes = pca.project(codes);
			Whitening(pca.eigenvalues, codes, PCAdim);
		}
		else
			throw std::runtime_error("PCAdim="+std::to_string(PCAdim)+" but TrainPCA() has never been called!");
	}

	void CloudCache::DoReljaPca(cv::Mat1f &codes, int dim){
		std::cout<<"Doing Relja PCA"<<std::endl;
		codes = codes.t();
		std::cout<<"codes rows="<<codes.rows<<" codes.cols="<<codes.cols<<std::endl;
		cv::Mat1f U, lams, mu, Utmu;
		cc::relja_PCA(U, lams, mu, Utmu, codes, dim);
		cv::Mat1f projectedCodes(dim, codes.cols);
		for(int i=0; i<codes.cols; i++){
			cv::Mat1f projectedCode = U.t()*codes.col(i) - Utmu;
			projectedCode.copyTo(projectedCodes.col(i));
		}
		codes = projectedCodes.t();
		Whitening(lams, codes, dim);
	}

	void CloudCache::DoPcaEigen (cv::Mat1f &codes, int dim){
		std::string pcaFile = binDir+"/egienPCA.bin";

		Eigen::MatrixXf X = Eigen::MatrixXf(codes.rows,codes.cols);
		cv::cv2eigen(codes, X);
		Eigen::MatrixXf aligned = X.rowwise() - X.colwise().mean();

		Eigen::MatrixXf W;

		if(!cc::FileExists(pcaFile.c_str())){

			std::cout<<"Computing Eigen PCA..."<<std::endl;
			// we can directly take SVD
			Eigen::JacobiSVD<Eigen::MatrixXf> svd(aligned, Eigen::ComputeThinV);

			// and here is the question what is the basis matrix and how can i reduce it
			// in my understanding it should be:
			W = svd.matrixV().leftCols(dim);
			cv::Mat1f cvW;
			cv::eigen2cv(W,cvW);
			cc::WriteMat(pcaFile, cvW);
		}
		else{
			cv::Mat1f cvW;
			cc::ReadMat(pcaFile, cvW);
			cv::cv2eigen(cvW, W);
		}
		// then to project the data:
		Eigen::MatrixXf projected = aligned * W; // or should we take a transpose() of W ?
		cv::eigen2cv(projected,codes);
	}


	std::string CloudCache::getExt(){
		return ext;
	}

	bool CloudCache::isTrained(){
		return this->trained;
	}

	float CloudCache::getThreshold(){
		return cacheHitThreshold;
	}

	void CloudCache::setThreshold(const float cacheHitThreshold){
		this->cacheHitThreshold = cacheHitThreshold;
	}

	bool contains_number(const std::string &c)
	{
	    return (c.find_first_of("0123456789") != std::string::npos);
	}

	bool CloudCache::isSameLabel(std::vector<std::string> &lines, const std::string &labels){
			std::reverse(std::begin(lines),std::end(lines));
			for(std::string line : lines){
				//if we meet the same set of labels or a line with only characters (another set of labels)
				//then we write labels too
				if(!line.compare(labels))
					return true;
				if(line.find_first_of("0123456789"))
					return false;
			}
			return false;
	}

	void CloudCache::GenerateQueryCode(const std::string &query, cv::Mat1f &queryCode){
		cv::Mat img = ccImRead(query);
		GenerateQueryCode(img, queryCode);
	}

	void CloudCache::GenerateQueryCode(cv::Mat &query, cv::Mat1f &queryCode){
		if(!trained)
			throw std::runtime_error("CloudCache is not trained! Call Train() first");
		queryCode = cv::Mat1f(1,encoder->codeSize);
		cv::Mat1f queryDescriptors;
		auto startDescriptor = cc::startTimer();
		descriptor->ComputeDescriptors_(query, queryDescriptors);
		totalDescriptorQueryTime += cc::stopTimer(startDescriptor,"query descriptor");
		auto startEncoder = cc::startTimer();
		encoder->encode(queryDescriptors, queryCode);
		if(PCAdim > 0 )
			DoPca(queryCode);
		totalEncoderQueryTime += cc::stopTimer(startEncoder,"query encoder");
		nQueries++;
	}

	float CloudCache::getAvgDescriptorQueryTime(){
		return descriptor->getOnlyDescriptor() / nQueries;
	}

	float CloudCache::getAvgEncoderQueryTime(){
		return totalEncoderQueryTime / nQueries;
	}

	float CloudCache::getOnlyDescriptor(){
		return descriptor->onlyDescriptor;
	}

	int CloudCache::getProcessedQueries(){
		return descriptor->processedImages;
	}

	int CloudCache::getPCAdim(){
		return PCAdim;
	}
}
