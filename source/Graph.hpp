/*
 * Graph.hpp
 *
 *  Created on: Mar 19, 2013
 *      Author: pom
 */



#ifndef GRAPH_HPP_
#define GRAPH_HPP_

#include <plplot/plstream.h>
#include <boost/circular_buffer.hpp>
#include <math.h>

#ifndef GRAPH_LEN
#define GRAPH_LEN 100
#endif

#define GRAPH_BOX_STR "bcinst"

class Graph {


private:
	plstream *pls;
	PLFLT pls_tArray[GRAPH_LEN];
	boost::circular_buffer<PLFLT> bX;
	boost::circular_buffer<PLFLT> bY;
	boost::circular_buffer<PLFLT> bZ;

	boost::circular_buffer<PLFLT> cX;
	boost::circular_buffer<PLFLT> cY;
	boost::circular_buffer<PLFLT> cZ;
	PLFLT az3d;
	PLFLT alt3d;

	int pls_graphPos;


public:
   Graph();
   ~Graph();
   void p(double x, double y, double z);
   void c(double x, double y, double z);
   void c3d_rotate_right();
   void c3d_rotate_left();
   void c3d_rotate_up();
   void c3d_rotate_down();
   void clear_c();
   void d3d();


};



#endif /* GRAPH_HPP_ */
