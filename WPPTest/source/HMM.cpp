/*
 * HMM.cpp
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
#include <cmath>


void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term) {
	ostringstream line;
	line << scientific;
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


void HMM::init(){
	state_count = HMM_STATE_COUNT;
	//const HMM_PROB_TYPE s_p = 1.0 / state_count;	//default probability of being in state
	const HMM_PROB_TYPE e_p = 1.0 / O_SYMBOL_COUNT;	//default probability of emmiting symbol in state
	vector<HMM_PROB_TYPE> temp_e;


	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)		//make temp_e into default vector of emission p.
			temp_e.push_back(e_p);

	emiss_P.clear();
	for (int i = 0; i < state_count; i++) {		//fill emission p with
		emiss_P.push_back(temp_e);
	}


/*
	for (int i = 0; i < state_count; i++)
			start_P.push_back(s_p);

	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)		//make temp_e into default vector of emission p.
		temp_e.push_back(e_p);

	for (int i = 0; i < state_count; i++) {		//fill emission p with
		emiss_P.push_back(temp_e);
		//		trans_P.push_back(start_P);
	}

	for (int i = 0; i < state_count; i++) {
		temp_e.clear();
		for (int j = 0; j < state_count; j++) {
			if ((i < j) && (j<=i+2))
				temp_e.push_back(1.0 / 2); //(state_count - i));
			else
				temp_e.push_back(0.0);
		}
		trans_P.push_back(temp_e);
	}
*/



	/* podla HMM v Wiigee
	 *
	 * (self-loop, next, jump next)
	 * 	  _   _
	 *  ^/ \^/ \^...
	 *  O-->O-->...
	 *
	 */
	/*
	int jumplimit = 2;

	start_P.clear();
	start_P.push_back(1.0);
	for (int i = 1; i < state_count; i++)
			start_P.push_back(0.0);

	trans_P.clear();
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
	*/


	int jumplimit = 1;

		start_P.clear();
		start_P.push_back(1.0);
		for (int i = 1; i < state_count; i++)
				start_P.push_back(0.0);

		trans_P.clear();
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
this->init();

}

//HMM::HMM(vector<Observation> observations) // construct HMM from observations

void HMM::Print() {
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

	line.str("");
	line << state_count;
	term.addLine(line.str());
}

HMM_PROB_TYPE HMM::Viterbi(Observation o) {
	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;
	ostringstream line;
	line << scientific;

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
	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
		tmp_line.clear();
		line.str("");
		line << o.at(observ_char) << ":";
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			previous_max = -99999;
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

	previous_max = -9999;
	//	find probability of best path
	for (int i = 0; i < state_count; i++) {
		previous_max = max(previous_max, V[o.length()][i]);

	}

	return previous_max;
}


HMM_PROB_TYPE HMM::ViterbiLog(Observation o) {
	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;

	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it	< start_P.end(); it++)
		tmp_line.push_back(log(*it));
	V.push_back(tmp_line);

	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
		tmp_line.clear();
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			previous_max = -99999;

			for (int source_state = 0; source_state < state_count; source_state++)
				//observ_char bcs V[0] is 0 chars read
				previous_max = max(previous_max, (V[observ_char][source_state] + log(trans_P[source_state][dest_state])));

			tmp_line.push_back(previous_max + log(emiss_P[dest_state][o.at(observ_char)]));
		}
		V.push_back(tmp_line);
	}
	previous_max = -9999;
	//	find probability of best path
	for (int i = 0; i < state_count; i++)
		previous_max = max(previous_max, V[o.length()][i]);

	return exp(previous_max);
}

HMM_PROB_TYPE HMM::ViterbiWindow(Observation o){// Same as Viterbi but only on last WINDOW_SIZE
	vector<vector<HMM_PROB_TYPE> > V;			//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE previous_max;
	ostringstream line;
	line << fixed;

	term.addLine("ViterbiWindow");
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
	for (int observ_char = max(0,o.length()-WINDOW_SIZE); observ_char < o.length(); observ_char++) {
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
	F.push_back(tmp_line); //Pravdepodobnost ze som v emittol 0 symbolov v stave i F[0][i]
#if VERBOSITY > 1
	term.addLine(line.str());
	term.addLine("o_i:o_at states...");
#endif

	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
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
				sum += F[observ_char][source_state]		//Predosla pravdepodobnost - pravdepodobnost ze emmitol uz observ_char znakov a je v stave source_state
						* trans_P[source_state][dest_state];	//Pravdepodobnost ze prejde do dalsieho stavu
#if VERBOSITY > 2
				line << "+" << F[observ_char][source_state] << "*"
				<< trans_P[source_state][dest_state];
#endif
			}
			//Pravdepodobnost ze presiel z hociktoreho stavu do dest_state a pri tom emmitol znak umiestneny v o na pozicii observ_char;
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

vector<vector<HMM_PROB_TYPE> > HMM::Backward(Observation o) {
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

	while ((repetitions < 3) or (((after - before) > 0.00001) and (repetitions
			< 80))) {
#if VERBOSITY > 1
		term.addLine("");
#endif
		repetitions++;
		before = 0;
		//for each observation J
		for (vector<Observation>::const_iterator j = observations.begin(); j
				< observations.end(); j++) {
#if VERBOSITY >3
			Print();
#endif
			o = *j;
			F = Forward(o);
			B = Backward(o);

#if VERBOSITY > 3
			term.addLine("b4");
			printVV(F, term);
			term.addLine("------");
			printVV(B, term);
#endif
			sum = 0;

			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.length() - 1][k] /** trans_P[k][0]*/;
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
				} else {
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
			F = Forward(o);
			P = 0;
			for (int k = 0; k < state_count; k++) {
				P += F[o.length() - 1][k] /** trans_P[k][0]*/;
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
		Print();
#endif
		line.str("");
		line << "before:" << before << " after:" << after << " delta:" << after
				- before << "repetiton:" << repetitions;
#if VERBOSITY > 0
		term.addLine(line.str());
#endif
	}
#if VERBOSITY > 1
	line.str("");
	line << "repetitons:" << repetitions;
	term.addLine(line.str());
#endif

#if VERBOSITY > 2
	Print();
	term.addLine("end");
#endif
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
	int j;
	printVV(forward,term);

	//	add probabilities
	for (uint i = 0; i < forward.size(); i++) { // for every state
		j=o.length();//for (j = 0; j < o.length(); j++)
			out += log(forward[j][i]);
	}
	return out;
}
