/*
 * main.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: pom
 *
 *    na to aby sme rozpoznali gesta tak musime mat HMM
 *    na to aby sme mali HMM musime ho mat zapamatane alebo ho vytrenovat
 *    HMM trenujeme tak ze mu predame vector<Observation>
 *    Device class sa initne a potom pomocou ::get() vlozi do Observation nove data
 *
 *    Terminal je iba pomocna classa ktora vypisuje (GRRTerminal pouziva GRRLib, STDTerminal pouziva stdout)
 *    pred pouzitim terminalu treba initnut grrlib
 *
 */

#include "define.h"


#ifdef FORWII
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include "FreeMonoBold_ttf.h"
#endif

//#include <stdlib.h>
#include <vector>
#include <sstream>

#include "Terminal.hpp"
#include "Observation.hpp"
#include "HMM.hpp"
#include "WiimoteTest.h"

using namespace std;

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

	WiimoteTest wmt;
	wmt.main(argc,argv);

	STDTerminal gt;

	ostringstream a;
	HMM hmm1(gt);
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

	hmm1.Backward(o0);
	hmm1.Forward(o0);
	hmm1.Print();
	hmm1.Train(o);
	hmm1.Print();

#ifdef FORWII
	while(1) {
		gt.printAll();
		VIDEO_WaitVSync();
	}

#else
	cout << "treningove data:" << endl;
	for (vector<Observation>::iterator it = o.begin(); it < o.end(); it++){
		cout << (*it).printOut() << endl;
	}

	Observation o5;
	int cislo;
	cout << fixed;
	while (1) {
		cout << "Zadaj int <0," << O_SYMBOL_COUNT << ")" << endl;
		cin >> dec >> cislo;
//		cout << cislo << endl;
//		return 3;
		o5.putNew(cislo);
//		cout << hmm1.Viterbi(o5) << endl;
		cout << hmm1.ViterbiWindow(o5) << endl;
	}
#endif

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
