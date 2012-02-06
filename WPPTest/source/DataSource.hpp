/*
 * DataSource.hpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */

#ifndef DATASOURCE_HPP_
#define DATASOURCE_HPP_

//#include <wiiuse/wpad.h>
#include <vector>
#include <iostream>
class DataSource;

class Observer {
public:
	virtual void notify(DataSource* s) = 0;
};

class DataSource {
private:
	std::vector<Observer*> observers;
protected:
	void notify_observers() {
		std::vector<Observer*>::iterator i;
		for (i = observers.begin(); i < observers.end(); i++) {
			(*i)->notify(this);
		}
	}
public:
	void register_observer(Observer *o) {
		observers.push_back(o);
	}
};

class DataSource_Keyboard: public DataSource {

};

class DataSource_Wii: public DataSource {
public:
	DataSource_Wii();
	void triggered();
};

#endif /* DATASOURCE_HPP_ */
