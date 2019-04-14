/*
 * CommandLineParser.hpp
 *
 *  Created on: 28 Jan 2017
 *      Author: luca
 */

#ifndef COMMANDLINEPARSER_HPP_
#define COMMANDLINEPARSER_HPP_

#include <memory>

#include "Descriptors/Descriptor.hpp"
#include "Encoders/Encoder.hpp"

namespace cc{
	enum dataset{oxford, holiday, painting, posters, oxfordForCache};
	dataset getDataset(int argc, const char*argv[]);
	std::shared_ptr<cc::Descriptor> makeDescriptor (int argc, const char*argv[]);
	std::shared_ptr<cc::Encoder> makeEncoder(int argc, const char*argv[]);
	template<typename T>
	void getParam(std::vector<std::string> args, const std::string &paramName, T &paramValue);
}

namespace cc{

	template< typename T >
	inline T convert(const char* str)
	{
		std::istringstream iss(str);
		T obj;
		iss >> std::ws >> obj >> std::ws;
		if(!iss.eof())
			throw std::runtime_error ("Cannot convert "+std::string(str)+" the required type");
		return obj;
	}

	template<typename T>
	void getParam(std::vector<std::string> args, const std::string &paramName, T &paramValue){
		std::vector<std::string>::iterator i = find(args.begin(),args.end(),paramName);
		if(i==args.end()){
			return;
		}
		paramValue = convert<T>((*(++i)).c_str());
	}

}


#endif /* COMMANDLINEPARSER_HPP_ */
