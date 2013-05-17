/*
 * SimpleSolution.cpp
 *
 * (pokus zacat odznova)
 *
 *  Created on: Jul 4, 2012
 *      Author: pom
 *
 *      ocistenie od gravitacie, smoothing
 *      online viterbi, online rozpoznavanie (?)
 *      ROZPOZNAVANIE MEDZI GESTAMI
 */

#include "SimpleSolution.hpp"

//gnuplot > splot "raw_data.txt" 1:2:3 with lines



using namespace std;


DIST_TYPE dist_s(const RAW_DATA_TYPE& a, const RAW_DATA_TYPE& b){ //BEZ SQRT
	return (DIST_TYPE)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z)); //sqrt je monotonne, netreba ho
}


class SimpleSolution {

private:
	wiimote** wiimotes;
	int found, connected;
	HMM_PROB_TYPE default_probability;
	HMM_PROB_TYPE default_probability0;
	HMM_PROB_TYPE default_o_len;
	RAW_DATA_TYPE filter_prev_r;
	Graph* plplot_graph;
	struct wiimote_t* wm;
	VectToSymb* transformer;


public:
	int main();
	int init();
	RAW_DATA_TYPE get();
	int train(HMM& hmm, HMM& abzdf);
	Observation observe();
	vector<RAW_DATA_TYPE> observe_raw();
	bool filter_out(RAW_DATA_TYPE r);
	void wait();
};

int SimpleSolution::init(){
	this->wiimotes =  wiiuse_init(MAX_WIIMOTES);

	this->found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
	if (!this->found) {
		printf ("No wiimotes found.");
		return 0;
	}

	this->connected = wiiuse_connect(this->wiimotes, MAX_WIIMOTES);
	if (this->connected)
		printf("Connected to %i wiimotes (of %i found).\n", this->connected, this->found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}

	int i;
	for (i=0; i<MAX_WIIMOTES;i++){
		wiiuse_set_leds(this->wiimotes[i], WIIMOTE_LED_1);
		wiiuse_motion_sensing(this->wiimotes[i],1);
		wiiuse_rumble(this->wiimotes[i], 1);
		usleep(200000);
		wiiuse_rumble(wiimotes[i], 0);
	}

	return this->connected;
}


int SimpleSolution::main(){
	/* Postup:
	 * 		Natrenujeme HMM (pripadne loadneme nejaky ulozeny state)
	 * 		Rozpoznavame gesta
	 */

	plplot_graph = new Graph();

	if (!this->init())
			exit(0);

	transformer = new VectToSymb(plplot_graph);
	STDTerminal terminal;
	HMM hmm1(terminal);
	HMM hmm0(terminal);
	nullHMM nhmm(terminal);
	wm = this->wiimotes[0];
	int should_exit = 0;
	while(!should_exit){
		if (wiiuse_poll(this->wiimotes, MAX_WIIMOTES) && wm->event == WIIUSE_EVENT){ //zatial len pre 1 wiimote
			if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_HOME)){
				should_exit++;
				printf("bb\n");
			}

			/*
			 * TRAIN IT!
			 */
			if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_PLUS)){
				printf("training!\n");
				hmm1.init();
				hmm0.init();
				this->train(hmm1,hmm0);
				hmm1.Print();
//				plplot_graph->clear_c();
				BOOST_FOREACH(RAW_DATA_TYPE rd, transformer->centroids){
//					plplot_graph->c((double)rd.x, (double)rd.y, (double)rd.z);
					printf("%f %f %f\n",rd.x,rd.y,rd.z);
				}
			}

			/*graph rotation*/
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_RIGHT))
				plplot_graph->c3d_rotate_right();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_LEFT))
				plplot_graph->c3d_rotate_left();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_UP))
				plplot_graph->c3d_rotate_up();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_DOWN))
				plplot_graph->c3d_rotate_down();

			/*
			 * RECOGNIZE IT!
			 */
			if(IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_A)){
				printf("recognizing!\n");
				Observation observation_now;
				observation_now = this->observe();
				HMM_PROB_TYPE viter = hmm1.ViterbiLog(observation_now);
				HMM_PROB_TYPE prob = hmm1.GetProb(observation_now);
				HMM_PROB_TYPE viterO = hmm1.ViterbiLogOnline(observation_now);

				HMM_PROB_TYPE viter0 = nhmm.ViterbiLog(observation_now);//hmm0.ViterbiLog(observation_now);
				HMM_PROB_TYPE prob0 = nhmm.GetProb(observation_now);//hmm0.GetProb(observation_now);
				HMM_PROB_TYPE viterO0 = nhmm.ViterbiLogOnline(observation_now);//hmm0.ViterbiLogOnline(observation_now);
				printf("observation: %s\n",observation_now.printOut().c_str());
				printf("probability Viterbi: %Le %Le\n", viter, viter0);
				printf("prob Online Viterbi: %Le %Le\n", viterO, viterO0);
				printf("probability GetProb: %Le %Le\n",prob, prob0);
				printf("probability default: %Le %Le\n",this->default_probability, this->default_probability0);
				printf("probability Normalized_prob: %Le %Le\n",prob/this->default_probability, prob0/this->default_probability0);

				bool good_len = abs((observation_now.length()/this->default_o_len)-1) < 0.3;//tolerancia dlzky 30%
				bool good_prob = prob!=0 && (prob/this->default_probability > 1.2e-3);
				bool good_prob0 = prob0!=0 && (prob0/this->default_probability0 > 1.2e-3);
				if (good_len)
					printf("good len!\n");
				if (good_prob)
					printf("GEEGEE!\n");
				if (good_prob0)
					printf("GG!\n");
				if (good_len && good_prob0)
					printf("Recognized!\n");
			}

		}
	}
	exit(0);
}

