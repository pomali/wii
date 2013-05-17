/*
 * VectToSymb.cpp
 *
 *  Created on: Mar 29, 2013
 *      Author: pom
 */

#include "VectToSymb.hpp"


DIST_TYPE dist(const RAW_DATA_TYPE& a, const RAW_DATA_TYPE& b){ //BEZ SQRT
	return (DIST_TYPE)((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z)); //sqrt je monotonne, netreba ho
}

VectToSymb::VectToSymb(Graph* g){
	plplot_graph = g;
}

O_TYPE VectToSymb::transform(RAW_DATA_TYPE v)
{
	return space_partition(v);
}

bool VectToSymb::train(vector< vector<RAW_DATA_TYPE> > training_observations){
	return clustering_train(training_observations);
}

bool VectToSymb::clustering_train(vector< vector<RAW_DATA_TYPE> > training_observations)
{
	BOOST_FOREACH(vector<RAW_DATA_TYPE> obs, training_observations){
		BOOST_FOREACH(RAW_DATA_TYPE raw_data, obs){
				master_observation.push_back(raw_data);
		}
	}

	this->train_centroids(master_observation);

	return true;
}


O_TYPE VectToSymb::space_partition(RAW_DATA_TYPE v)
{

	int o = 0;

#if O_SYMBOL_COUNT>27
		float treshold = 30;
		if(v.x>treshold)
			o+=2;
		else if(v.x<(-1*treshold))
			o+=1;
		if(v.y>treshold)
					o+=6;
				else if(v.y<(-1*treshold))
					o+=3;
		if(v.z>treshold)
					o+=9;
				else if(v.x<(-1*treshold))
					o+=18;
#else
		if(v.x>0)
			o+=1;
		if(v.y>0)
			o+=2;
		if(v.z>0)
			o+=4;
#endif

		return o;
}

vector<Observation> VectToSymb::vv_transform(vector< vector< RAW_DATA_TYPE> > v){
	vector<Observation> out;
	BOOST_FOREACH(vector<RAW_DATA_TYPE> gesture_raw, v){
			Observation o = this->v_transform(gesture_raw);
			out.push_back(o);
	}
	return out;
}


Observation VectToSymb::v_transform(vector<RAW_DATA_TYPE> raw_data){
	return v_space_partitioning(raw_data);
}


Observation VectToSymb::v_space_partitioning(vector<RAW_DATA_TYPE> raw_data){
	Observation out;
	BOOST_FOREACH(RAW_DATA_TYPE val, raw_data){
		out.putNew(space_partition(val));
	}
	return out;
}

Observation VectToSymb::v_kmean_clustering(vector<RAW_DATA_TYPE> raw_data){
	vector<O_TYPE> o_vector = quantize_clustering(raw_data);
	Observation out;
	BOOST_FOREACH(O_TYPE val, o_vector){
		out.putNew(val);
	}
	return out;
}



void VectToSymb::train_centroids(vector<RAW_DATA_TYPE> v){
	//INIT
	return;
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



vector<O_TYPE> VectToSymb::quantize_clustering(vector<RAW_DATA_TYPE> v){
//INIT
	//zatial iba skopirovane

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
