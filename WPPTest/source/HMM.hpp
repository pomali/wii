/*
 * HMM.hpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */

#ifndef HMM_HPP_
#define HMM_HPP_
#include "define.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include "Observation.hpp"
#include "Terminal.hpp"


using namespace std;

class HMM {
public:
	//private
	vector<HMM_PROB_TYPE > start_P; // start_P[S] probability of starting in state S
	vector<vector<HMM_PROB_TYPE> > trans_P; //trans_P[i][j] probability of moving from state i to state j
	//TODO: prerobit trans_P na map
	vector<vector<HMM_PROB_TYPE> > emiss_P; //emiss_P[S][O] probability of emitting O in state S
	int state_count;
	Terminal &term;

public:
	//HMM(); //TODO: prerobit Viter,Fwd,Back do log() priestoru
	HMM(Terminal &terminal);
	void init();
	HMM_PROB_TYPE Viterbi(Observation o); //FIXME: mozno by som mal prerobit na predavanie referencii
	void Print(int verbosity);
	void Print();
	void Train(vector<Observation> observations);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o);
	HMM_PROB_TYPE ViterbiWindow(Observation o);
	HMM_PROB_TYPE GetProb(Observation o);
};

void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term);

#endif /* HMM_HPP_ */
