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


#include "HMM.hpp"

void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term) {
	int c = 0;
	cout << "   ";
	BOOST_FOREACH(HMM_PROB_TYPE inside, V.front()){
		cout << c++ << ": ";
	}
	cout << endl;
	c=0;
	BOOST_FOREACH(vector<HMM_PROB_TYPE> inside_vector, V){
		cout << c++ << ": ";
		BOOST_FOREACH(HMM_PROB_TYPE l, inside_vector){
			cout<<l<<" ";
		}
		cout<<endl;
	}
}

void HMM::init(){
	gesture_state_count =  HMM_GESTURE_STATE_COUNT;
	noise_state_count = HMM_NOISE_STATE_COUNT;
	state_count =gesture_state_count+noise_state_count+2; //cast pre noise, cast pre gesto, start, koniec

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

			for (int source_state = 0; source_state < state_count; source_state++){
				//observ_char bcs V[0] is 0 chars read
				previous_max = max(previous_max, (V[observ_char][source_state] + log(trans_P[source_state][dest_state])));
			}
			tmp_line.push_back(previous_max + log(emiss_P[dest_state][o.at(observ_char)]));
		}
		V.push_back(tmp_line);
	}
	previous_max = -9999;
	//	find probability of best path
	for (int i = 0; i < state_count; i++) //state_count
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
vector<vector<HMM_PROB_TYPE> > HMM::Forward0(Observation o) {
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



vector<vector<HMM_PROB_TYPE> > HMM::Forward(Observation o) {
	//returns F[iTime][jState] probability of emmiting i-th symbol in state j and sequence before
	vector<vector<HMM_PROB_TYPE> > f(o.length()+1,vector<HMM_PROB_TYPE>(state_count,0)); //F[i][j]probability of being in state j and emitting i-th symbol from o
	vector<HMM_PROB_TYPE> tmp_line;

	//fill f with starting probabilities
	for (int l = 0; l< state_count; l++){
		HMM_PROB_TYPE sap = start_P.at(l);
		O_TYPE sao = o.at(0);
		HMM_PROB_TYPE eap = emiss_P.at(l).at(sao);
		f[0][l] = ( sap * eap);
	}
	for (int i = 1; i < o.length(); i++){//symbol / time
		for (uint k = 0; k < f[0].size();k++){//from state
			HMM_PROB_TYPE sum = 0;
			for (int j = 0; j < state_count; j++){ //to state
				sum += f[i-1][j] * trans_P[j][k];
			}
			f[i][k] = sum * emiss_P[k][o[i]];
		}
	}
	return f;
}


vector<vector<HMM_PROB_TYPE> > HMM::Backward0(Observation o) {
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

vector<vector<HMM_PROB_TYPE> > HMM::Backward(Observation o){
	//returns B[iTime][jState] probability of emmiting i-th symbol in state j and sequence after
	const int T = (int)o.length(); //duration of gesture
	vector<vector<HMM_PROB_TYPE> > b(T,vector<HMM_PROB_TYPE>(state_count, 0));

	for (int i = 0; i < state_count; i++)
		b[T-1][i] = 1;

	for (int t = T-2; t >=0; t--){
		for (int i = 0; i < state_count; i++){
			b[t][i] = 0;
			for (int j = 0; j < state_count; j++){
				b[t][i] += (b[t+1][j] * trans_P[i][j] * emiss_P[j][o[t+1]]);
			}
		}
	}
	return b;
}


/*
 * Otazka je ci som tu nieco zle nenakodil lebo vysledky co dostavam su take ... nanic;
*/

void HMM::Train0(vector<Observation> observations) {
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

void HMM::Train(vector<Observation> observations){

	vector<vector<HMM_PROB_TYPE> > a_new(this->trans_P.size(), vector<HMM_PROB_TYPE> (this->trans_P.at(0).size(), 0)); //trans_P[i][j] probability of moving from state i to state j
	vector<vector<HMM_PROB_TYPE> > b_new(this->emiss_P.size(), vector<HMM_PROB_TYPE> (this->emiss_P.at(0).size(), 0)); //emiss_P[S][O] probability of emitting O in state S

	for (uint i=0; i<a_new.size(); i++){
		for (uint j=0; j<a_new[0].size();j++){
			HMM_PROB_TYPE numerator=0;
			HMM_PROB_TYPE denominator=0;

			BOOST_FOREACH(Observation seq, observations){
				vector<vector<HMM_PROB_TYPE> > fwd = this->Forward(seq);
				vector<vector<HMM_PROB_TYPE> > bwd = this->Backward(seq);
				HMM_PROB_TYPE prob = this->GetProb(seq) + 1e-200;

				HMM_PROB_TYPE numer_innersum = 0;
				HMM_PROB_TYPE denom_innersum = 0;

				for (int t=0; t<seq.length()-1; t++){
					numer_innersum += fwd[t][i]*trans_P[i][j]*emiss_P[j][seq.at(t+1)]*bwd[t+1][j];
					denom_innersum += fwd[t][i]*bwd[t][i];
				}
				numerator   += (1/prob)*numer_innersum;
				denominator += (1/prob)*denom_innersum;

			}
			a_new[i][j] = numerator/denominator;
		}
	}

	for (uint i=0; i<b_new.size(); i++){
		for (uint j=0; j<b_new[i].size(); j++){
			HMM_PROB_TYPE numerator=0;
			HMM_PROB_TYPE denominator=0;

			BOOST_FOREACH(Observation seq, observations){
				vector<vector<HMM_PROB_TYPE> > fwd = this->Forward(seq);
				vector<vector<HMM_PROB_TYPE> > bwd = this->Backward(seq);
				HMM_PROB_TYPE prob = this->GetProb(seq) + 1e-200;

				HMM_PROB_TYPE numer_innersum = 0;
				HMM_PROB_TYPE denom_innersum = 0;

				for(int t=0;t<seq.length()-1; t++){
					if(seq.at(t)==(int)j){
						numer_innersum+=fwd[t][i]*bwd[t][i];
					}
					denom_innersum+=fwd[t][i]*bwd[t][i];
				}
				numerator 	+= 	(1/prob)*numer_innersum;
				denominator	+=	(1/prob)*denom_innersum;
			}

			b_new[i][j] = numerator/denominator;
		}
	}

	this->trans_P = a_new;
	this->emiss_P = b_new;


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
//	int j;
//	cout << "forward:" <<endl;
//	printVV(forward,term);

	//	add probabilities
	for (uint i = 0; i < forward.at(0).size(); i++) { // for every state
//		for (j = 0; j <= o.length(); j++)
			out += forward[forward.size()-1][i];
	}
	return out;
}

vector<int> HMM::ViterbiOnlinePath(Observation o){
//podla diplomovky R. Srameka "On-line Viterbi Algorithm"
//	nedokoncene

	//Viterbi normal
//	Model M;
	Observation X;
	int n=o.length();
	int m=state_count;
	/*
	 * v_gamma[i][j] , i:miesto (cas) v pozorovani, j:stav
	 * maximalna pravdepodobnost ze state path skonci v stave j s pouzitim (predposlednym stavom?) gamma[i-1]
	 * takze gamma[i][j] zalezi (pouziva) presne od jednej hodnoty z gamma[i-1]
	 * po precitani i znakov z pozornovania (??)
	 */
	vector< vector <HMM_PROB_TYPE> > v_gamma (n, vector <HMM_PROB_TYPE> (m,0));

	/*
	 * delta[i][j], i:miesto (cas) v pozorovani, j:stav
	 * stav z ktoreho pochadza hodnota v v_gamma[i][j]; teda mapa spatnych smernikov
	 * delta[i][j] = stav (cislo stavu) z ktoreho si sa dostal po precitani prvych i znakov z Observation a po nom si siel na stav j
	 * teda predposledny stav cez ktory prechadzal state path a konci v stave j
	 */
	vector< vector<int> >delta (m, vector<int> ());

	/*
	 * s[n]
	 *
	 *
	 */
	vector<int> s;

	for (int i=1;i<=m;i++) {
		//Initialization
		v_gamma[0][i] = start_P[i]; //= πj; (co je toto?) pravdepodobnost ze emitujem znak j??
		delta[0][i] = 0;
	}


	HMM_PROB_TYPE curr_val, max_val;
	int arg_max;

//m je pocet stavov ?? ; n je observation length ???
	for (int j=1; j<=n; j++) { //prechadzam postupne cez vsetky pozorovane znaky; cez pozorovanu postupnost
		//First phase
		for (int i=1; i<=m;i++){ // pre kazdy stav vypocitam pravdepodobnost ze som sa tam dostal (s emitovanim/videnim znaku o.at(j)) a to odkial som sa tam dostal
			//Recurrence
			for(int k=1;k<=m;k++){ //max + argmax
				curr_val = (v_gamma[j-1][k] * trans_P[k][i] * emiss_P[i][o.at(j)]); //o.at(j) = X_j je videny/emitovany znak?? ; t je pravdepodobnost prechodu
				if (curr_val>=max_val || k==1){
					arg_max = k;
					max_val = curr_val;
				}
			}
			v_gamma[j][i] = max_val;
			delta[j][i] = arg_max;
		}
	}

	for(int k=1;k<=n;k++){
		curr_val = v_gamma[n][k];
		if (curr_val>=max_val || k==1){
			max_val = curr_val;
			arg_max=k;
		}
	}
	 s[n] = arg_max; //stav v ktorom skoncim v poslednom kroku najpravdepodobnejsieho state pathu
	for(int i=n-1;i<=1;i++){ //Second phase (back-tracing)
		s[i] = delta[i+1][s[i+1]];
	}

//	reverse(s.begin(), s.end());
	return s; //vracia poradie najpravdepodobnejsieho postupu
	//Viterbi normal end
}


HMM_PROB_TYPE HMM::ViterbiLogOnline(Observation o) {
	/*
	 * slo by prerobit trans_P na adjacency list (pre kazdy node, list nodov s ktorymi susedi a s akou pravdepodobnostou)
	 */
	vector<vector<HMM_PROB_TYPE> > V;//V[i][j] probability of most probable path ending in state j at i-th symbol of Observation o
	vector<HMM_PROB_TYPE> tmp_line;
	HMM_PROB_TYPE max_val = 0;
	int max_arg;
	HMM_PROB_TYPE current_val;
	vector<vector<int> > delta; // delta[i][j] 'predposledny stav', stav z ktoreho som dosiel na j po precitani i znakov z O

	for (vector<HMM_PROB_TYPE>::const_iterator it = start_P.begin(); it	< start_P.end(); it++)
		tmp_line.push_back(log(*it));
	V.push_back(tmp_line);

	for (int observ_char = 0; observ_char < o.length(); observ_char++) {
		tmp_line.clear();
		delta.push_back(vector<int> () );
		for (int dest_state = 0; dest_state < state_count; dest_state++) {
			for (int source_state = 0; source_state < state_count; source_state++){
				current_val = (V[observ_char][source_state] + log(trans_P[source_state][dest_state]));
				if (max_val<= current_val || source_state==0) { // teda v prvom prechode urcite ano
					max_val = current_val;
					max_arg = source_state;
				}
			}
			delta[observ_char].push_back(max_arg);
			tmp_line.push_back(max_val + log(emiss_P[dest_state][o.at(observ_char)]));
		}
		V.push_back(tmp_line);
	}  //XXX: ak by sme chceli robit 'prirastkove' pridavanie znakov tu by sa dopocital iba jeden dalsi znak (?)

	//	find probability of best path
	for (int i = 0; i < state_count; i++){ //state_count
		current_val = V[o.length()][i];
		if (max_val <= current_val || i==0){
			max_val =current_val;
			max_arg = i;
		}
	} //XXX: ak by sme chceli robit 'prirastkove' pridavanie znakov toto by trebalo znova

	cout<< "maxarg: "<< max_arg << " maxval: " << max_val << endl << "delta:" << endl;

	BOOST_FOREACH(vector<int> v_delta, delta){
		BOOST_FOREACH(int i_delta, v_delta){
			cout << i_delta << " ";
		}
		cout << endl;
	}

	deque<int> best_path;
	best_path.push_front(max_arg); // najdi najpravdepodobnejsiu cestu odzadu, potom ju reverzni na prirodzeny smer
	BOOST_REVERSE_FOREACH(vector<int> v_delta, delta){
		best_path.push_front(v_delta.at(best_path.front()));
	}
	cout << "bestpath:" << endl;
	BOOST_FOREACH(int i_path, best_path){
		cout << i_path << " ";
	}
	cout<<endl;



	return exp(max_val);
}

void nullHMM::init(){
	state_count = 1;
	const HMM_PROB_TYPE e_p = 1.0 / O_SYMBOL_COUNT;	//default probability of emmiting symbol in state
	vector<HMM_PROB_TYPE> temp_e;

	for (O_TYPE j = 0; j < O_SYMBOL_COUNT; j++)		//make temp_e into default vector of emission p.
			temp_e.push_back(e_p);
	emiss_P.clear();
	for (int i = 0; i < state_count; i++) {		//fill emission p with
		emiss_P.push_back(temp_e);
	}

	start_P.clear();
	start_P.push_back(1.0);

	trans_P.clear();
	trans_P.push_back(vector<HMM_PROB_TYPE>(1,0));
}

nullHMM::nullHMM(Terminal &terminal) : HMM (terminal) {
	this->init();
}



