/*
 * FileSystem.cpp
 *
 *  Created on: 9 Dec 2016
 *      Author: luca
 */

#include "Utilities/FileSystem.hpp"

namespace cc{
	bool FileExists(const char* file) {
			struct stat buf;
			return (stat(file, &buf) == 0);
	}

	void WriteMat(const std::string& filename, const cv::Mat& mat, const bool append){
		std::ofstream fs;
		if(append)
			fs.open(filename.c_str(), std::fstream::binary | std::fstream::app);
		else
			fs.open(filename.c_str(), std::fstream::binary);

		// Header
		int type = mat.type();
		fs.write((char*)&mat.rows, sizeof(int));    // rows
		fs.write((char*)&mat.cols, sizeof(int));    // cols
		fs.write((char*)&type, sizeof(int));        // type

		// Data
		if (mat.isContinuous())
		{
			fs.write(mat.ptr<char>(0), (mat.dataend - mat.datastart));
		}
		else
		{
			int rowsz = CV_ELEM_SIZE(type) * mat.cols;
			for (int r = 0; r < mat.rows; ++r)
			{
				fs.write(mat.ptr<char>(r), rowsz);
			}
		}
	}

	size_t ReadMat(const std::string& filename, cv::Mat &mat, const size_t &offset){
		std::ifstream fs(filename, std::fstream::binary);
		fs.seekg(offset);
		// Header
		int rows, cols, type;
		fs.read((char*)&rows, sizeof(int));         // rows
		fs.read((char*)&cols, sizeof(int));         // cols
		fs.read((char*)&type, sizeof(int));         // type

		// Data
		mat.create(rows,cols, type);
		fs.read((char*)mat.data, CV_ELEM_SIZE(type) * rows * cols);

		return 3 * sizeof(int) + CV_ELEM_SIZE(type) * rows * cols;
	}

	void getAll(const std::string& dirPath, const std::string& ext, std::vector<std::string>& files){
		fs::recursive_directory_iterator it(dirPath);
		fs::recursive_directory_iterator endit;

		while(it != endit){
			if(fs::is_regular_file(*it) && it->path().extension() == ext){
				files.push_back(it->path().string());
			}
			it++;
		}

	}

}
