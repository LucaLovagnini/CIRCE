/*
 * FileSystem.hpp
 *
 *  Created on: 9 Dec 2016
 *      Author: luca
 */

#ifndef FILESYSTEM_HPP_
#define FILESYSTEM_HPP_

#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <iostream>

#include <boost/filesystem.hpp>

#include "opencv2/core.hpp"

namespace fs = ::boost::filesystem;

namespace cc{

	bool FileExists(const char* file);
	void WriteMat(const std::string& filename, const cv::Mat& mat, const bool append = false);
	size_t ReadMat(const std::string& filename, cv::Mat &mat, const size_t &offset = 0);
	void getAll(const std::string& dirPath, const std::string& ext, std::vector<std::string>& files);

	template<typename T>
	bool WriteData( const T* pdata, size_t bytes, const std::string& file_path)
	{
		std::ofstream os(file_path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
		if ( !os.is_open() )
			return false;
		os.write(reinterpret_cast<const char*>(pdata), std::streamsize(bytes));
		os.close();
		return true;
	}

	template<typename T>
	bool ReadData( T* pdata, size_t bytes, const std::string& file_path, const size_t pos=0)
	{
		std::ifstream is(file_path.c_str(), std::ios::binary | std::ios::in);
		if (!is)
			return false;
		is.seekg (pos, is.beg);
		is.read(reinterpret_cast<char*>(pdata), std::streamsize(bytes));
		is.close();
		return true;
	}

	template<typename T>
	void WriteMatVect(const std::string& filename, const std::vector<T>& mats){
		if(mats.empty()){
			std::runtime_error("WriteMatVect: no cv::Mat in mats");
			return;
		}
		size_t size = mats.size();
		WriteData(&size,sizeof(size),filename);
		for(size_t i=0 ; i<mats.size() ; i++)
			WriteMat(filename, mats[i], true);
	}

	template<typename T>
	void ReadMatVect(const std::string& filename, std::vector<T>& mats, size_t offset=0){
		size_t size;
		ReadData<size_t> (&size, sizeof(size), filename, offset);
		offset += sizeof(size);
		mats.reserve(size);
		for(size_t i=0 ; i<size ; i++){
			cv::Mat mat;
			offset += ReadMat(filename, mat, offset);
			mats.push_back(mat);
		}
	}

}

#endif
