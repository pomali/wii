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
	double posterior_probability;

	Hmm2();
	bool init();
	void init_known_casino();
	void init_unknown_casino();
	int test();
	boost::numeric::ublas::matrix<double> Viterbi(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> Forward(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> Backward(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> ForwardLabeled(std::vector<int> sequence,std::vector<int> labels);
	boost::numeric::ublas::matrix<double> BackwardLabeled(std::vector<int> sequence,std::vector<int> labels);
	boost::numeric::ublas::matrix<double> P_pi_k_x(std::vector<int> sequence);
	boost::numeric::ublas::matrix<double> P_pi_k_x2(std::vector<int> sequence);
	double PosterioriDecoding(std::vector<int> sequence);
	void BaumWelchTraining(std::vector<int> sequence);
	void BaumWelchTrainingBio(std::vector<int> sequence);
	void BaumWelchTrainingBioStep(std::vector<int> sequence);
	bool get_state_label(int state);
	double g(int state);

	/*
	 * vsetko v logaritmickom priestore
	 *
	 * ForwardBackward (?)
	 *
	 * Baum-Welch training
	 *
	 * Viterbi training
	 *
	 * pridat (biol seq p64) a_r(k,l) e_r(k,b) = pseudocounts ktore sa pripocitavaju pri trenovani
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
