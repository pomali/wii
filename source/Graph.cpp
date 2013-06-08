/*
 * Graph.cpp
 *
 *  Created on: Mar 19, 2013
 *      Author: pom
 */

#include "Graph.hpp"


using namespace std;


Graph::Graph(){
		//inicializacia

		for(int i=0; i<GRAPH_LEN; i++)
			pls_tArray[i] = i;

		pls_graphPos = 0;
		bX = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		bY = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		bZ = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		cX = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		cY = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		cZ = boost::circular_buffer<PLFLT>(GRAPH_LEN);
		az3d = 45.0;
		alt3d = 45.0;

		pls = new plstream();
		pls->sdev("xwin");
//		pls->sdev("wxwidgets");
//		pls->sdev("xcairo");
//		pls->setopt("drvopt", "smooth=0"); //iba pre wxWidgets
		pls->setopt("nb","");
		pls->setopt("db","");
		pls->schr(0,0.4);
		plspause( 0 );
		pls->init();
		pls->col0(7);
		pls->ssub(1,2);

//			int pli;
//			for (pli=0;pli<200;pli++){
//						this->p((double)(pli*1),(double)(pli*2),(double)(pli*3));
//			}


	if (true);
	pls->env(0,500,0,500,0,-2);
	pls->ptex( 100, 100, 0, 0, 0, "Connect Wiimote (press 1+2)" );


}

Graph::~Graph(){
	delete pls;
}


void Graph::p(double x, double y, double z){
	bX.push_back(x);
	bY.push_back(y);
	bZ.push_back(z);

	pls->bop();
	/*
	 * XYZ GRAPH
	 */
//	pls->adv(1);
	pls->col0(7);
	pls->env0(0,GRAPH_LEN,-255.0,255,0,0);

	pls->col0(1);
	pls->line(GRAPH_LEN, pls_tArray, bX.linearize());

	pls->col0(2);
	pls->line(GRAPH_LEN, pls_tArray, bY.linearize());

	pls->col0(3);
	pls->line(GRAPH_LEN, pls_tArray, bZ.linearize());

	/*
	 *  3D GRAPH
	 */
/*
	pls->adv(2);
	pls->col0(7);
	pls->env0(-255, 255, -255, 255, 0, -2);
	pls->w3d(255.0, 255.0, 255.0, 0.0, 255.0, 0.0, 255.0, 0.0, 255.0, 45.0, 45.0);
	pls->box3("bnstu","X",0.0, 0, "bnstu", "Y",0.0,0,"bcdmnstv","Z",0.0,0);
	pls->col0(1);
	pls->line3(GRAPH_LEN, bX.linearize(), bY.linearize(), bZ.linearize());
	PLFLT **zd;
	PLFLT ad[1] = {0};
	PLFLT clevel[1] = {0};
	pls->plAlloc2dGrid( &zd, 1, 1);
	c_plot3d(bX.linearize(),bY.linearize(),zd, 1, 1, 1, 1);
	c_plot3d(bX.linearize(),bY.linearize(),zd, 1, 1, 1, 1);
	c_plot3d(bX.linearize(),bY.linearize(),zd, 1, 1, 1, 1);
	pls->meshc(bX.linearize(),bY.linearize(),zd, 1, 1, 1, clevel,1);
*/


/*
	//X Graph
	pls->adv(3);
	pls->col0(7);
	pls->env0(0,GRAPH_LEN,0,255,0,0);
	pls->col0(1);
	pls->line(GRAPH_LEN, pls_tArray, bX.linearize());


	//Y graph
	pls->adv(4);
	pls->col0(7);
	pls->env0(0,GRAPH_LEN,0,255,0,0);
	pls->col0(2);
	pls->line(GRAPH_LEN, pls_tArray, bY.linearize());

	//Z GRAPH
	pls->adv(5);
	pls->col0(7);
	pls->env0(0,GRAPH_LEN,0,255,0,0);
	pls->col0(3);
	pls->line(GRAPH_LEN, pls_tArray, bY.linearize());
*/
	pls->eop();


}

void Graph::d3d(){

	float k = 45.0;
	pls->bop();
	/*
	 *  3D GRAPH
	 */

	//pls->adv(2);
	pls->col0(7);
	pls->env0(-1*k, k, -1*k, k, 0, -2);
	pls->w3d(k, k, k, -1*k, k, -1*k, k, -1*k, k, alt3d, az3d);
	pls->box3("bnstu","X",0.0, 0, "bnstu", "Y",0.0,0,"bcdmnstv","Z",0.0,0);
	pls->col0(1);
	pls->poin3(cX.size(), cX.linearize(), cY.linearize(), cZ.linearize(),88);
	PLFLT **zd;
	//PLFLT ad[1] = {0};
	PLFLT clevel[1] = {0};
	pls->plAlloc2dGrid( &zd, 1, 1);
	c_plot3d(cX.linearize(),cY.linearize(),zd, 1, 1, 1, 1);
	c_plot3d(cX.linearize(),cY.linearize(),zd, 1, 1, 1, 1);
	c_plot3d(cX.linearize(),cY.linearize(),zd, 1, 1, 1, 1);
	pls->meshc(cX.linearize(),cY.linearize(),zd, 1, 1, 1, clevel,1);

	pls->eop();

}

void Graph::c3d_rotate_right(){
	az3d = fmod((az3d+5.0), 360);
	d3d();
}

void Graph::c3d_rotate_left(){
	az3d = fmod((az3d+715.0), 360);
	d3d();
}

void Graph::c3d_rotate_up(){
	alt3d = fmod((alt3d+2.0), 90);
	d3d();
}

void Graph::c3d_rotate_down(){
	alt3d = fmod((alt3d+178.0), 90);
	d3d();
}

void Graph::c(double x, double y, double z){
	cX.push_back(x);
	cY.push_back(y);
	cZ.push_back(z);

	d3d();

}

/*
 * prepise celym c polia ktore sa vykresluju a vykresli zmenu
 *
 *
 */

void Graph::clear_c(){
	cX.clear();
	cY.clear();
	cZ.clear();
}




