/*
 * SimpleSolution.cpp
 *
 * (pokus zacat odznova)
 *
 *  Created on: Jul 4, 2012
 *      Author: pom
 *
 *
 *
 *      vizualizacia dat
 *      ocistenie od gravitacie, smoothing
 */

#include "define.h"
#include "HMM.hpp"
#include <wiiuse.h>
#include "Terminal.hpp"
#include <vector>
#include <set>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/minmax.hpp>
#include <math.h>
#include <fstream>
#include "Graph.hpp"

//gnuplot > splot "raw_data.txt" 1:2:3 with lines



using namespace std;


#define DIST_TYPE long double
DIST_TYPE dist(const RAW_DATA_TYPE& a, const RAW_DATA_TYPE& b){ //BEZ SQRT
	return (DIST_TYPE)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z)); //sqrt je monotonne, netreba ho
}

class SimpleSolution {

private:
	wiimote** wiimotes;
	int found, connected;
	vector<RAW_DATA_TYPE> centroids; 	// body okolo podla ktorych su clustery
	bool quantizer_trained;
	HMM_PROB_TYPE default_probability;
	HMM_PROB_TYPE default_probability0;
	HMM_PROB_TYPE default_o_len;
	RAW_DATA_TYPE filter_prev_r;
	Graph* plplot_graph;
	struct wiimote_t* wm;


public:
	int main();
	int init();
	RAW_DATA_TYPE get();
	int train(HMM& hmm, HMM& abzdf);
	vector<O_TYPE> quantize(vector<RAW_DATA_TYPE> v);
	vector<O_TYPE> quantize_clustering(vector<RAW_DATA_TYPE> v);
	vector<O_TYPE> quantize_space_part(vector<RAW_DATA_TYPE> v);
	Observation observe();
	vector<RAW_DATA_TYPE> observe_raw();
	void train_centroids(vector<RAW_DATA_TYPE> v);
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

