/*
 * Hmm2.h
 *
 *  Created on: Jun 10, 2013
 *      Author: pom
 */

#ifndef HMM2_H_
#define HMM2_H_

#include <set>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <cmath>


#define START_STATE 0

class Hmm2 {
public:
	boost::numeric::ublas::matrix<double> e, a; //emisna pravdepodobnost, a prechodova pravdepdoobnost



	std::set<int> s; //silent states


	int state_gesture_count, state_noise_count, state_total_count, symbols_count;
	int end_state;


	Hmm2();
	bool init();
	int test();
	boost::numeric::ublas::matrix<double> Viterbi(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> Forward(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> Backward(std::vector<int> sequence);
	bool is_in_gesture(int state);

	/*
	 * vsetko v logaritmickom priestore
	 *
	 * Viterbi
	 * Forward
	 * Backward
	 * ForwardBackward
	 *
	 * Baum-Welch training
	 *
	 * Viterbi training
	 *
	 *
	 *
	 */



	/*      /->N1->N2->..........Nn\->
	 * Start <-----------------------  End
	 * 		\->G1->G2->.........Gn/->
	 *
	 * Gi maju otocku na seba
	 * Ni sa mozu vratit kedykolvek na zaciatok
	 *
	 * Chceme urobit Baum-Welch training
	 *
	 * pridat silent state, start state, end state
	 *
	 * prerobit do boost (ma vypisy a podobne vyhody)
	 *
	 *
	 */



	virtual ~Hmm2();
};

#endif /* HMM2_H_ */
