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
	posterior_probability=0;

	this->init();
}

Hmm2::~Hmm2() {
}


/*
 * log sum exp trick ... input: log_a ( a v log priestore) log_b (b v log priestore) ; returns log(a+b) = log(a) + log(1 + exp(log(b)-log(a)))
 */

double eexp(double log_x){
	if (log_x==-INFINITY)
		return 0;
	else
		return exp(log_x);
}

double elog(double x){
	if (x==0)
		return -INFINITY;
	else if (x>0)
		return log(x);
	else
		throw 20;
}

double elogsum(double log_a, double log_b){
//	if(log_a>0 or log_b>0)
//			std::cout<<"sum log_a: "<<log_a<<" log_b: "<<log_b<<std::endl;
	if (log_a==-INFINITY)
		return log_b;
	else if(log_b==-INFINITY)
		return log_a;
	else{
		if (log_a>log_b)
				return log_a + log1p(eexp(log_b-log_a));
			else
				return log_b + log1p(eexp(log_a-log_b));
	}
}

double elogproduct(double log_a, double log_b){
//	if(log_a>0 or log_b>0)
//		std::cout<<"prod log_a: "<<log_a<<" log_b: "<<log_b<<std::endl;
	if (log_a ==-INFINITY or log_b==-INFINITY)
		return -INFINITY;
	else
		return log_a+log_b;
}

double elogdiv(double log_a, double log_b){
	if (log_a ==-INFINITY or log_b==-INFINITY)
		return -INFINITY;
	return log_a-log_b;
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

	this->init_known_casino();

	return true;
}

