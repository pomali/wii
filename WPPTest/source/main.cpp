/*
 * main.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: pom
 */

//#define FORWII

#include <stdlib.h>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>

#ifdef FORWII
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include "FreeMonoBold_ttf.h"
#endif
#ifndef FORWII
#include <stdio.h>
#endif

using namespace std;

#define O_TYPE int
#define O_SYMBOL_COUNT 5
#define HMM_PROB_TYPE double
#define HMM_STATE_COUNT 3
#define VERBOSITY 1

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
	ostringstream output;
	vector<O_TYPE>::const_iterator cii;
	for (cii = _observ_hist.begin(); cii < _observ_hist.end(); cii++)
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

class Terminal { //TODO: dorobit class STDTerminal na vypis na stdout
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
;
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

void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term) {
	ostringstream line;
	line << fixed;
	for (vector<vector<HMM_PROB_TYPE> >::const_iterator i = V.begin(); i
			< V.end(); i++) {
		line.str("");
		for (vector<HMM_PROB_TYPE>::const_iterator j = (*i).begin(); j
				< (*i).end(); j++) {
			line << *j << " ";
		}
		term.addLine(line.str());
	}
}

//------------------------
class HMM {
public:
	//private
	vector<HMM_PROB_TYPE > start_P; // start_P[S] probability of starting in state S
	vector<vector<HMM_PROB_TYPE> > trans_P; //trans_P[i][j] probability of moving from state i to state j
	vector<vector<HMM_PROB_TYPE> > emiss_P; //emiss_P[S][O] probability of emitting O in state S
	int state_count;

public:
	HMM(void);
	HMM(Terminal& term);
	HMM_PROB_TYPE Viterbi(Observation o, Terminal& term); //TODO: prerobit Viter,Fwd,Back do log() priestoru
	HMM_PROB_TYPE Viterbi(Observation o); //FIXME: mozno by som mal prerobit na predavanie referencii
	void Print(Terminal& term);
	void Train(vector<Observation> observations, Terminal& term);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o, Terminal& term);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o, Terminal& term);
};

HMM::HMM(void) {
	state_count = HMM_STATE_COUNT;
	const HMM_PROB_TYPE s_p = 1.0 / state_count;
	const HMM_PROB_TYPE e_p = 1.0 / O_SYMBOL_COUNT;
	vector<HMM_PROB_TYPE> temp_e;
	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)
		temp_e.push_back(e_p);

	for (int i = 0; i < state_count; i++)
		start_P.push_back(s_p);

	for (int i = 0; i < state_count; i++) {
		emiss_P.push_back(temp_e);
		//		trans_P.push_back(start_P);
	}

	for (int i = 0; i < state_count; i++) {
		temp_e.clear();
		for (int j = 0; j < state_count; j++) {
			if (j >= i)
				temp_e.push_back(1.0 / (state_count - i));
			else
				temp_e.push_back(0.0);
		}

		trans_P.push_back(temp_e);
	}

}

//HMM::HMM(vector<Observation> observations) // construct HMM from observations

void HMM::Print(Terminal& term) {
	ostringstream line;

	term.addLine("Start_P");
	line << fixed;
	for (int i = 0; i < state_count; i++) {
		line << start_P[i] << " ";
	}
	term.addLine(line.str());
	line.str("");

	term.addLine("Trans_P:");
	for (int i = 0; i < state_count; i++) {
		for (int j = 0; j < state_count; j++)
			line << trans_P[i][j] << " ";
		term.addLine(line.str());
		line.str("");
	}

	term.addLine("Emiss_P:"); //emiss_P;
	for (vector<vector<HMM_PROB_TYPE> >::const_iterator vit = emiss_P.begin(); vit
			< emiss_P.end(); vit++) {
		for (vector<HMM_PROB_TYPE>::const_iterator mit = (*vit).begin(); mit
				< (*vit).end(); mit++)
			line << (*mit) << " ";
		term.addLine(line.str());
		line.str("");
	}
}

HMM_PROB_TYPE HMM::Viterbi(Observation o, Terminal& term) {
	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;
	ostringstream line;
	line << fixed;

	term.addLine("Viterbi");
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
		line << " " << *it;
	}
	V.push_back(tmp_line);
