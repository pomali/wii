/*
 * Hmm2.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: pom
 */

#include "Hmm2.h"

using namespace boost::numeric::ublas;

Hmm2::Hmm2() {
	state_gesture_count = 1;
	state_noise_count = 1;
	state_total_count = state_noise_count+state_gesture_count+2;
	symbols_count = 6;

	this->init();
}

Hmm2::~Hmm2() {
}

bool Hmm2::init(){
	using namespace boost::numeric::ublas;
	// matrix (rows, cols)
	e = matrix<double>(state_total_count, symbols_count);	//in_state, symbol
	a = matrix<double>(state_total_count,state_total_count); //from, to
	end_state = state_total_count-1;
	s = std::set<int>();
	s.insert(START_STATE); //0 je zaciatocny stav ktory je silent
	s.insert(end_state); //posledny stav je silent


	//occasionally dishonest casino example

    e.clear();
	unsigned j;
    for (j = 0; j < e.size2(); ++ j){
            e(1, j) = (double)1/6;
    }
    for (j = 0; j < e.size2(); ++ j){
                e(2, j) = (double)1/10;
    }
    e(2,5) = (double) 1/2;

    a.clear();
    a(0,1) = 0.5;
    a(0,2) = 0.5;
    a(1,1) = 0.85;
    a(1,2) = 0.05;
    a(2,1) = 0.1;
    a(2,2) = 0.8;

    a(1,end_state) = 0.1;
    a(2,end_state) = 0.1;

	return true;
}

int Hmm2::test(){
	using namespace boost::numeric::ublas;

    this->init();
    int pv[60] = {2, 2, 2, 5, 5, 5, 4, 4, 1, 6, 6, 6, 5, 6, 6, 5, 6, 3, 5, 6, 4, 3, 2, 4, 3, 6, 4, 1, 3, 1, 5, 1, 3, 4, 6, 5, 1, 4, 6, 3, 5, 3, 4, 1, 1, 1, 2, 6, 4, 1, 4, 6, 2, 6, 2, 5, 3, 3, 5, 6};
    //{6, 5, 1, 1, 6, 6, 4, 5, 3, 1, 3, 2, 6, 5, 1, 2, 4, 5, 6, 3, 6, 6, 6, 4, 6, 3, 1, 6, 3, 6, 6, 6, 3, 1, 6, 2, 3, 2, 6, 4, 5, 5, 2, 3, 6, 2, 6, 6, 6, 6, 6, 6, 2, 5, 1, 5, 1, 6, 3, 1};
    //{3,1,5,1,1,6,2,4,6,4,4,6,6,4,4,2,4,5,3,1,1,3,2,1,6,3,1,1,6,4,1,5,2,1,3,3,6,2,5,1,4,4,5,4,3,6,3,1,6,5,6,6,2,6,5,6,6,6,6,6};

    std::vector<int> seq;
    for (unsigned i=0; i<60; i++){
    	seq.push_back(pv[i]-1);
    }


    this->Viterbi(seq);
    this->Forward(seq);
    this->Backward(seq);

    std::cout<<"papa"<<std::endl;

    return 0;
}


matrix<double> Hmm2::Viterbi(std::vector<int> sequence){
 matrix<double> v = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
 matrix<double> ptr = matrix<double>(sequence.size()+1, state_total_count);
 for(unsigned i =0; i<v.size1(); i++){
	 for(unsigned j=0; j<v.size2();j++){
		 v(i,j) = -INFINITY;
	 }
 }

 v(START_STATE,0) = log(1);

 for(unsigned i=0; i<sequence.size(); i++){

	 unsigned argmax = 0;
	 for(int l=0; l<state_total_count; l++){
		 //BEGIN max+argmax
		 matrix_column< matrix<double> > c_v = column(v, i);
		 matrix_column< matrix<double> > c_a = column(a, l);
		 double max = -INFINITY;
		 for (int k=0; k<state_total_count;k++){
			 double val = c_v(k) + log(c_a(k));
			 if (val>max){
				 max = val;
				 argmax = k;
			 }
		 }
		 //END max+argmax

		 v(l,i+1) = log(e(l,sequence.at(i))) + max;
		 ptr(i+1,l) = argmax;
	 }
 }

 //chcem vyplnit "posledny" stlpec V - teda to z kadial sa da dostat do koncoveho stavu s najvacsiou pravdepodobnostou

 matrix_column< matrix<double> > c_v = column(v, sequence.size());
 matrix_column< matrix<double> > c_a = column(a, end_state);

 double path_prob = -INFINITY;
 unsigned last_state = 0;
 for (int k=0; k<state_total_count;k++){
		 double val = c_v(k)+log(c_a(k));
		 if (val>path_prob){
			 path_prob = val;
			 last_state = k;
		 }
	 }

//dopocitam path kadial isla najpravdepodobnejsia cesta
 vector<int> path= vector<int>(sequence.size()+1);
 vector<bool> path_labels= vector<bool>(sequence.size());
 path.clear();
 path_labels.clear();
 path(sequence.size())=last_state;
 for (int i=sequence.size(); i>0;i--){
	 path(i-1) = ptr(i,path(i));
	 path_labels(i-1) = is_in_gesture(path(i));
 }

//std::cout<<"v:"<< v << std::endl;
std::cout<<"path_prob:"<< exp(path_prob)<<std::endl;
//std::cout<<"path:"<< path << std::endl;
std::cout<<"path_labels:"<< path_labels << std::endl;
//std::cout<<"ptr:"<< ptr << std::endl;

 return v;
}



