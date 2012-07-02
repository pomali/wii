/*
 * WiimoteTest.h
 *
 *  Created on: Jun 30, 2012
 *      Author: pom
 */

#ifndef WIIMOTETEST_H_
#define WIIMOTETEST_H_

#include <wiiuse.h>
#include <unistd.h>
#define MAX_WIIMOTES	4

class WiimoteTest {
public:
	void handle_event(struct wiimote_t* wm);
	void handle_ctrl_status(struct wiimote_t* wm);
	void handle_disconnect(wiimote* wm);
	void test(struct wiimote_t* wm, byte* data, unsigned short len);
	void handle_read(struct wiimote_t* wm, byte* data, unsigned short len);
	int main(int argc, char** argv);
	WiimoteTest();

private:
	wiimote** wiimotes_pp;
	int wm_found,wm_connected;


};

#endif /* WIIMOTETEST_H_ */
