/*
 * Device.hpp
 *
 *  Created on: Feb 12, 2012
 *      Author: pom
 *
 *   Class Filter (najskor iba base class)
 *    obsahuje svoje filtre
 *
 */

#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include <vector>
#include "Observation.hpp"


using namespace std;

class Device;

class Device {

public:
	class Filter{
	public:
		int use(Device* dev);
	};

	class Quantizer{
	public:
		O_TYPE use(Device* dev);
	};

private:
	vector<Filter> filters; 			//different between different device types
	Quantizer quantizer; 		//different between different device types
	vector<Observation> observations; 	//same for one hmm system
	virtual int readPending();

public:
	void init();
	void addFilter(Filter f) {filters.push_back(f);};
	void setQuantizer(Quantizer& q) {quantizer = q;};
	void addObservation(Observation o) {observations.push_back(o);}
	void get();
};


#endif /* DEVICE_HPP_ */
