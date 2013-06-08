/*
 * Observation.hpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 *
 *      Recorded observation
 *
 */

#ifndef OBSERVATION_HPP_
#define OBSERVATION_HPP_

#include "define.h"
#include <vector>
#include <sstream>

using namespace std;

class Observation {
private:
	O_TYPE _last;
	vector<O_TYPE> _observ_hist;

public:
	Observation();
	O_TYPE getLast() {
		return _last;
	}
	void putNew(O_TYPE value);
	string printOut();
	int length();
	O_TYPE at(int n);

	typedef vector<O_TYPE>::reverse_iterator reverse_iterator;
	reverse_iterator rbegin() {
		return _observ_hist.rbegin();
	}
	reverse_iterator rend() {
		return _observ_hist.rend();
	}
	reverse_iterator rend_window(){
		return (_observ_hist.rbegin()-WINDOW_SIZE);
	}

	O_TYPE at_rit(reverse_iterator rit){
		return (*rit);
	}

	O_TYPE& operator[](const int n){
		return _observ_hist.at(n);
	}
};


#endif /* OBSERVATION_HPP_ */