/**
 * Returns true if two vectors are too similar, false if they aren't
 *
 */
bool is_similar(RAW_DATA_TYPE a, RAW_DATA_TYPE b){
	const int threshold = 10;
	if ((abs(a.x-b.x)>threshold) || (abs(a.x-b.x)>threshold) || (abs(a.x-b.x)>threshold))
		return false;
	return true;
}

void SimpleSolution::wait(){
	printf("waiting");
	int end = 0;
	while(!end){
		if (wiiuse_poll(this->wiimotes, MAX_WIIMOTES) && wm->event == WIIUSE_EVENT){
			if(IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE))
				end++;

			/*graph rotation*/
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_RIGHT))
				plplot_graph->c3d_rotate_right();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_LEFT))
				plplot_graph->c3d_rotate_left();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_UP))
				plplot_graph->c3d_rotate_up();
			if (IS_PRESSED(wm,WIIMOTE_BUTTON_DOWN))
				plplot_graph->c3d_rotate_down();
		}
	}
}


Observation SimpleSolution::observe(){
	WM_MACRO
//	uint j;
	vector<RAW_DATA_TYPE> raw_data;
	vector<O_TYPE> points_to_clusters;

	while (!IS_PRESSED(wm,WIIMOTE_BUTTON_A)){
		usleep(300);
		wiiuse_poll(this->wiimotes, MAX_WIIMOTES);
	}

	RAW_DATA_TYPE current,current_raw,previous_raw=this->get();//(RAW_DATA_TYPE){128,128,128}; //128
	current_raw = this->get(); 		//Musime odfiltrovat prvu s gravitaciou, a potom berieme zmeny ale potrebujeme mat aspon jednu
	current.x = current_raw.x-previous_raw.x;
	current.y = current_raw.y-previous_raw.y;
	current.z = current_raw.z-previous_raw.z;
	raw_data.push_back(current);
	previous_raw = current_raw;

	while (IS_PRESSED(wm,WIIMOTE_BUTTON_A)){ //nahrame ukazkove gesto
		current_raw = this->get();
		current.x = current_raw.x-previous_raw.x;
		current.y = current_raw.y-previous_raw.y;
		current.z = current_raw.z-previous_raw.z;
		if (this->filter_out(current))
			continue;
		raw_data.push_back(current);
		previous_raw = current_raw;
	}

	Observation trans_o =transformer->v_transform(raw_data);
	return trans_o;
}

vector<RAW_DATA_TYPE> SimpleSolution::observe_raw(){
	WM_MACRO
//	uint j;
	vector<RAW_DATA_TYPE> raw_data;

	while (!IS_PRESSED(wm,WIIMOTE_BUTTON_A)){
		usleep(300);
		wiiuse_poll(this->wiimotes, MAX_WIIMOTES);
	}

	RAW_DATA_TYPE current,current_raw,previous_raw=this->get();//(RAW_DATA_TYPE){128,128,128}; //128
		while (IS_PRESSED(wm,WIIMOTE_BUTTON_A)){ //nahrame ukazkove gesto
			current_raw = this->get();
			current.x = current_raw.x-previous_raw.x;
			current.y = current_raw.y-previous_raw.y;
			current.z = current_raw.z-previous_raw.z;
			if (this->filter_out(current))
				continue;
			raw_data.push_back(current);
			previous_raw = current_raw;
		}

	return raw_data;
}



