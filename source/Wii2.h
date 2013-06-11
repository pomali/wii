/*
 * Wii2.h
 *
 *  Created on: Jun 12, 2013
 *      Author: pom
 */

#ifndef WII2_H_
#define WII2_H_

#include <wiiuse.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <vector>
#include <fstream>

class Wii2 {
private:
	wiimote** wiimotes;
	int found, connected;
	struct wiimote_t* wm;

public:
	Wii2();
	int init();
	int main();
	int record();
	int v3c2symb(vec3b_t v);

	virtual ~Wii2();
};

#endif /* WII2_H_ */
