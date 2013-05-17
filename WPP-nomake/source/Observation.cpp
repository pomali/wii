/*
 * Observation.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 *
 *      asi jedine na co je tato classa dobra je loose coupling, mozno by bolo prehladnejsie vyhodit ju uplne
 */

#include "define.h"
#include "Observation.hpp"


Observation::Observation() {
	_last = 0;
}

string Observation::printOut() {
	ostringstream output;
	vector<O_TYPE>::const_iterator cii;
	for (cii = _observ_hist.begin(); cii < _observ_hist.end(); cii++)
		output << *cii << " ";
	return output.str();
}

int Observation::length() {
	return _observ_hist.size();
}

void Observation::putNew(O_TYPE value) {
	if (value > O_SYMBOL_COUNT)
		return;
	_observ_hist.push_back(value);
	_last = value;
}

O_TYPE Observation::at(int n) {
	return _observ_hist.at(n);
}