#if VERBOSITY > 2
	term.addLine(line.str());
	term.addLine("o_at: dest_state");
#endif
	for (int observ_char = 0; observ_char < o.lenght(); observ_char++) {
		tmp_line.clear();
		line.str("");
		line << o.at(observ_char) << ":";
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			previous_max = 0;
#if VERBOSITY >2
			line << "max(";
#endif
			for (int source_state = 0; source_state < state_count; source_state++) {
				previous_max = max(previous_max, (V[observ_char][source_state] //observ_char bcs V[0] is 0 chars read
						* trans_P[source_state][dest_state]
						* emiss_P[dest_state][o.at(observ_char)])); // o.at(observ_char) bsc its char at position 0
#if VERBOSITY >2
				line << "," << V[observ_char][source_state] << "*"
						<< trans_P[source_state][dest_state] << "*"
						<< emiss_P[dest_state][o.at(observ_char)];
#endif
			}
#if VERBOSITY >2
			line << ")=";
#endif
			line << previous_max << " ";
			tmp_line.push_back(previous_max);
		}
		V.push_back(tmp_line);
		term.addLine(line.str());
	}

	previous_max = 0;
	//	find probability of best path
	for (int i = 0; i < state_count; i++) {
		previous_max = max(previous_max, V[o.lenght()][i]);

	}

	return previous_max;
}

vector<vector<HMM_PROB_TYPE> > HMM::Forward(Observation o, Terminal& term) {
	//returns F[i][j] probability of emmiting i-th symbol in state j and sequence before
	vector<vector<HMM_PROB_TYPE> > F; //F[i][j]probability of being in state j and emitting i-th symbol from o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;
	ostringstream line;
	line << fixed;
#if VERBOSITY > 1
	term.addLine("Forward");
#endif
	//fill F with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
		line << *it << " ";
	}
	F.push_back(tmp_line);
#if VERBOSITY > 1
	term.addLine(line.str());
	term.addLine("o_i:o_at states...");
#endif

	for (int observ_char = 0; observ_char < o.lenght(); observ_char++) {
		tmp_line.clear();
		line.str("");
		line << observ_char << ":";
		line << o.at(observ_char) << " ";
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			sum = 0;
#if VERBOSITY > 2
			line << "(";
#endif
			for (int source_state = 0; source_state < state_count; source_state++) {
				sum += F[observ_char][source_state]
						* trans_P[source_state][dest_state];
#if VERBOSITY > 2
				line << "+" << F[observ_char][source_state] << "*"
						<< trans_P[source_state][dest_state];
#endif
			}
			tmp_line.push_back(sum * emiss_P[dest_state][o.at(observ_char)]); // o.at(observ_char) bsc its char at position 0

#if VERBOSITY > 2
			line << ")*" << emiss_P[dest_state][o.at(observ_char)];
			line << "=";
#endif
			line << sum * emiss_P[dest_state][o.at(observ_char)] << " ";

		}
		F.push_back(tmp_line);
#if VERBOSITY > 1
		term.addLine(line.str());
#endif
	}

	return F;
}

vector<vector<HMM_PROB_TYPE> > HMM::Backward(Observation o, Terminal& term) {
	//returns B[i][j] probability of emmiting i-th symbol in state j and sequence after
	vector<vector<HMM_PROB_TYPE> > B;
	//B[i][j] probability of emmiting sequence (o.at(o.length()-i) to o.end() (+-1)) in state j
	// this gets reversed at the end
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;
	ostringstream line;

	line << fixed;
#if VERBOSITY > 1
	term.addLine("Backward");
#endif
	//fill B with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(1);
		line << 1 << " ";
	}
	B.push_back(tmp_line); //B will be filled in "reverse"
#if VERBOSITY > 1
	term.addLine(line.str());
	term.addLine("o_at: States...");