	STDTerminal terminal;
	HMM hmm1(terminal);
	HMM hmm0(terminal);
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
				BOOST_FOREACH(RAW_DATA_TYPE rd, this->centroids){
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

				HMM_PROB_TYPE viter0 = hmm0.ViterbiLog(observation_now);
				HMM_PROB_TYPE prob0 = hmm0.GetProb(observation_now);
				printf("%s\n",observation_now.printOut().c_str());
				printf("probability Viterbi: %Le %Le\n", viter, viter0);
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
	uint j;
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

	points_to_clusters = this->quantize(raw_data);
	Observation o;
	for (j=0;j<raw_data.size();j++){
/*		 if (j!=0 && is_similar(raw_data[j],raw_data[j-1])) //Filter na rovnake predosle, trosku brutalny
			continue;*/
		o.putNew(points_to_clusters[j]);
	}
	return o;
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

void SimpleSolution::train_centroids(vector<RAW_DATA_TYPE> v){
	//INIT
		int i = 0;
		vector<RAW_DATA_TYPE> centroids; 	// body okolo podla ktorych su clustery
		vector< set<int> > clusters_to_points; 	//spytas sa cluster_id a on ti vrati set pid
		vector<O_TYPE> points_to_clusters;	// spytas sa point_id a on ti vrati cluster_id

		int origin = 0; //128
		DIST_TYPE r  = 0;
		DIST_TYPE rs  = 0;
		BOOST_FOREACH(RAW_DATA_TYPE val, v){
			r += sqrt(dist(val, (RAW_DATA_TYPE){origin,origin,origin})); // DIST JE BEZ SQRT!!!
//			printf("x:%f y:%f z:%f r:%Lf \n",val.x,val.y,val.z,sqrt(dist(val, (RAW_DATA_TYPE){origin,origin,origin})));
		}

		r = r/v.size();
		rs = r*sin(M_PI_4);
		printf("\n r: %Lf  rs:%Lf\n",r,rs);

/*
		//Two Circles
		centroids.push_back((RAW_DATA_TYPE){origin,origin,origin+r});
		centroids.push_back((RAW_DATA_TYPE){origin,origin+rs,origin+rs});
		centroids.push_back((RAW_DATA_TYPE){origin,origin+r, origin});
		centroids.push_back((RAW_DATA_TYPE){origin,origin+rs,origin-rs});
		centroids.push_back((RAW_DATA_TYPE){origin,origin,   origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin,origin-rs,origin-rs});
		centroids.push_back((RAW_DATA_TYPE){origin,origin-r, origin});
		centroids.push_back((RAW_DATA_TYPE){origin,origin-rs,origin+rs});

		centroids.push_back((RAW_DATA_TYPE){origin+r, origin,origin});
		centroids.push_back((RAW_DATA_TYPE){origin+rs,origin,origin-rs});
		centroids.push_back((RAW_DATA_TYPE){origin-rs,origin,origin-rs});
		centroids.push_back((RAW_DATA_TYPE){origin-r, origin,origin});
		centroids.push_back((RAW_DATA_TYPE){origin-rs,origin,origin+rs});
		centroids.push_back((RAW_DATA_TYPE){origin+rs,origin,origin+rs});
		plplot_graph->clear_c();
		BOOST_FOREACH(RAW_DATA_TYPE rd, centroids){
			printf("%f %f %f \n",rd.x,rd.y,rd.z);
			plplot_graph->c((double)rd.x, (double)rd.y, (double)rd.z);
		}
		wait();*/



		//CUBE + midpoints (r~=128)
		r=40;
		centroids.push_back((RAW_DATA_TYPE){origin-r,origin-r,origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin-r,origin+r,origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin-r,origin+r,origin+r});
		centroids.push_back((RAW_DATA_TYPE){origin-r,origin-r,origin+r});
		centroids.push_back((RAW_DATA_TYPE){origin+r,origin-r,origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin+r,origin+r,origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin+r,origin+r,origin+r});
		centroids.push_back((RAW_DATA_TYPE){origin+r,origin-r,origin+r});

		centroids.push_back((RAW_DATA_TYPE){origin,origin,origin-r});
		centroids.push_back((RAW_DATA_TYPE){origin,origin,origin+r});
		centroids.push_back((RAW_DATA_TYPE){origin,origin-r,origin});
		centroids.push_back((RAW_DATA_TYPE){origin,origin+r,origin});
		centroids.push_back((RAW_DATA_TYPE){origin-r,origin,origin});
		centroids.push_back((RAW_DATA_TYPE){origin+r,origin,origin});
//		plplot_graph->clear_c();
		BOOST_FOREACH(RAW_DATA_TYPE rd, centroids){
			printf("%f %f %f \n",rd.x,rd.y,rd.z);
//			plplot_graph->c((double)rd.x, (double)rd.y, (double)rd.z);
		}
//		wait();


//		RAW_DATA_TYPE point = {0,0,0};
		for (i=0;i<O_SYMBOL_COUNT;i++){
//			centroids.push_back(point);
			set<int> s;
			clusters_to_points.push_back(s);
		}



	//K-MEANS

		bool some_point_is_moving = true;
		int num_iterations = 0;
		int cid = 0;

		uint pid;
		for(pid = 0; pid<v.size(); pid++){
			points_to_clusters.push_back(pid%O_SYMBOL_COUNT);
			clusters_to_points[pid%O_SYMBOL_COUNT].insert(pid);
		}

		while (some_point_is_moving && num_iterations < 10){ //kym nekonvergujeme
			some_point_is_moving = false;
			//UPRAV BODY (ASSIGNMENT/EXPECTATION)
			DIST_TYPE d, min;
			bool move;
			int to_cluster;
			for (pid=0; pid<v.size();pid++){
				min = dist(centroids[points_to_clusters[pid]],v[pid]);
				cid = 0;
				move=false;
				BOOST_FOREACH(RAW_DATA_TYPE c, centroids){
					d = dist(c,v[pid]);
					if (d<min){
						min = d;
						move = true;
						to_cluster = cid;
						clusters_to_points[points_to_clusters[pid]].erase(pid);

						some_point_is_moving = true;
					}
					cid++;
				}

				if (move){
					points_to_clusters[pid] = to_cluster;
					clusters_to_points[to_cluster].insert(pid);
				}
			}





			//VYPOCITAJ CENTROIDY (UPDATE/MAXIMIZATION)
			int num_points_in_cluster;
			cid = 0;
			BOOST_FOREACH(RAW_DATA_TYPE& centroid,centroids){
				num_points_in_cluster = 0;
				BOOST_FOREACH(int pid, clusters_to_points[cid]){
					RAW_DATA_TYPE p = v[pid];
					centroid.x += p.x;
					centroid.y += p.y;
					centroid.z += p.z;
					num_points_in_cluster++;
				}
				if (num_points_in_cluster>0){
					centroid.x /= num_points_in_cluster;
					centroid.y /= num_points_in_cluster;
					centroid.z /= num_points_in_cluster;
				}
				cid++;
			}


			plplot_graph->clear_c();
			BOOST_FOREACH(RAW_DATA_TYPE rd, centroids){
				printf("%f %f %f \n",rd.x,rd.y,rd.z);
				plplot_graph->c((double)rd.x, (double)rd.y, (double)rd.z);
			}
			wait();

			num_iterations++;
		}//while some_point_is_moving

	this->centroids = centroids;
	this->quantizer_trained = true;
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
	uint i,j;
	vector<vector<RAW_DATA_TYPE> > training_observations;
	vector<RAW_DATA_TYPE> master_observation;

	for (i=0;i<TRAINING_MOVES;i++){ //potrebujeme viacero treningovych ukazok
		printf("training gesture %d/%d\n",i+1,TRAINING_MOVES);
		training_observations.push_back(this->observe_raw());
	}

	ofstream file_raw_data("raw_data.txt");
	BOOST_FOREACH(vector<RAW_DATA_TYPE> obs, training_observations){
		BOOST_FOREACH(RAW_DATA_TYPE raw_data, obs){
				master_observation.push_back(raw_data);
				file_raw_data <<raw_data.x<<" "<<raw_data.y<<" "<<raw_data.z<<"\n";
		}
	}
	file_raw_data.close();

	this->train_centroids(master_observation);
	vector<Observation> obss;
	vector<O_TYPE> points_to_clusters;
	BOOST_FOREACH(vector<RAW_DATA_TYPE> gesture_raw, training_observations){
			points_to_clusters = this->quantize(gesture_raw);
			Observation o;
			for (j=0;j<gesture_raw.size();j++){
				o.putNew(points_to_clusters[j]);
			}
			obss.push_back(o);
	}

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
	BOOST_FOREACH(Observation prob_observ, obss){
		prob+=hmm.GetProb(prob_observ);
		printf("%s\n",prob_observ.printOut().c_str());
	}
	this->default_probability = prob/obss.size();
	this->default_o_len = master_observation.size()/obss.size();

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

//	plplot_graph->p((double)out.x, (double)out.y, (double)out.z);
	plplot_graph->p((double)wm->orient.yaw, (double)wm->orient.pitch, (double)wm->orient.roll);
	return out;
}


bool SimpleSolution::filter_out(RAW_DATA_TYPE r){

	double r_abs = sqrt(dist(r,(RAW_DATA_TYPE){0,0,0})); //128
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

vector<O_TYPE> SimpleSolution::quantize(vector<RAW_DATA_TYPE> v){
	return quantize_clustering(v);
}

vector<O_TYPE> SimpleSolution::quantize_space_part(vector<RAW_DATA_TYPE> v){
	//INIT
//iba skopirovane zatial

		if (!this->quantizer_trained)
			this->train_centroids(v);

		int i = 0;
		vector< set<int> > clusters_to_points; 	//spytas sa cluster_id a on ti vrati set pid
		vector<O_TYPE> points_to_clusters;	// spytas sa point_id a on ti vrati cluster_id

		for (i=0;i<O_SYMBOL_COUNT;i++){
			set<int> s;
			clusters_to_points.push_back(s);
		}

	//K-MEANS

		bool some_point_is_moving = true, move;
		int num_iterations = 0;

		uint pid;
		for(pid = 0; pid<v.size(); pid++){
			points_to_clusters.push_back(pid%O_SYMBOL_COUNT);
			clusters_to_points[pid%O_SYMBOL_COUNT].insert(pid);
		}

		while (some_point_is_moving){ //kym nekonvergujeme
			some_point_is_moving = false;
			int cid = 0;

			//UPRAV BODY (ASSIGNMENT/EXPECTATION)
			DIST_TYPE d, min;
			int to_cluster;
			for (pid=0; pid<v.size();pid++){
				min = dist(centroids[points_to_clusters[pid]],v[pid]);
				cid = 0;
				move=false;
				BOOST_FOREACH(RAW_DATA_TYPE c, centroids){
					d = dist(c,v[pid]);
					if (d<min){
						min = d;
						move = true;
						to_cluster = cid;
						clusters_to_points[points_to_clusters[pid]].erase(pid);

						some_point_is_moving = true;
					}
					cid++;
				}

				if (move){
					points_to_clusters[pid] = to_cluster;
					clusters_to_points[to_cluster].insert(pid);
				}
			}
			num_iterations++;
		}//while some_point_is_moving

		return points_to_clusters;

}


vector<O_TYPE> SimpleSolution::quantize_clustering(vector<RAW_DATA_TYPE> v){
//INIT

	if (!this->quantizer_trained)
		this->train_centroids(v);

	int i = 0;
	vector< set<int> > clusters_to_points; 	//spytas sa cluster_id a on ti vrati set pid
	vector<O_TYPE> points_to_clusters;	// spytas sa point_id a on ti vrati cluster_id

	for (i=0;i<O_SYMBOL_COUNT;i++){
		set<int> s;
		clusters_to_points.push_back(s);
	}

//K-MEANS

	bool some_point_is_moving = true, move;
	int num_iterations = 0;

	uint pid;
	for(pid = 0; pid<v.size(); pid++){
		points_to_clusters.push_back(pid%O_SYMBOL_COUNT);
		clusters_to_points[pid%O_SYMBOL_COUNT].insert(pid);
	}

	while (some_point_is_moving){ //kym nekonvergujeme
		some_point_is_moving = false;
		int cid = 0;

		//UPRAV BODY (ASSIGNMENT/EXPECTATION)
		DIST_TYPE d, min;
		int to_cluster;
		for (pid=0; pid<v.size();pid++){
			min = dist(centroids[points_to_clusters[pid]],v[pid]);
			cid = 0;
			move=false;
			BOOST_FOREACH(RAW_DATA_TYPE c, centroids){
				d = dist(c,v[pid]);
				if (d<min){
					min = d;
					move = true;
					to_cluster = cid;
					clusters_to_points[points_to_clusters[pid]].erase(pid);

					some_point_is_moving = true;
				}
				cid++;
			}

			if (move){
				points_to_clusters[pid] = to_cluster;
				clusters_to_points[to_cluster].insert(pid);
			}
		}
		num_iterations++;
	}//while some_point_is_moving

	return points_to_clusters;
}