void Hmm2::init_known_casino(){
	//occasionally dishonest casino example
    e.clear();
    for(unsigned i=0; i<e.size1();i++){
    	if(this->get_state_label(i)){
			for (unsigned j = 0; j < e.size2(); ++ j){
					e(i, j) = ((double)1/symbols_count)-0.0001;
			}
    	}
    	else if(i!=(unsigned)START_STATE and i!=(unsigned)end_state){
			for (unsigned j = 0; j < e.size2(); ++ j){
						e(i, j) = (double)1/10;
			}
    	}
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
}


void Hmm2::init_unknown_casino(){
	//occasionally dishonest casino example
    e.clear();
    for(unsigned i=0; i<e.size1();i++){
		for (unsigned j = 0; j < e.size2(); ++ j){
			if(!this->issilent(i)) //ak sa i nenachadza v silent state
				e(i, j) = ((double)1/symbols_count);
			else
				e(i,j) = 0;
		}
    }

    for (unsigned i=0; i<a.size1();i++){
    	for(unsigned j=0;j<a.size2();j++){
    		if(i==(unsigned)end_state )
    			a(i,j)=0;
    		else if (i==START_STATE){
				if (j==(unsigned)end_state || j==START_STATE)
					a(i,j)=0;
				else
					a(i,j)=((double) 1/(state_total_count-2));
    		}
    		else if (j==START_STATE)
    			a(i,j)=0;
    		else if (j==(unsigned)end_state)
    			a(i,j)=((double) 1/(state_total_count-1));
    		else
    		    a(i,j)=((double) 1/(state_total_count-1));
    	}
    }
}


int Hmm2::test(){
	using namespace boost::numeric::ublas;

    this->init();
    int pv1[60] = {3, 1, 5, 1, 1, 6, 2, 4, 6, 4, 4, 6, 6, 4, 4, 2, 4, 5, 3, 1, 1, 3, 2, 1, 6, 3, 1, 1, 6, 4, 1, 5, 2, 1, 3, 3, 6, 2, 5, 1, 4, 4, 5, 4, 3, 6, 3, 1, 6, 5, 6, 6, 2, 6, 5, 6, 6, 6, 6, 6};
    int pl1[60] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int pv2[60] = {6, 5, 1, 1, 6, 6, 4, 5, 3, 1, 3, 2, 6, 5, 1, 2, 4, 5, 6, 3, 6, 6, 6, 4, 6, 3, 1, 6, 3, 6, 6, 6, 3, 1, 6, 2, 3, 2, 6, 4, 5, 5, 2, 3, 6, 2, 6, 6, 6, 6, 6, 6, 2, 5, 1, 5, 1, 6, 3, 1};
    int pl2[60] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    int pv3[60] = {2, 2, 2, 5, 5, 5, 4, 4, 1, 6, 6, 6, 5, 6, 6, 5, 6, 3, 5, 6, 4, 3, 2, 4, 3, 6, 4, 1, 3, 1, 5, 1, 3, 4, 6, 5, 1, 4, 6, 3, 5, 3, 4, 1, 1, 1, 2, 6, 4, 1, 4, 6, 2, 6, 2, 5, 3, 3, 5, 6};
    int pl3[60] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};

    std::vector<int> seq;
    for (unsigned i=0; i<60; i++){
    	seq.push_back(pv3[i]-1); //kocky su cislovane od 1 po 6, stavy od 0 po 5
    }


//    this->Viterbi(seq);
    this->Forward(seq);
    this->Backward(seq);
//    this->PosterioriDecoding(seq);
//    std::cout<<"a:"<<a<<std::endl;
//    std::cout<<"e:"<<e<<std::endl;
//    this->BaumWelchTrainingBioStep(seq);
//    this->Forward(seq);

//    double a = 3;
//    double b = 0.012;
//    std::cout<< log(a) << " " << log(b) <<" "<<log(a+b)<<" "<< elogsum(log(a),log(b)) << std::endl;

    std::cout<<"a:"<<a<<std::endl;
    std::cout<<"e:"<<e<<std::endl;
    std::cout<<"zbohom"<<std::endl;
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
	 path_labels(i-1) = get_state_label(path(i));
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
	matrix<double> f_filled = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
	 for(unsigned k =0; k<f.size1(); k++){//stavy
		 for(unsigned l =0; l<f.size2(); l++){//stavy
				f(k,l) = NAN;
		 }
	 }
	/*
	 * Initialization - pravdepodobnost ze zacnem (prejdem zo Start do ineho) stavu
	 */
	 for(unsigned i =0; i<f.size1(); i++){
			 f(i,0) = -INFINITY; // log(0)
	 }
	 f(START_STATE,0) = 0; //log(1)
	/*
	 *Recursion
	 */

	 for(unsigned i=0; i<sequence.size(); i++){ //Observation time
		 for(int l=0; l<state_total_count; l++){ //FROM state
			 double logalpha=-INFINITY;
			 for (int k=0; k<state_total_count;k++){ //TO state
				 logalpha = elogsum(logalpha, elogproduct( f(k,i), elog(a(k,l))) );
			 }
			 if(!this->issilent(l)) //is state silent?
				 f(l,i+1) = elogproduct(logalpha,elog(e(l,sequence.at(i))));
			 else
				 f(l,i+1) = logalpha;
			 f_filled(l,i+1) = 10*i+1*l;
		 }
	 }
/*
 * Termination
 */


	 //sum(k)(f(k,L) * a(k,0))
	 double sum = -INFINITY;
	 for (int k=0; k<state_total_count; k++){
		 sum = elogsum(sum, elogproduct(f(k,sequence.size()), elog(a(k,end_state))));
	 }
	 posterior_probability=sum;
	 std::cout<<"f_prob:"<<(sum)<<std::endl;
	 std::cout<<"f(E,L):"<<f(end_state,sequence.size())<<std::endl;
	 std::cout<<"f:"<<f<<std::endl;
	 std::cout<<"f_f:"<<f_filled<<std::endl;

	 return f;
}