#endif

	for (vector<O_TYPE>::reverse_iterator observ_rit = o.rbegin(); observ_rit
			< o.rend(); observ_rit++) {
		tmp_line.clear();
		line.str("");
		line << *observ_rit << ":";
		for (int dest_state = state_count - 1; dest_state >= 0; dest_state--) {
			sum = 0;
			for (int source_state = 0; source_state < state_count; source_state++) {
				sum += B.back()[source_state]
						* trans_P[dest_state][source_state]
						* emiss_P[source_state][*observ_rit];
#if VERBOSITY>2
				line << "+" << B.back()[source_state] << "*"
						<< trans_P[dest_state][source_state] << "*"
						<< emiss_P[source_state][*observ_rit];
#endif
			}
			tmp_line.push_back(sum);
#if VERBOSITY>2
			line << "=";
#endif
			line << sum << " ";
		}

		B.push_back(tmp_line);
#if VERBOSITY > 1
		term.addLine(line.str());
#endif
	}
	reverse(B.begin(), B.end()); //reversed to get B in standard order B[i]
	return B;
}

void HMM::Train(vector<Observation> observations, Terminal& term) {
	//TODO: Forward-backward/baum-welch training
	vector<vector<HMM_PROB_TYPE> > e_trans_P(state_count,
			vector<HMM_PROB_TYPE> (state_count, 0)); //[state_from][state_to] expected number of transitions from training data

	vector<vector<HMM_PROB_TYPE> > e_emiss_P(state_count,
			vector<HMM_PROB_TYPE> (O_SYMBOL_COUNT, 0)); //[state][emit_symbol] expected number of emissions from training data

	vector<vector<HMM_PROB_TYPE> > F;
	vector<vector<HMM_PROB_TYPE> > B;
	ostringstream line;
	Observation o;
	int repetitions = 0;
	HMM_PROB_TYPE before = 0;
	HMM_PROB_TYPE after = 0;
	HMM_PROB_TYPE P;
	HMM_PROB_TYPE sum;

	line << fixed;
#if VERBOSITY > 0
	term.addLine("Training");
#endif

	while ((repetitions < 2) or (((after - before) > 0.001) and (repetitions
			< 30))) {
#if VERBOSITY > 1
		term.addLine("");
#endif
		repetitions++;
		before = 0;
		//for each observation J
		for (vector<Observation>::const_iterator j = observations.begin(); j
				< observations.end(); j++) {
#if VERBOSITY >3
			Print(term);
#endif
			o = *j;
			F = Forward(o, term);
			B = Backward(o, term);


#if VERBOSITY > 3
			term.addLine("b4");
			printVV(F, term);
			term.addLine("------");
			printVV(B, term);
#endif
			sum = 0;

			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.lenght() - 1][k] /** trans_P[k][0]*/;
			}

			before += P;
#if VERBOSITY > 1
			line.str("");
			line << "P:" << P << " ";
			term.addLine(line.str());
#endif

			for (int k = 0; k < state_count; k++) {
				//calculate e_trans_P[k]
				for (int l = 0; l < state_count; l++) {
					sum = 0;
					for (int i = 0; i < o.lenght(); i++) {
						sum += F[i][k] * trans_P[k][l] * emiss_P[l][o.at(i)]
								* B[i + 1][l];
					}
					e_trans_P[k][l] += (sum / P);
				}

				//calculate e_emiss_P[k]
				for (int b = 0; b < O_SYMBOL_COUNT; b++) {//iterate through all symbols
					sum = 0;
					for (int i = 0; i < o.lenght(); i++) {
						if (o.at(i) == b) {
							sum += F[i][k] * B[i][k];
						}
					}
					e_emiss_P[k][b] += (sum / P);
				}
			}
#if VERBOSITY > 3
			term.addLine("e_trans_p");
			printVV(e_trans_P, term);
#endif

		}
#if VERBOSITY > 3
		term.addLine("---Changing model parameters---");
