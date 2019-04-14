/*
 * HolidayTest.cpp
 *
 *  Created on: 3 Mar 2017
 *      Author: luca
 */

#include <cstdio>
#include <fstream>
#include <queue>
#include "python2.7/Python.h"

#include "HolidayTest.hpp"
#include "Utilities/FileSystem.hpp"

namespace cc{

	float HolidayTest::computeMap(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir, const float cutK){
		size_t k = (size_t) codes.rows < 10000 ? (size_t) codes.rows : 10000;
		std::ofstream resultFile;
		std::vector<std::string> holidayFiles;
		//it's important to use getAll since codes follow the same order
		//first line is relative to holiday_files[0] and so on
		cc::getAll(dataDir, ".jpg", holidayFiles);
		for(size_t i=0; i<holidayFiles.size(); i++){
			holidayFiles[i] = holidayFiles[i].substr(holidayFiles[i].find_last_of("/\\")+1);//get file name
		}
		if((int) holidayFiles.size() != codes.rows)
			throw std::runtime_error("holidayFiles.size()="+std::to_string(holidayFiles.size())+" codes.rows="+std::to_string(codes.rows));
		resultFile.open(gtDir+"/cc_result.dat");
		for(int i=0; i<500; i++){
			std::string query = std::to_string(i+1000)+"00.jpg";
			size_t pos = std::find(holidayFiles.begin(), holidayFiles.end(), query) - holidayFiles.begin();
			if( pos < holidayFiles.size() ) {
					cv::Mat1f queryCode = codes.row(pos);
					std::vector<std::string> topkTest;
					std::vector<float> topkDistance;
					resultFile<<query;
					Topk(topkTest , topkDistance, holidayFiles, queryCode, codes, k);
					for(size_t j=0; j<topkTest.size(); j++)
						resultFile<<" "<<j<<" "<<topkTest[j];
					resultFile<<"\n";
			}
			else
				throw std::runtime_error("query "+query+" not found in "+dataDir);
		}
		resultFile.close();
		Py_Initialize();
	    PyRun_SimpleString ("import sys; sys.path.insert(0, '../Utilities')");
		PyObject* myModuleString = PyString_FromString((char*)"holidays_map");
		PyObject* myModule = PyImport_Import(myModuleString);
		PyObject* myFunction = PyObject_GetAttrString(myModule,(char*)"getMap");
		PyObject* args = PyTuple_Pack(1,PyString_FromString((gtDir+"/cc_result.dat").c_str()));
		PyObject* myResult = PyObject_CallObject(myFunction, args);
		return PyFloat_AsDouble(myResult);
	}

	float HolidayTest::computeMrr(const cv::Mat1f &codes, const std::string &dataDir, const std::string &gtDir){
		throw std::runtime_error("computMrr not supported for holiday dataset!");
	}

	void HolidayTest::getQueriesResults(std::vector<queryResults> &queries, const std::string &dataDir, const std::string &gtDir){
		std::vector<std::string> holidayFiles;
		std::ifstream imagesFs (gtDir+"/holidays_images.dat");
		std::string image;
		while(std::getline(imagesFs, image))
			holidayFiles.push_back(image);

		for(int i=0; i<500; i++){
			queryResults q;
			std::string queryPrefix = std::to_string(i+1000);
			//if query not found, throw exception
			q.query = dataDir+"/"+queryPrefix+"00.jpg";
			for(size_t i=0; i<holidayFiles.size(); i++){
				if(!holidayFiles[i].compare(q.query))
					continue;
				if(!holidayFiles[i].compare(0, queryPrefix.length(), queryPrefix)){
					q.truePositives.push_back(dataDir+"/"+holidayFiles[i]);
				}
			}
			queries.push_back(q);
		}
	}
}