boost::numeric::ublas::matrix<double> Hmm2::Backward(std::vector<int> sequence){
/*
 * Initialization
 */
	matrix<double> b = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
//	 for(unsigned k =0; k<b.size1(); k++){//stavy
//				b(k,sequence.size()) = elog(a(k,end_state));
//	 }
	 for(unsigned k =0; k<b.size1(); k++){//stavy
		 for(unsigned l =0; l<b.size2(); l++){//stavy
			 if(l==sequence.size())
				 b(k,l) = 0;
			 else
				 b(k,l) = NAN;
		 }
	 }

	b(end_state,sequence.size()) = 0; //log(1)

/*
 * Recursion
 */

	 for(unsigned i=sequence.size(); (int)i>0; i--){ //Observation time
		 for(int k=0; k<state_total_count; k++){ //FROM state
			 double logbeta=-INFINITY;
			 for(int l=0; l<state_total_count; l++){//To state
				 if(!this->issilent(l) && i!=0){ //is state silent?
					 logbeta = elogsum(logbeta,
										elogproduct(elog(a(k,l)),
													elogproduct(elog(e(l, sequence.at(i-1))),
																b(l,i))
													));

				 }
				else{
					 logbeta = elogsum(logbeta, elogproduct(elog(a(k,l)),b(l,i-1)));
				}
			 }
			 b(k,i-1) = logbeta;
		 }
	 }
/*
 * Termination
 */

	 double sum=-INFINITY;
	 for(int l=0;l<state_total_count;l++){
		 sum = elogsum(sum,
					   elogproduct(elog(a(0,l)),
								   elogproduct(elog(e(l, sequence.at(0))),
											   b(l,1)
											  )
								   )
						);
	 }

	 posterior_probability=sum;
	 std::cout<<"b_prob:"<<(sum)<<std::endl;
	 std::cout<<"b(S,0):"<<b(START_STATE,0)<<std::endl;
	 std::cout<<"b:"<<b<<std::endl;

	 return b;
}

matrix<double> Hmm2::P_pi_k_x(std::vector<int> sequence){
	matrix<double> f = this->Forward(sequence);
	matrix<double> b = this->Backward(sequence);
	matrix<double> p(state_total_count, sequence.size());
	for(unsigned i=0;i<sequence.size();i++){
		for(int k=0;k<state_total_count;k++){
			p(k,i)= elogdiv( elogproduct(f(k,i), b(k,i)), posterior_probability);
		}
	}
	std::cout<<"p:"<<p<<std::endl;
	return p;
}

matrix<double> Hmm2::P_pi_k_x2(std::vector<int> sequence){
	matrix<double> f = this->Forward(sequence);
	matrix<double> b = this->Backward(sequence);
	matrix<double> p(state_total_count, sequence.size());
	for(unsigned i=0;i<sequence.size();i++){
		double normalizer = -INFINITY;
		for(int k=0;k<state_total_count;k++){
			p(k,i)= elogproduct(f(k,i), b(k,i));
			normalizer = elogsum(normalizer, p(k,i));
		}
		for(int k=0;k<state_total_count;k++){
			p(k,i)= elogproduct( p(k,i), -normalizer);
		}
	}
	std::cout<<"p2:"<<p<<std::endl;
	return p;
}



double Hmm2::PosterioriDecoding(std::vector<int> sequence){
	matrix<double> p = this->P_pi_k_x(sequence);
	matrix<double> p2 = this->P_pi_k_x2(sequence);

	vector<int> path(sequence.size());
	vector<double> g(sequence.size()); //pravdepodobnosti ze znak vypusteny v case i bol zo stavu v geste
	for (unsigned i=0; i<sequence.size();i++){
		/*
		 * Prva moznost - zistit path (nemusi vsak naozaj existovat)
		 */
		double max=-INFINITY;
		int max_k=0;
		for(int k=0; k<state_total_count;k++){
			double val=p(k,i);
			if(max<val){
				max=val;
				max_k=k;
			}
		}
		path(i)=max_k;
		/*
		 * Druha moznost zistit derived property v nasom pripade kategoriu (gesto-negesto)
		 *
		 */
		double sum=-INFINITY;
		for(int k=0; k<state_total_count;k++){
			sum = elogsum(sum, elogproduct(elog(this->g(k)),
											p(k,i)
										)
						);
		}
		g(i)=sum;
	}


	std::cout<<"path:"<<path<<std::endl;
	std::cout<<"g:"<<g<<std::endl;
	std::cout<<"pp:"<<posterior_probability<<std::endl;

	return 0;
}




