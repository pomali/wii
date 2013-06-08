/*
 * HMM.cpp minimal
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 *
 *	Chcem aby model vedel odpovedat na otazku "Ako velmi sa podoba pozorovanie na natrenovane gesta?"
 *
 *
 *	Neviem ci je sa ma brat emitovanie symbolu po prichode alebo po odchode zo stavu
 *		(a teda ci to ze zacnem v nejakom stave vyemituje pismeno alebo nie)
 *
 *	problem je ze GetProb aj Viterbi davaju lepsie vysledky pre kratsie (najkratsie) gesta/kusy giest
 *
 *
 */
#include "define.h"
#include "HMM.hpp"

/*
 * Initialize blank HMM
 *
 * je to left-right model bez prechodov na sameho seba
 * je moznost pridat specialny stav v ktorom musia vsetky koncit/ znak ktory musia na konci generovat
 *
 *
 *
 */
HMM::HMM(Terminal &terminal) : term(terminal) {
	state_count = HMM_STATE_COUNT;
	const HMM_PROB_TYPE e_p = 1.0 / O_SYMBOL_COUNT;	//default probability of emmiting symbol in state
	vector<HMM_PROB_TYPE> temp_e;

	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)		//make temp_e into default vector of emission p.
			temp_e.push_back(e_p);

	for (int i = 0; i < state_count; i++) {		//fill emission p with
		emiss_P.push_back(temp_e);
	}
	//podla HMM v Wiigee
	int jumplimit = 1;

	start_P.push_back(1.0);
	for (int i = 1; i < state_count; i++)
			start_P.push_back(0.0);

	for (int i = 0; i < state_count; i++) {
		temp_e.clear();
		for (int j = 0; j < state_count; j++) {
			if(i==state_count-1 && j==state_count-1) { // last row
				temp_e.push_back(1.0);
			} else if(i==state_count-2 && j==state_count-2) { // next to last row
				temp_e.push_back(0.5);
			} else if(i==state_count-2 && j==state_count-1) { // next to last row
				temp_e.push_back(0.5);
			} else if(i<=j && i>j-jumplimit-1) {
				temp_e.push_back(1.0/(jumplimit+1));
			} else {
				temp_e.push_back(0.0);
			}
		}
		trans_P.push_back(temp_e);
	}
}
HMM_PROB_TYPE HMM::Viterbi(Observation o) {
	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;

	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
	}
	V.push_back(tmp_line);
	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
		tmp_line.clear();
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			previous_max = -999999;
			for (int source_state = 0; source_state < state_count; source_state++) {
				previous_max = max(previous_max, (V[observ_char][source_state] //observ_char bcs V[0] is 0 chars read
						* trans_P[source_state][dest_state]
						* emiss_P[dest_state][o.at(observ_char)])); // o.at(observ_char) bsc its char at position 0
			}
			tmp_line.push_back(previous_max);
		}
		V.push_back(tmp_line);
	}

	previous_max = -99999;
	//	find probability of best path
	for (int i = 0; i < state_count; i++) {
		previous_max = max(previous_max, V[o.length()][i]);
	}
	return previous_max;
}

/*
 * Forward algorithm
 *
 * Returns: forward[Number_of_observed_symbols_from_o][State] = Probablity
 *
 * pricom forward[0][State] je pravdepodobnost ze zacneme v tom stave (teda start_P[State])
 *
 */
vector<vector<HMM_PROB_TYPE> > HMM::Forward(Observation o) {
	vector<vector<HMM_PROB_TYPE> > F; //F[i][j]probability of being in state j and emitting i-th symbol from o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;
	//fill F with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(*it);
	}
	F.push_back(tmp_line); //Pravdepodobnost ze som v emittol 0 symbolov v stave i F[0][i]

	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
		tmp_line.clear();
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			sum = 0;
			for (int source_state = 0; source_state < state_count; source_state++) {
				sum += F[observ_char][source_state]		//Predosla pravdepodobnost - pravdepodobnost ze emmitol uz observ_char znakov a je v stave source_state
						* trans_P[source_state][dest_state];	//Pravdepodobnost ze prejde do dalsieho stavu
			}
			//Pravdepodobnost ze presiel z hociktoreho stavu do dest_state a pri tom emmitol znak umiestneny v o na pozicii observ_char;
			tmp_line.push_back(sum * emiss_P[dest_state][o.at(observ_char)]); // o.at(observ_char) bsc its char at position 0
		}
		F.push_back(tmp_line);
	}

	return F;
}

/**
 * Backward algorithm
 *
 * Returns: backward[i_Number_of_observed_symbols_from_o][State] = probablity of emmiting i-th symbol in state State and sequence after
 *
 */
