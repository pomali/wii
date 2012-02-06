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

#endif /* HMM_HPP_ */
