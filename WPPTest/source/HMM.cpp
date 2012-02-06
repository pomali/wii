/*
 * HMM.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 */
#include "define.h"
#include "HMM.hpp"

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
	Print(term);
	term.addLine("end");
#endif
}
