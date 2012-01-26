/*
 * main.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: pom
 */

#include <stdlib.h>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include "FreeMonoBold_ttf.h"

using namespace std;

#define O_TYPE int
#define O_SYMBOL_COUNT 2
#define HMM_PROB_TYPE double
#define HMM_STATE_COUNT 2
#define WIN true

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
	int lenght();
	O_TYPE at(int n);
	//TODO: iterators
	typedef vector<O_TYPE>::reverse_iterator reverse_iterator;
	reverse_iterator rbegin() {
		return _observ_hist.rbegin();
	}
	reverse_iterator rend() {
		return _observ_hist.rend();
	}
};

Observation::Observation() {
	_last = 0;
}

string Observation::printOut() {
	stringstream output;
	vector<O_TYPE>::const_iterator cii;
	for (cii = _observ_hist.begin(); cii != _observ_hist.end(); cii++)
		output << *cii << " ";
	return output.str();
}

int Observation::lenght() {
	return _observ_hist.size();
}

void Observation::putNew(O_TYPE value) {
	_observ_hist.push_back(value);
	_last = value;
}

O_TYPE Observation::at(int n) {
	return _observ_hist.at(n);
}


class Terminal{ //TODO: dorobit class STDTerminal na vypis na stdout
protected:
	vector<string> lines;
	int _lastline;
public:
	void clear();
	virtual void printAll() = 0;
	virtual void addLine(string s) = 0;
};

void Terminal::clear() {
	lines.clear();
}

class STDTerminal : public Terminal {
public:
	STDTerminal();
	void clear();
	void printAll();
	void addLine(string s);
private:
	void *xfb;
	GXRModeObj *rmode;
};


void STDTerminal::clear() {
	lines.clear();
	printf("\x1b[2J");
}

STDTerminal::STDTerminal(){
//	TODO:osetrit ak je video uz initnute
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void STDTerminal::addLine(string s){
	lines.push_back(s);
	printf(s.c_str());
	printf("\n");
	VIDEO_WaitVSync();
};
void STDTerminal::printAll(){
	printf("\x1b[2J");
	vector<string>::const_iterator cii;
	for (cii = lines.begin(); cii != lines.end(); cii++) {
		printf((*cii).c_str());
		printf("\n");
	}
	VIDEO_WaitVSync();
}


class GRRTerminal : public Terminal {
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
	for (cii = lines.begin(); cii != lines.end(); cii++) {
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


//------------------------
class HMM {
private:
	vector<HMM_PROB_TYPE > start_P; // start_P[S] probability of starting in state S
	vector<vector<HMM_PROB_TYPE> > trans_P; //trans_P[i][j] probability of moving from state i to state j
	vector<map<O_TYPE, HMM_PROB_TYPE> > emiss_P; //emiss_P[S][O] probability of emitting O in state S

public:
	HMM(void);
	HMM(Terminal& term);
	HMM_PROB_TYPE Viterbi(Observation o, Terminal& term); //TODO: prerobit Viter,Fwd,Back do log() priestoru
	HMM_PROB_TYPE Viterbi(Observation o); //FIXME: mozno by som mal prerobit na predavanie referencii
	void Print(Terminal& term);
	void Train(Observation o);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o, Terminal& term);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o, Terminal& term);
};

HMM::HMM(void) {
	const HMM_PROB_TYPE s_p = static_cast<float> (1) / HMM_STATE_COUNT;
	const HMM_PROB_TYPE e_p = static_cast<float> (1) / O_SYMBOL_COUNT;
	map<O_TYPE, HMM_PROB_TYPE> temp_e;
	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)
		temp_e.insert(pair<O_TYPE, HMM_PROB_TYPE> (j, e_p));

	for (int i = 0; i < HMM_STATE_COUNT; i++)
		start_P.push_back(s_p);

	for (int i = 0; i < HMM_STATE_COUNT; i++) {
		trans_P.push_back(start_P);
		emiss_P.push_back(temp_e);
	}
}

void HMM::Print(Terminal& term) {
	stringstream line;

	line << "Start_P:";
	for (int i = 0; i < HMM_STATE_COUNT; i++) {
		line << " " << start_P[i];
	}
	term.addLine(line.str());
	line.str("");

	term.addLine("Trans_P:");
	for (int i = 0; i < HMM_STATE_COUNT; i++) {
		for (int j = 0; j < HMM_STATE_COUNT; j++)
			line << " " << trans_P[i][j];
		term.addLine(line.str());
		line.str("");
	}

	term.addLine("Emiss_P:"); //emiss_P;
	for (map<O_TYPE, HMM_PROB_TYPE>::const_iterator mit = emiss_P.front().begin(); mit
					!= emiss_P.front().end(); mit++)
				line << " " << (*mit).first;
	term.addLine(line.str());
	line.str("");

	for (vector<map<O_TYPE, HMM_PROB_TYPE> >::const_iterator vit =
			emiss_P.begin(); vit < emiss_P.end(); vit++) {
		for (map<O_TYPE, HMM_PROB_TYPE>::const_iterator mit = (*vit).begin(); mit
				!= (*vit).end(); mit++)
			line << " " << (*mit).second;
		term.addLine(line.str());
		line.str("");
	}
}


