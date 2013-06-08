/*
 * VectToSymb.hpp
 *
 *  Created on: Mar 29, 2013
 *      Author: pom
 */

#ifndef VECTTOSYMB_HPP_
#define VECTTOSYMB_HPP_

#include "Observation.hpp"
#include "Graph.hpp"
#include <boost/foreach.hpp>
#include <vector>
#include <set>
#include <wiiuse.h>
#include <math.h>

/*
 * Transform vector (x,y,z) into symbol n (0<n<O_SYMBOL_COUNT)
 */
class VectToSymb{
public:
	vector<RAW_DATA_TYPE> centroids; 	// body okolo podla ktorych su clustery
	VectToSymb(Graph* g);
	O_TYPE transform(RAW_DATA_TYPE v); //Transform single value
	O_TYPE space_partition(RAW_DATA_TYPE v);

	Observation v_transform(vector<RAW_DATA_TYPE> raw_data); //Transform vector/list of vectors
	Observation v_kmean_clustering(vector<RAW_DATA_TYPE> raw_data);
	Observation v_space_partitioning(vector<RAW_DATA_TYPE> raw_data);

	bool train(vector< vector<RAW_DATA_TYPE> > training_observations);
	bool clustering_train(vector< vector<RAW_DATA_TYPE> > training_observations);
	vector<Observation> vv_transform(vector< vector< RAW_DATA_TYPE> > v);
	void train_centroids(vector<RAW_DATA_TYPE> v);
	vector<O_TYPE> quantize_clustering(vector<RAW_DATA_TYPE> v);

private:
	bool quantizer_trained;
	vector<vector<RAW_DATA_TYPE> > training_observations;
	vector<RAW_DATA_TYPE> master_observation;
	Graph* plplot_graph;

};


#endif /* VECTTOSYMB_HPP_ */