/*int SimpleSolution::train(HMM& hmm){
	uint i;
	vector<Observation> training_observations;
	for (i=0;i<TRAINING_MOVES;i++){ //potrebujeme viacero treningovych ukazok
		printf("trainig gesture %d/%d\n",i+1,TRAINING_MOVES);
		training_observations.push_back(this->observe());
	}
	BOOST_FOREACH(Observation obs, training_observations)
		printf("%s\n",obs.printOut().c_str());


	hmm.Train(training_observations);
	printf("training done!\n");
	return 1;
}*/

int SimpleSolution::train(HMM& hmm, HMM& abzdf){
	uint i;
	vector<vector<RAW_DATA_TYPE> > training_observations;
	vector<Observation> obss;
	for (i=0;i<TRAINING_MOVES;i++){ //potrebujeme viacero treningovych ukazok
		printf("training gesture %d/%d\n",i+1,TRAINING_MOVES);
		training_observations.push_back(this->observe_raw());
	}

	int file_index =0;
	BOOST_FOREACH(vector<RAW_DATA_TYPE> obs, training_observations){
		stringstream sst_filename;
		sst_filename << "raw_data" << file_index++ << ".txt";
		ofstream file_raw_data(sst_filename.str().c_str());
		file_raw_data.setf(ios::fixed, ios::floatfield);
		BOOST_FOREACH(RAW_DATA_TYPE raw_data, obs){
//				master_observation.push_back(raw_data);
				file_raw_data <<raw_data.x<<" "<<raw_data.y<<" "<<raw_data.z<<"\n";
		}
		file_raw_data.close();
	}


	obss = transformer->vv_transform(training_observations);
	int train_size = training_observations.size();
	int obs_size = obss.size()+1;
	hmm.Print();
	hmm.Train(obss);
	hmm.Print();

	HMM_PROB_TYPE probb4 = 0;
//	while (((this->default_probability0-probb4) > 1e-200) || (probb4 == 0)){ //ajtak je to cele nanic,
	for (int i_aaa =0 ; i_aaa<2; i_aaa++){
		abzdf.Train(obss);
		HMM_PROB_TYPE prob=0;
		BOOST_FOREACH(Observation prob_observ, obss){
			prob+=abzdf.GetProb(prob_observ);
			printf("%s\n",prob_observ.printOut().c_str());
		}

		probb4 = this->default_probability0;
		this->default_probability0 = prob/obss.size();
	}

	HMM_PROB_TYPE prob=0;
	int total_len=0;
	BOOST_FOREACH(Observation prob_observ, obss){
		prob+=hmm.GetProb(prob_observ);
		total_len+=prob_observ.length();
		printf("%s\n",prob_observ.printOut().c_str());
	}
	this->default_probability = prob/obs_size;
	this->default_o_len = total_len/obs_size;

	printf("training done!\n");
	return 1;
}


int mainx(int argc, char **argv){
	SimpleSolution ss;
	return ss.main();
}

RAW_DATA_TYPE SimpleSolution::get(){
	WM_MACRO
	while (!wiiuse_poll(this->wiimotes, MAX_WIIMOTES) || wm->event != WIIUSE_EVENT)
		;
	RAW_DATA_TYPE out;
	out.x = wm->accel.x;
	out.y = wm->accel.y;
	out.z = wm->accel.z;

//	printf("%f %f %f\n",out.x,out.y,out.z);

	plplot_graph->p((double)out.x, (double)out.y, (double)out.z);
//	plplot_graph->p((double)wm->orient.yaw, (double)wm->orient.pitch, (double)wm->orient.roll);
	return out;
}


bool SimpleSolution::filter_out(RAW_DATA_TYPE r){

	double r_abs = sqrt(dist_s(r,(RAW_DATA_TYPE){0,0,0})); //128
	const bool verbose = false;

	if (verbose)
		printf("%f %f %f r:%f ",r.x, r.y, r.z, r_abs);

	const double idle_sensitivity = 3;
	if (r_abs < idle_sensitivity){
		if (verbose)
			printf("filter idle\n");
		return true;
	}
	/*
	const int directional_equiv_sensitivity = 2;
	if ((abs(r.x-filter_prev_r.x)<directional_equiv_sensitivity)
			&&(abs(r.x-filter_prev_r.x)<directional_equiv_sensitivity)
			&& (abs(r.x-filter_prev_r.x)<directional_equiv_sensitivity)){
		printf("filter directional equiv\n");
		return true;
	}
	filter_prev_r = r;
	*/
	if (verbose)
		printf("\n");
	return false;
}

