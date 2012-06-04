/*
 * Terminal.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */

#include "Terminal.hpp"

void STDTerminal::clear() {
	lines.clear();
	printf("\x1b[2J");
}

STDTerminal::STDTerminal() {
#ifdef FORWII
	//	TODO:osetrit ak je video uz initnute
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth
			* VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
	VIDEO_WaitVSync();
#endif
}

void STDTerminal::addLine(string s) {
	lines.push_back(s);
	printf("%s\n", s.c_str());
#ifdef FORWII
	VIDEO_WaitVSync();
#endif
}

void STDTerminal::addLine(string s,int verbosity) {
	lines.push_back(s);
	printf("%s\n", s.c_str());
#ifdef FORWII
	VIDEO_WaitVSync();
#endif
}

void STDTerminal::printAll() {
	printf("\x1b[2J");
	vector<string>::const_iterator cii;
	for (cii = lines.begin(); cii < lines.end(); cii++) {
		printf("%s\n", (*cii).c_str());
	}
#ifdef FORWII
	VIDEO_WaitVSync();
#endif
}

#ifdef FORWII
GRRTerminal::GRRTerminal(const u8* font_ttf, s32 font_ttf_size) {
	_lastline = 0;
	_size = 15;
	_color = 0xFFFFFFFF;
	_x = 20;
	_y = 20;
	_pos = 0;
	_font = GRRLIB_LoadTTF(font_ttf, font_ttf_size);

	//	TODO:osetrit ak grrlib nieje initnute
}

void GRRTerminal::printAll() {
	int pos = 0;
	vector<string>::const_iterator cii;
	GRRLIB_2dMode();
	for (cii = lines.begin(); cii < lines.end(); cii++) {
		GRRLIB_PrintfTTF(_x, _y + (_size * 1.1) * pos++, _font, (*cii).c_str(),
				_size, _color);
	}
}

void GRRTerminal::addLine(string s) {
	lines.push_back(s);
	GRRLIB_2dMode();
	GRRLIB_PrintfTTF(_x, _y + (_size * 1.1) * _lastline++, _font, s.c_str(),
			_size, _color);
}
#endif
