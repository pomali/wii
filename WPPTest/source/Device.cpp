/*
 * Device.cpp
 *
 *  Created on: Feb 12, 2012
 *      Author: pom
 *
 *
 */

#include "Device.hpp"
#include "Observation.hpp"

void Device::get(){
	readPending();
	for (vector<Filter>::iterator i=filters.begin(); i<filters.end(); i++){
		if ( i->use(this) < 0)
			return;
	}

	for (vector<Observation>::iterator i=observations.begin(); i<observations.end(); i++){
		i->putNew(quantizer.use(this));
	}
}

void Device::init(){
	//pridame default quantizre
	//pridame default filtrey

}


int Device::Filter::use(Device* dev){

	return 0;
}

O_TYPE Device::Quantizer::use(Device* dev){

	return 0;
}
