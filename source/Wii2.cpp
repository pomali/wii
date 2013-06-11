/*
 * Wii2.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: pom
 */

#include "Wii2.h"
#define MAX_WIIMOTES 1
#define WM_MACRO wiimote_t* wm = this->wiimotes[0];

using namespace std;


Wii2::Wii2() {
	// TODO Auto-generated constructor stub

}

int Wii2::init()
{
	this->wiimotes =  wiiuse_init(MAX_WIIMOTES);


	this->found = wiiuse_find(wiimotes, MAX_WIIMOTES, 10);
	if (!this->found) {
		std::cout<<"No wiimotes found."<<std::endl;
		return 0;
	}

	this->connected = wiiuse_connect(this->wiimotes, MAX_WIIMOTES);
	if (this->connected)
		cout<<"Connected: "<<this->connected<<"/"<<this->found << endl;
	else {
		cout<<"Failed to connect to any wiimote."<<endl;
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

int Wii2::main(){
	if (!this->init())
			exit(0);

	wm = this->wiimotes[0];
	int should_exit = 0;
	while(!should_exit){
		if (wiiuse_poll(this->wiimotes, MAX_WIIMOTES) && wm->event == WIIUSE_EVENT){ //zatial len pre 1 wiimote
					if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_HOME)){
						should_exit++;
						cout<<"bb"<<endl;
					}

					if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_PLUS)){
								cout<<"training!"<<endl;
								this->record();
					}
		}
	}
	return 0;
}

int Wii2::record(){
	WM_MACRO ;
	vector<int> data;
	vector<int> labels;
	int should_stop = 0;
	while(!should_stop){
		if (wiiuse_poll(this->wiimotes, MAX_WIIMOTES) && wm->event == WIIUSE_EVENT){ //zatial len pre 1 wiimote
			if (IS_JUST_PRESSED(wm,WIIMOTE_BUTTON_HOME)){
				should_stop++;
				cout<<"stopping training"<<endl;
			}

			data.push_back(this->v3c2symb(wm->accel));
			if(IS_PRESSED(wm,WIIMOTE_BUTTON_A))
				labels.push_back(1);
			else
				labels.push_back(0);
		}
	}


    ofstream os ("training_data.dat", ios::binary);
    int size1 = data.size();
	os.write((const char*)&size1, sizeof(int));
	os.write((const char*)&data[0], size1 * sizeof(int));
	os.close();

	os.open("training_labels.dat", ios::binary);
	size1 = labels.size();
	os.write((const char*)&size1, sizeof(int));
	os.write((const char*)&labels[0], size1 * sizeof(int));
	os.close();

	/*
	 * ako ich precitat naspat:
	 *
    ifstream is("training_data.dat", ios::binary);
    int size2;
    is.read((char*)&size2, sizeof(int));
    data.resize(size2);
    is.read((char*)&data[0], size2 * sizeof(int));

    is.open("training_labels.dat", ios::binary);
    int size2;
    is.read((char*)&size2, sizeof(int));
    labels.resize(size2);
    is.read((char*)&labels[0], size2 * sizeof(int));
	 */

	BOOST_FOREACH(int i, data)
		cout<<i<<" ";
	cout<<endl;
	BOOST_FOREACH(int i, labels)
		cout<<i<<" ";
	cout<<endl;
	return 0;
}

int Wii2::v3c2symb(vec3b_t v){
//	cout<<(int)v.x<<" "<<(int)v.y<<" "<<(int)v.z<<endl;
	int o=0;
	if(v.x>122)
		o+=1;
	if(v.y>122)
		o+=2;
	if(v.z>122)
		o+=4;
	return o;
}

Wii2::~Wii2() {
	// TODO Auto-generated destructor stub
}
