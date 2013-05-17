/*
 * define.h
 *
 *  Created on: Feb 6, 2012
 *      Author: pom
 *
 *     LEN HLUPACI SI NEPISU COMMENTY A POTOM PLACU ZE NEVEDIA CO TYM CHCELI POVEDAT!!!:'(
 *
 *
 */

#ifndef DEFINE_H_
#define DEFINE_H_

//#define FORWII
#define O_TYPE int
#define O_SYMBOL_COUNT 14		//	kolko znakov ma abeceda pohybu (do ktorej premieta quantizer)
#define HMM_PROB_TYPE long double
#define HMM_STATE_COUNT 20
#define VERBOSITY 0		//	cim vyssia tym viac povie
#define r_trans 0e+1		//	pravdepodobnost ktora sa pripocitava pri trenovani
#define r_emiss 0e+1		//	pravdepodobnost ktora sa pripocitava pri trenovani
#define WINDOW_SIZE 3			//	dlzka rozpoznavaneho gesta



/*From SimpleSolution*/
#define MAX_WIIMOTES 1
#define TRAINING_MOVES 3
#define WM_MACRO wiimote_t* wm = this->wiimotes[0];
#define RAW_DATA_TYPE vec3f_t
#define DIST_TYPE long double

#endif /* DEFINE_H_ */
