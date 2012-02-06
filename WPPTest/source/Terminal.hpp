/*
 * Terminal.hpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */

#ifndef TERMINAL_HPP_
#define TERMINAL_HPP_
#include "define.h"

#ifdef FORWII
#include <grrlib.h>
#include "FreeMonoBold_ttf.h"
#endif

#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

class Terminal { //TODO: dorobit class STDTerminal na vypis na stdout
protected:
	vector<string> lines;
	int _lastline;
public:
	void clear(){ lines.clear();}
	virtual void printAll() = 0;
	virtual void addLine(string s) = 0;
};



class STDTerminal: public Terminal {
public:
	STDTerminal();
	void clear();
	void printAll();
	void addLine(string s);
#ifdef FORWII
private:
	void *xfb;
	GXRModeObj *rmode;
#endif
};

#ifdef FORWII
class GRRTerminal: public Terminal {
private:
	GRRLIB_ttfFont *_font;
	int _size; //size of line
	u32 _color; // color of letters
	int _x, _y; // top left corner of "window"
	int _pos; //position of cursor (last line written)
public:
	GRRTerminal(const u8* font_ttf, s32 font_ttf_size);
	void addLine(string s);
	void printAll();
};
#endif

#endif /* TERMINAL_HPP_ */