#endif
		//new model parameters
		//fill trans_P with modified values
		for (int k = 0; k < state_count; k++) {
			for (int l = 0; l < state_count; l++) {
				sum = 0;
				for (int l2 = 0; l2 < state_count; l2++) {
					sum += e_trans_P[k][l2];
				}
				if (sum == 0) {
#if VERBOSITY > 1
					term.addLine("!!!!!!!!!!!!!!!!!!DELIS NULOU!!!!!!!!!!!!!!");
#endif
					trans_P[k][l] = 0.0;
				} else{
					trans_P[k][l] = e_trans_P[k][l] / sum;
#if VERBOSITY > 3
					line.str("");
					line << k <<":"<< l <<" "<<e_trans_P[k][l]<< "/" << sum;
					term.addLine(line.str());
#endif
				}
			}
		}

		//fill emiss_P with modified values
		for (int k = 0; k < state_count; k++) {
			for (int b = 0; b < O_SYMBOL_COUNT; b++) {
				sum = 0;
				for (int b2 = 0; b2 < O_SYMBOL_COUNT; b2++) {
					sum += e_emiss_P[k][b2];
				}
				if (sum == 0) {
#if VERBOSITY > 1
					term.addLine("!!!!!!!!!!!!!!!!!!DELIS NULOU!!!!!!!!!!!!!!");
#endif
					emiss_P[k][b] = 0.0;
				} else
					emiss_P[k][b] = e_emiss_P[k][b] / sum;
#if VERBOSITY > 3
					line.str("");
					line << k <<":"<< b <<" "<<e_emiss_P[k][b]<< "/" << sum;
					term.addLine(line.str());
#endif
			}
		}

		after = 0;
		for (vector<Observation>::const_iterator j = observations.begin(); j
				< observations.end(); j++) {
			o = *j;
			F = Forward(o, term);
			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.lenght() - 1][k] /** trans_P[k][0]*/;
			}
			after += P;
#if VERBOSITY > 3
			line.str("");
			line << "after P:" << P;
			term.addLine(line.str());
			printVV(F, term);
#endif
		}
#if VERBOSITY > 3
		Print(term);
#endif
		line.str("");
		line << "b:" << before << " a:" << after << " d:" << after - before
				<< " n:" << repetitions;
#if VERBOSITY > 0
		term.addLine(line.str());
#endif
	}
#if VERBOSITY > 2
	Print(term);
	term.addLine("end");
#endif
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

	ostringstream a;
	HMM hmm1;
	Observation o0;
	o0.putNew(1);
	o0.putNew(1);
	o0.putNew(1);

	Observation o1;
	o1.putNew(1);
	o1.putNew(1);
	o1.putNew(2);


	Observation o2;
	o2.putNew(1);
	o2.putNew(1);
	o2.putNew(2);

	Observation o3;
	o3.putNew(1);
	o3.putNew(1);
	o3.putNew(2);

	Observation o4;
	o4.putNew(2);
	o4.putNew(1);
	o4.putNew(1);

	vector<Observation> o(3, o0);
	o.push_back(o1);
	o.push_back(o2);
	o.push_back(o3);



	//	hmm1.Print(gt);
	//	hmm1.Backward(obs, gt);
	//	hmm1.Forward(obs, gt);
	//	hmm1.Viterbi(obs, gt);
	//	hmm1.Forward(o0, gt);
	//	hmm1.Forward(o1, gt);
	//	hmm1.Forward(o2, gt);
	//	vector<vector <HMM_PROB_TYPE> > F;
	//	F = hmm1.Forward(o3, gt);
	//	printVV(F,gt);
	//	HMM_PROB_TYPE P;
	//	for (int k = 0; k < hmm1.state_count; k++) {
	//					P += F[o3.lenght()-1][k] * hmm1.trans_P[k][0];
	//				}

	//	a << " o3:"<< o3.lenght()<< " P:" << P;
	//	gt.addLine(a.str());

	hmm1.Print(gt);
	a.str("");
	a << fixed << hmm1.Viterbi(o4, gt)
			<< " VITERBIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";
	gt.addLine(a.str());
	hmm1.Train(o, gt);
//	printVV(hmm1.Forward(o0, gt), gt);
	hmm1.Print(gt);

	a.str("");
	a << hmm1.Viterbi(o4, gt)
			<< " VITERBIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";
	gt.addLine(a.str());

	gt.printAll();
	//	while(1)
	//	{	gt.printAll();
	//		GRRLIB_Render();};

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