HMM_PROB_TYPE HMM::Viterbi(Observation o, Terminal& term) {

	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;
	stringstream line;

	term.addLine("Viterbi");
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
		line << " " << *it;
	}
	V.push_back(tmp_line);
	term.addLine(line.str());

	for (int observ_char = 0; observ_char < o.lenght(); observ_char++) {
		tmp_line.clear();
		line.str("");
		line << o.at(observ_char) << ":";
		for (int dest_state = 0; dest_state < HMM_STATE_COUNT; dest_state++) {
			previous_max = 0;
			for (int source_state = 0; source_state < HMM_STATE_COUNT; source_state++) {
				previous_max = max(previous_max, (V[observ_char][source_state] //observ_char bcs V[0] is 0 chars read
													* trans_P[source_state][dest_state]
													* emiss_P[dest_state][o.at(observ_char)])); // o.at(observ_char) bsc its char at position 0
			}
			tmp_line.push_back(previous_max);
			line << " " << previous_max;
		}
		V.push_back(tmp_line);
		term.addLine(line.str());
	}

	previous_max = 0;
	//	find probability of best path
	for (int i = 0; i < HMM_STATE_COUNT; i++) {
		previous_max = max(previous_max, V[o.lenght()][i]);

	}

	return previous_max;
}


vector<vector<HMM_PROB_TYPE> > HMM::Forward(Observation o, Terminal& term) {
	vector<vector<HMM_PROB_TYPE> > F; //F[i][j]probability of being in state j and emitting i-th symbol from o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;
	stringstream line;

	term.addLine("Forward");
	//fill F with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
		line << " " << *it;
	}
	F.push_back(tmp_line);
	term.addLine(line.str());

	for (int observ_char = 0; observ_char < o.lenght(); observ_char++) {
		tmp_line.clear();
		line.str("");
		line << o.at(observ_char) << ":";
		for (int dest_state = 1; dest_state < HMM_STATE_COUNT; dest_state++) {
			sum = 0;
			for (int source_state = 0; source_state < HMM_STATE_COUNT; source_state++) {
				sum += F[observ_char][source_state]
						* trans_P[source_state][dest_state];
			}
			tmp_line.push_back(sum * emiss_P[dest_state][o.at(observ_char)]); // o.at(observ_char) bsc its char at position 0
			line << " " << sum * emiss_P[dest_state][o.at(observ_char)];
		}
		F.push_back(tmp_line);
		term.addLine(line.str());
	}

	return F;
}


vector<vector<HMM_PROB_TYPE> > HMM::Backward(Observation o, Terminal& term) {
	vector<vector<HMM_PROB_TYPE> > B;//FIXME:WRONG:B[i][j] probability of being in state j after i-th symbol from o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;
	stringstream line;

	term.addLine("Backward");
	//fill F with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(1);
		line << " " << 1;
	}
	B.push_back(tmp_line); //B will be filled in "reverse"
	term.addLine(line.str());

	for (vector<O_TYPE>::reverse_iterator observ_rit = o.rbegin();
			observ_rit < o.rend(); observ_rit++) {
		tmp_line.clear();
		line.str("");
		line << *observ_rit << ":";
		for (int dest_state = HMM_STATE_COUNT-1; dest_state >= 0 ; dest_state--) {
			sum = 0;
			for (int source_state = HMM_STATE_COUNT-1 ; source_state >= 0; source_state--) {
				sum += B.back()[source_state]
						* trans_P[dest_state][source_state]
						* emiss_P[source_state][*observ_rit];
			}
			tmp_line.push_back(sum);
			line << " " << sum;
		}

		B.push_back(tmp_line);
		term.addLine(line.str());
	}
	reverse(B.begin(), B.end());
	return B;
}

void HMM::Train(Observation o) {

}

int restart = 0;

int main(int argc, char **argv) {

//	WPADData *wd;
//	WPAD_Init();
//	WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC);


//	GRRLIB_Init();
//	GRRLIB_Settings.antialias = true;
//	GRRLIB_SetBackgroundColour(0x00, 0x00, 0x00, 0xFF);
//	GRRLIB_Camera3dSettings(0.0f, 0.0f, 13.0f, 0, 1, 0, 0, 0, 0);
//	GRRTerminal gt(FreeMonoBold_ttf, FreeMonoBold_ttf_size);

	STDTerminal gt;

	stringstream a;
	HMM hmm1;
	Observation obs;
	obs.putNew(0);
	obs.putNew(1);
//	obs.putNew(2);
//	obs.putNew(3);
	obs.putNew(0);
//	obs.putNew(2);
	obs.putNew(1);
	hmm1.Print(gt);
	hmm1.Backward(obs, gt);
	hmm1.Forward(obs, gt);
	hmm1.Viterbi(obs, gt);
	gt.addLine("dopisane");
	gt.printAll();
//	GRRLIB_Render();

	while(1);

//	while (!restart) {
//		WPAD_ReadPending(WPAD_CHAN_ALL, NULL);
//		wd = WPAD_Data(0);
//		//		a.str("");
//		//		a << hmm1.Viterbi(obs);
//		//		gt.addLine(a.str());
//		if (wd->btns_d & WPAD_BUTTON_HOME) {
//			restart = 1;
//			gt.addLine("koncime");
//		}
//		gt.printAll();
//		GRRLIB_Render();
//	}

	exit(0);
}
