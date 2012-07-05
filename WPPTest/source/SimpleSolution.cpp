/*
 * SimpleSolution.cpp
 *
 *  Created on: Jul 4, 2012
 *      Author: pom
 */

#include "define.h"
#include "HMM.hpp"
#include <wiiuse.h>
#include "Terminal.hpp"
#include <vector>
#include <set>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

using namespace std;

void printVV(vector<vector<HMM_PROB_TYPE> > V, Terminal & term) {
	ostringstream line;
	line << fixed;
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



#define MAX_WIIMOTES 1
#define TRAINING_MOVES 10
#define WM_MACRO wiimote_t* wm = this->wiimotes[0];
#define RAW_DATA_TYPE vec3f_t


class SimpleSolution {

private:
	wiimote** wiimotes;
	int found, connected;


public:
	int main();
	int init();
	RAW_DATA_TYPE get();
	int train(HMM& hmm);
	vector<O_TYPE> quantize(vector<RAW_DATA_TYPE> v);
	Observation observe();


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

	if (!this->init())
			exit(0);

	STDTerminal terminal;
	HMM hmm1(terminal);
	struct wiimote_t* wm = this->wiimotes[0];
	int should_exit = 0;
	while(!should_exit){
		if (wiiuse_poll(this->wiimotes, MAX_WIIMOTES) && wm->event == WIIUSE_EVENT){ //zatial len pre 1 wiimote
			if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_HOME))
				should_exit++;
			/*
			 * TRAIN IT!
			 */
			if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_PLUS)){
				printf("training!\n");
				this->train(hmm1);
				hmm1.Print();
			}

			/*
			 * RECOGNIZE IT!
			 */
			if(IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_MINUS)){
				printf("recognizing!\n");
				printf("probability: %f\n",hmm1.Viterbi(this->observe()));
			}

		}
	}
	exit(0);
}

bool is_similar(RAW_DATA_TYPE a, RAW_DATA_TYPE b){
	const int threshold = 33;
	if ((abs(a.x-b.x)>threshold) || (abs(a.x-b.x)>threshold) || (abs(a.x-b.x)>threshold))
		return false;
	return true;
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

	while (IS_PRESSED(wm,WIIMOTE_BUTTON_A)){ //nahrame ukazkove gesto
		raw_data.push_back(this->get());
	}

	points_to_clusters = this->quantize(raw_data);
	Observation o;
	for (j=0;j<raw_data.size();j++){
		/*
		 if (j!=0 && is_similar(raw_data[j],raw_data[j-1])) //Filter na rovnake predosle, trosku brutalny
			continue;
		*/
		o.putNew(points_to_clusters[j]);
	}
	return o;
}

int SimpleSolution::train(HMM& hmm){
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

	return out;
}

#define DIST_TYPE double
DIST_TYPE dist(const RAW_DATA_TYPE& a, const RAW_DATA_TYPE& b){
	return (DIST_TYPE)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z)); //sqrt je monotonne, netreba ho
}

vector<O_TYPE> SimpleSolution::quantize(vector<RAW_DATA_TYPE> v){
//INIT
	int i = 0;
	vector<RAW_DATA_TYPE> centroids; 	// body okolo podla ktorych su clustery
	vector< set<int> > clusters_to_points; 	//spytas sa cluster_id a on ti vrati set pid
	vector<O_TYPE> points_to_clusters;	// spytas sa point_id a on ti vrati cluster_id

	RAW_DATA_TYPE point = {0,0,0};
	for (i=0;i<O_SYMBOL_COUNT;i++){
		centroids.push_back(point);
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

		//VYPOCITAJ CENTROIDY (UPDATE/MAXIMIZATION)
		int num_points_in_cluster;
		int cid = 0;
		BOOST_FOREACH(RAW_DATA_TYPE& centroid,centroids){
			num_points_in_cluster = 0;
			BOOST_FOREACH(int pid, clusters_to_points[cid]){
				RAW_DATA_TYPE p = v[pid];
				centroid.x += p.x;
				centroid.y += p.y;
				centroid.z += p.z;
				num_points_in_cluster++;
			}
			centroid.x /= num_points_in_cluster;
			centroid.y /= num_points_in_cluster;
			centroid.z /= num_points_in_cluster;
			cid++;
		}

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