boost::numeric::ublas::matrix<double> Hmm2::Forward(std::vector<int> sequence){
	matrix<double> f = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
	 for(unsigned i =0; i<f.size1(); i++){
		 for(unsigned j=0; j<f.size2();j++){
			 f(i,j) = -INFINITY;
		 }
	 }
	 f(START_STATE,0) = log(1);

	 for(unsigned i=0; i<sequence.size(); i++){

		 for(int l=1; l<state_total_count; l++){
			 matrix_column< matrix<double> > c_f = column(f, i);
			 matrix_column< matrix<double> > c_a = column(a, l);
			 vector<double> sum_a(state_total_count);
			 sum_a.clear();
			 //BEGIN max+argmax+pre_sum
			 double max = -INFINITY;
			 int argmax;
			 for (int k=0; k<state_total_count;k++){
				 double val = (c_f(k)) + log(c_a(k)); //LOG
				 if (val>max){
					 max = val;
					 argmax = k;
				 }
				 sum_a(k) = val;
			 }

			 //END max+argmax+pre_sum
			 double sum = 0;
			 for (int k=0; k<state_total_count; k++){
				 double val =sum_a(k) - max; //shiftneme vsetky body tak aby max bol na nule
				 if (val==val) //ak NIEJE pravda ze val == +-NAN ak robim rozdiel dvoch -nekonecien tak to zahodim, preco? netusim, aby to nerobilo bordel, ale v principe su to dve nuly v nelog priestore
					 sum += exp(val);
			 }
			 sum = log(sum) +max; // +max



			 f(l,i+1) = log(e(l, sequence.at(i))) + sum;
		 }
	 }

	 matrix_column< matrix<double> > c_f = column(f, sequence.size());
	 matrix_row< matrix<double> > r_a = row(a, 0);
	 vector<double> sum_a(state_total_count);
	 sum_a.clear();
	 //BEGIN max+argmax+pre_sum
	 double max = -INFINITY;
	 int argmax;
	 for (int k=0; k<state_total_count;k++){
		 double val = c_f(k) + log(r_a(k));
		 if (val>max){
			 max = val;
			 argmax = k;
		 }
		 sum_a(k) = val;
	 }
	 //END max+argmax+pre_sum
	 double sum = 0;
	 for (int k=0; k<state_total_count; k++){
			 sum += exp( sum_a(k)-max ); //shiftneme vsetky body tak aby max bol na nule
	 }

	 double prob = exp(max + log(sum));

	 std::cout<<"prob:"<<prob<<std::endl;
	 std::cout<<"f:"<<f<<std::endl;

	 return f;
}


boost::numeric::ublas::matrix<double> Hmm2::Backward(std::vector<int> sequence){
/*
 * Initialization
 */
	matrix<double> b = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
	 for(unsigned k =0; k<b.size1(); k++){
		 for(unsigned j=0; j<b.size2();j++){
			 if (j==sequence.size()){
				b(k,j) = log(a(k,end_state));
			 }
			 else
				 b(k,j) = -INFINITY;
		 }
	 }


	 std::cout<<"b:"<<b<<std::endl;
/*
 * Recursion
 */

	 for(unsigned i=sequence.size(); i>0; i--){
		 for(int k=0; k<state_total_count; k++){
			 double max_val=-INFINITY;
			 vector<double> sum_a(state_total_count);
			 for(int l=0; l<state_total_count; l++){
				 double val= log(a(k,l)) + log(e(l,sequence.at(i-1))) + b(l,i);
				 if(val>max_val)
					 max_val = val;
				 sum_a(l) = val;
			 }

			 double sum=0;
			 for(int l=0; l<state_total_count; l++){
				 double val = (sum_a(k) - max_val);
				 if (val==val)
					 sum+=exp(val);
			 }
			 sum = log(sum) + max_val;

			 b(k,i-1) = sum;
		 }
	 }
/*
 * Termination
 */

	 vector<double> sum_a(state_total_count);
	 double max_val=-INFINITY;
	 for(int l=0;l<state_total_count;l++){
		 double val = log(a(0,l)) + log(e(l, sequence.at(0))) + b(l, 1);
		 if(val>max_val)
			 max_val=val;
		 sum_a(l)=val;
	 }

	 double sum=0;
	 for(int l=0;l<state_total_count;l++){
		 double val = sum_a(l) - max_val;
		 sum += exp(val);
	 }
	 sum = max_val + log(sum);
	 double prob = exp(sum);

	 std::cout<<"b_prob:"<<prob<<std::endl;
	 std::cout<<"b:"<<b<<std::endl;

	 return b;
}


bool Hmm2::is_in_gesture(int state){
	return ( 0 < state and  state < state_gesture_count+1);
}
