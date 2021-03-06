/*
 * HMM.hpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */

#ifndef HMM_HPP_
#define HMM_HPP_
#include "Terminal.hpp"
#include "Observation.hpp"
#include <boost/foreach.hpp>
#include <deque>
#include <cmath>
#include <sys/types.h>
#include <algorithm>


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
	void Train0(vector<Observation> observations);
	vector<vector<HMM_PROB_TYPE> > Forward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Forward0(Observation o);
	vector<vector<HMM_PROB_TYPE> > Backward(Observation o);
	vector<vector<HMM_PROB_TYPE> > Backward0(Observation o);
	HMM_PROB_TYPE ViterbiWindow(Observation o);
	HMM_PROB_TYPE GetProb(Observation o);
	HMM_PROB_TYPE ViterbiLog(Observation o);
	vector<int> ViterbiOnlinePath(Observation o);
	HMM_PROB_TYPE ViterbiLogOnline(Observation o);
};

class nullHMM : public HMM{
public:
	void init();
	nullHMM(Terminal &terminal);

};


void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term);

#endif /* HMM_HPP_ */