vector<vector<HMM_PROB_TYPE> > HMM::Backward(Observation o) {
	//B[i][j] probability of emmiting sequence (o.at(o.length()-i) to o.end() (+-1)) in state j
	//this gets reversed at the end
	vector<vector<HMM_PROB_TYPE> > B;

	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE sum;

	//fill B with starting probabilities
	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it
			< start_P.end(); it++) {
		tmp_line.push_back(1);
	}
	B.push_back(tmp_line); //B will be filled in "reverse"

	for (vector<O_TYPE>::reverse_iterator observ_rit = o.rbegin(); observ_rit
			< o.rend(); observ_rit++) {
		tmp_line.clear();
		for (int dest_state = state_count - 1; dest_state >= 0; dest_state--) {
			sum = 0;
			for (int source_state = 0; source_state < state_count; source_state++) {
				sum += B.back()[source_state]
						* trans_P[dest_state][source_state]
						* emiss_P[source_state][*observ_rit];
			}
			tmp_line.push_back(sum);
		}

		B.push_back(tmp_line);
	}
	reverse(B.begin(), B.end()); //reversed to get B in standard order B[i]
	return B;
}


/*
 * Otazka je ci som tu nieco zle nenakodil lebo vysledky co dostavam su take ... nanic;
*/

void HMM::Train(vector<Observation> observations) {
	//TODO: Forward-backward/baum-welch training
	vector<vector<HMM_PROB_TYPE> > e_trans_P(state_count,
			vector<HMM_PROB_TYPE> (state_count, 0)); //[state_from][state_to] expected number of transitions from training data

	vector<vector<HMM_PROB_TYPE> > e_emiss_P(state_count,
			vector<HMM_PROB_TYPE> (O_SYMBOL_COUNT, 0)); //[state][emit_symbol] expected number of emissions from training data

	vector<vector<HMM_PROB_TYPE> > F;
	vector<vector<HMM_PROB_TYPE> > B;
	Observation o;
	int repetitions = 0;
	HMM_PROB_TYPE before = 0;
	HMM_PROB_TYPE after = 0;
	HMM_PROB_TYPE P;
	HMM_PROB_TYPE sum;

	while ((repetitions < 2) or (((after - before) > 0.001) and (repetitions
			< 30))) {
		repetitions++;
		before = 0;
		//for each observation J
		for (vector<Observation>::const_iterator j = observations.begin(); j
				< observations.end(); j++) {
			o = *j;
			F = Forward(o);
			B = Backward(o);
			sum = 0;
			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.length() - 1][k] /** trans_P[k][0]*/;
			}
			before += P;

			for (int k = 0; k < state_count; k++) {
				//calculate e_trans_P[k]
				for (int l = 0; l < state_count; l++) {
					sum = 0;
					for (int i = 0; i < o.length(); i++) {
						sum += F[i][k] * trans_P[k][l] * emiss_P[l][o.at(i)]
								* B[i + 1][l];
					}
					e_trans_P[k][l] += (sum + r_trans / P);
				}

				//calculate e_emiss_P[k]
				for (int b = 0; b < O_SYMBOL_COUNT; b++) {//iterate through all symbols
					sum = 0;
					for (int i = 0; i < o.length(); i++) {
						if (o.at(i) == b) {
							sum += F[i][k] * B[i][k];
						}
					}
					e_emiss_P[k][b] += (sum + r_emiss / P);
				}
			}
		}
		//new model parameters
		//fill trans_P with modified values
		for (int k = 0; k < state_count; k++) {
			for (int l = 0; l < state_count; l++) {
				sum = 0;
				for (int l2 = 0; l2 < state_count; l2++) {
					sum += e_trans_P[k][l2];
				}
				if (sum == 0) {
					trans_P[k][l] = 0.0;
				} else {
					trans_P[k][l] = e_trans_P[k][l] / sum;
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
					emiss_P[k][b] = 0.0;
				} else
					emiss_P[k][b] = e_emiss_P[k][b] / sum;
			}
		}

		after = 0;
		for (vector<Observation>::const_iterator j = observations.begin(); j
				< observations.end(); j++) {
			o = *j;
			F = Forward(o);
			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.length() - 1][k] /** trans_P[k][0]*/;
			}
			after += P;
		}
	}
}

/*
 * Kedze pouzivam noobsky jedno HMM pre jedno gesto
 *  riesim Problem 1 - pravdepodobnost ze HMM patri ku pozorovane gesto
 *
 *
 *  Pravdepodobnost ze nastalo gesto na ktore sa pytam sa rovna
 *  suctu pravdepodobnosti vsetkych moznych ciest ktore vygenerovali to gesto
 */
HMM_PROB_TYPE HMM::GetProb(Observation o){
	HMM_PROB_TYPE out = 0.0;
	vector<vector<HMM_PROB_TYPE> > forward = this->Forward(o);
	//	add probabilities
	for (uint i = 0; i < forward.size(); i++) { // for every state
		out += forward[o.length()][i];
	}
	return out;
}