bool Hmm2::get_state_label(int state){
	return ( 0 < state and  state < state_gesture_count+1); //1 ak je v hladanom geste 0 inak
}

double Hmm2::g(int state){
	if(this->get_state_label(state))
		return 1;
	else
		return 0;
}

bool Hmm2::issilent(int state){
	return false;//(s.find(state)!=s.end());
}


boost::numeric::ublas::matrix<double> Hmm2::ForwardLabeled(std::vector<int> sequence,std::vector<int> labels){
	if (sequence.size()!=labels.size())
		throw 99;

	matrix<double> f = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
	/*
	 * Initialization - pravdepodobnost ze zacnem (prejdem zo Start do ineho) stavu
	 */
	 for(unsigned i =0; i<f.size1(); i++){
		 for(unsigned j=0; j<f.size2();j++){
			 f(i,j) = -INFINITY;
		 }
	 }
	 f(START_STATE,0) = elog(1);
	/*
	 *Recursion
	 */
	 for(unsigned i=0; i<sequence.size(); i++){ //Observation time
		 for(int l=1; l<state_total_count; l++){ //FROM state
			 if (labels.at(i)==this->get_state_label(l)){
				 double logalpha=-INFINITY;
				 for (int k=0; k<state_total_count;k++){ //TO state
					 logalpha = elogsum(logalpha, elogproduct( f(k,i), elog(a(k,l))) );
				 }
				 f(l,i+1) = elogproduct(logalpha,elog(e(l,sequence.at(i))));
			 }
			 else
				 f(l,i+1) = elog(0); //niesom si isty tym log()
		 }
	 }
/*
 * Termination
 */
	 double sum = -INFINITY;
	 for (int k=0; k<state_total_count; k++){
		 sum = elogsum(sum, elogproduct(f(k,sequence.size()), elog(a(k,end_state))));
	 }
//	 posterior_probability=sum;
	 std::cout<<"prob:"<<exp(sum)<<std::endl;
	 std::cout<<"f:"<<f<<std::endl;
	 return f;
}


boost::numeric::ublas::matrix<double> Hmm2::BackwardLabeled(std::vector<int> sequence,std::vector<int> labels){
	if (sequence.size()!=labels.size())
			throw 99;
/*
 * Initialization
 */
	matrix<double> b = matrix<double>(state_total_count, sequence.size()+1); //kvoli zaciatocnemu stavu
	 for(unsigned k =0; k<b.size1(); k++){//stavy
				b(k,sequence.size()) = elog(a(k,end_state));
	 }
/*
 * Recursion
 */
	 for(unsigned i=sequence.size(); i>0; i--){ //Observation time
		 for(int k=0; k<state_total_count; k++){ //FROM state
			 if (labels.at(i)==this->get_state_label(k)){
				 double logbeta=-INFINITY;
				 for(int l=0; l<state_total_count; l++){//To state
					 logbeta = elogsum(logbeta,
										elogproduct(elog(a(k,l)),
													elogproduct(elog(e(l, sequence.at(i-1))),
																b(l,i)
																)
													)
										);
				 }
				 b(k,i-1) = logbeta;
			 }
			 else
				 b(k,i-1) = elog(0);
		 }
	 }
/*
 * Termination
 */

	 double sum=-INFINITY;
	 for(int l=0;l<state_total_count;l++){
		 sum = elogsum(sum,
					   elogproduct(elog(a(0,l)),
								   elogproduct(elog(e(l, sequence.at(0))),
											   b(l,1)
											  )
								   )
						);
	 }
//	 posterior_probability=sum;
	 std::cout<<"b_prob:"<<exp(sum)<<std::endl;
	 std::cout<<"b:"<<b<<std::endl;
	 return b;
}


