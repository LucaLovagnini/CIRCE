/*
 * Setup.hpp
 *
 *  Created on: 27 Jan 2017
 *      Author: luca
 */

#ifndef CCCORE_HPP_
#define CCCORE_HPP_

#include <string>
#include <utility>
#include <fstream>

#include "Utilities/FileSystem.hpp"
#include "Encoders/Encoder.hpp"
#include "Descriptors/Descriptor.hpp"

#include "DatasetTests/DatasetTest.hpp"
#include "DatasetTests/OxfordTestManager.hpp"
#include "DatasetTests/HolidayTest.hpp"
#include "Descriptors/SIFTOpenCV.hpp"
#include "Descriptors/DSIFTVLFeat.hpp"
#include "Descriptors/SURFOpenCV.hpp"
#include "Descriptors/SIFTVLFeat.hpp"
#include "Encoders/VLADEncoder.hpp"
#include "Encoders/FisherVectorEncoder.hpp"
#include "Utilities/Utility.hpp"
#include "Utilities/EuclideanDistance.hpp"


namespace cc{

	class CloudCache {
	public:
		CloudCache(	const int argc, const char *argv[],
					const std::string &binDir,
					const int PCAdim = 0,
					const bool VERBOSE=true,
					const std::string &ext = ".jpg");
		void Train(const std::string &trainDir);
		void GenerateCodes(const std::string &dataDir, cv::Mat1f &codes);
		void GenerateThreshold(const std::string &dataDir, const cv::Mat1f &codes, const std::string &gtDir, const std::unique_ptr<cc::DatasetTest> &test,
				const float minThreshold=0.5, const float maxThreshold=2, const float thresholdStep = 0.05);
		void DoPca(cv::Mat1f &codes);
		void DoPcaEigen (cv::Mat1f &codes, int dim=128);
		void DoReljaPca (cv::Mat1f &codes, int dim=128);
		void GenerateQueryCode(const std::string &query, cv::Mat1f &queryCode);
		void GenerateQueryCode(cv::Mat &query, cv::Mat1f &queryCode);
		std::string getExt();
		cv::Mat1f getCodes();
		bool isTrained();
		float getThreshold();
		float getAvgDescriptorQueryTime();
		float getAvgEncoderQueryTime();
		float getOnlyDescriptor();
		int   getPCAdim();
		int getProcessedQueries();
		void setThreshold(const float cacheHitThreshold);
		template <typename T, typename ... Ts>
		void WriteOds(const std::string &fileName, std::vector<std::string> extraLabels, T &first, Ts ... ts);
		template <typename T>
		void WriteTimes(const std::string &fileName, const std::vector<T> &keys, const std::vector<float> &times,
				const float totalCodeTime, const float min, const float max, const float avg);
		void TrainPca(const cv::Mat1f &codes);
	private:
		void AddTimesToParams();
		template <typename T>
		void WriteOds (std::ofstream &ods, T &elem);
		template <typename T, typename ... Ts>
		void WriteOds (std::ofstream &ods, T &elem, Ts ... ts);
		void ComputeDescriptors(const std::string &binDescFile,
								const std::string &imgDir,
								std::vector<cv::Mat1f> &descMats);
		bool isSameLabel(std::vector<std::string> &lines, const std::string &labels);
		std::string binDir, ext, descDataFile, descTrainFile, codesFile, trainerFile, thresholdFile;
		std::vector<std::pair<std::string,std::string>> params;
		bool VERBOSE;
		std::shared_ptr<cc::Encoder> encoder;
		std::shared_ptr<cc::Descriptor> descriptor;
		double descriptorTime = 0, trainingTime = 0, encodingTime = 0;
		unsigned int sampled = 0;
		bool trained = false; //if true, CC can receive queries
		bool codesGenerated = true;
		float cacheHitThreshold = -1;
		float totalDescriptorQueryTime = 0; //used by GenerateQueryCode
		float totalEncoderQueryTime = 0;			//used by GenerateQueryCode
		float nQueries = 0;
		int PCAdim = 0;
		bool PCADone = false;
		cv::PCA pca;
	};
}


namespace cc{

	template <typename T>
	void CloudCache::WriteOds (std::ofstream &ods, T &elem){
		ods<<elem<<"\n";
	}

	template <typename T, typename ... Ts>
	void CloudCache::WriteOds (std::ofstream &ods, T &elem, Ts ... ts){
		ods<<elem<<"\t";
		WriteOds(ods,ts...);
	}

	template <typename T, typename ... Ts>
	void CloudCache::WriteOds(const std::string &fileName, std::vector<std::string> extraLabels, T &first, Ts ... ts){
		std::ofstream ods;
		std::string labels;
		for(std::pair<std::string,std::string> i : params)
			labels.append(i.first+" \t ");
		for(size_t i=0;i<extraLabels.size();i++)
			labels.append(extraLabels[i]+" \t");
		if(cc::FileExists(fileName.c_str())){
			ods.open(fileName.c_str(), std::ios::out | std::ios::app);
			//check that the last line containing string labels is the same as the actual params
			std::ifstream iods(fileName);
			std::string line;
			std::vector<std::string> lines;
			while (std::getline(iods, line))
			   lines.push_back(line);
			if(!isSameLabel(lines,labels)){
				ods<<labels;
				ods<<"\n";
			}
		}
		else{
			ods.open(fileName.c_str(), std::ios::out);
			ods<<labels;
			ods<<"\n";
		}
		for(std::pair<std::string,std::string> i : params)
			ods<<i.second<<"\t";
		WriteOds (ods, first, ts...);
		ods.close();
	}
	template <typename T>
	void CloudCache::WriteTimes(const std::string &fileName, const std::vector<T> &keys, const std::vector<float> &times,
			const float totalCodeTime, const float min, const float max, const float avg){
		std::ofstream ods;
		if(cc::FileExists(fileName.c_str()))
			ods.open(fileName.c_str(), std::ios::out | std::ios::app);
		else
			ods.open(fileName.c_str(), std::ios::out);
		ods<<"min="<<min<<"\n";
		ods<<"max="<<max<<"\n";
		ods<<"avg="<<avg<<"\n";
		ods<<"total="<<totalCodeTime<<"\n";
		ods<<"Keys\tTimes\n";
		for(size_t i=0; i<keys.size(); i++)
			ods<<keys[i]<<"\t"<<times[i]<<"\n";
	}
}

#endif /* CCCORE_HPP_ */
