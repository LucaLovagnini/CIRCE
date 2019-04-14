/*
 * Distance.hpp
 *
 *  Created on: 7 Mar 2017
 *      Author: luca
 */

#ifndef UTILITIES_DISTANCE_HPP_
#define UTILITIES_DISTANCE_HPP_

namespace cc{
	template<typename C, typename D>
	class Distance{
	public:
		virtual D compute (const C &first, const C &second) = 0;
		virtual ~Distance();
	};
	template<typename C, typename D>
	Distance<C,D>::~Distance(){}
}

#endif /* UTILITIES_DISTANCE_HPP_ */