void Hmm2::BaumWelchTraining(std::vector<int> sequence){
/*
	matrix<double> es(sequence.size(),state_total_count, state_total_count);
	for(unsigned t=0; t<sequence.size(); t++){
		double normalizer = -INFINITY;
		for (int i=0; i<state_total_count; i++){
			for(int j=0;  j<state_total_count; j++){
				es(t, i,j) = elogproduct(	f(i,t),
										elogproduct(elog(a(i,j)),
													elogproduct(b(i,t+1),
																elog(e(j, sequence.at(t+1)))
																)
													)
									);
				normalizer = elogsum(normalizer, es(t, i,j));
			}
		}
		for (int i=0;i<state_total_count; i++){
			for(int j=0; j<state_total_count; j++){
				es(t, i, j) = elogproduct(es(t,i,j), -normalizer);
			}
		}

	}

*/

}


void Hmm2::BaumWelchTrainingBio(std::vector<int> sequence){



}
void Hmm2::BaumWelchTrainingBioStep(std::vector<int> sequence){
	matrix<double> f = this->Forward(sequence);
	matrix<double> b = this->Backward(sequence);
	matrix<double> a_count(state_total_count, state_total_count); //pocet pouziti daneho spojenia
	matrix<double> e_count(state_total_count, symbols_count) ;
	vector<double> a_sum_count(state_total_count);
	vector<double> e_sum_count(state_total_count);

	std::cout<<"b:"<<b<<std::endl;

	/*
	 * Hladame (odhadujeme) pocet pouziti spojeni medzi vrcholmi
	 */
	for(unsigned k =0; k<(unsigned)state_total_count;k++){
		for(unsigned l=0; l<(unsigned)state_total_count;l++){
			double sum = -INFINITY;
			for(unsigned i=0;i<sequence.size(); i++){
				if (!this->issilent(l)){
					sum = elogsum(sum, elogproduct(f(k,i),
							elogproduct(elog(a(k,l)),
								elogproduct(elog(e(l,sequence.at(i))), //i+1
										b(l,i+1)
									)
								)
							  )
							);
				}
				else{
					sum = elogsum(sum, elogproduct(f(k,i),
							elogproduct(elog(a(k,l)),
										b(l,i+1)
								)
							  )
							);
				}
				if(this->issilent(l))
					std::cerr<<sum
						<<"s"<<(s.find(l)!=s.end())
						<<" a:"<<a(k,l)
						<<" e:"<<e(l,sequence.at(i))
						<<" b:"<<b(l,i+1)
						<<" f:"<<f(k,i+1)
						<<" k:"<<k
						<<" l:"<<l
						<<" i:"<<i
						<<std::endl;
			}
			a_count(k,l) = (elogdiv(sum, posterior_probability));
			a_sum_count(k) = elogsum(a_sum_count(k), a_count(k,l));
		}
	}
	/*
	 * Hladame (odhadujeme) pocet emisii stavov
	 */

	for(unsigned k =0; k<(unsigned)state_total_count;k++){
		for(int bi=0;bi<symbols_count;bi++){
			double sum = -INFINITY;
			for(unsigned i=0;i<sequence.size();i++){
				if (sequence.at(i)==bi){
					sum=elogsum(sum, elogproduct(f(k,i+1),b(k,i+1)) );
				}
			}
			e_count(k,bi)=(elogdiv(sum,posterior_probability));
			e_sum_count(k) = elogsum(e_sum_count(k), e_count(k,bi));
		}
	}

	std::cout<<"acount:"<<a_count<<std::endl;
	std::cout<<"ecount:"<<e_count<<std::endl;
	std::cout<<"a_sum_count:"<<a_sum_count<<std::endl;
	std::cout<<"e_sum_count:"<<e_sum_count<<std::endl;

	/*
	 * Zmenit model ... toto by sa malo diat iba raz za iteraciu, predosle by malo pre kazdu sekvenciu
	 */
	for(unsigned k =0; k<(unsigned)state_total_count;k++){
		for(unsigned l=0; l<(unsigned)state_total_count;l++){
			a(k,l)= eexp(elogdiv(a_count(k,l), a_sum_count(k)));
		}
	}

	for(unsigned k =0; k<(unsigned)state_total_count;k++){
		for(int bi=0;bi<symbols_count;bi++){
			e(k,bi) = eexp(elogdiv(e_count(k,bi), e_sum_count(k)));
		}
	}

}



