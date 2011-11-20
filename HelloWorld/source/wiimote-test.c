/*
 * wiimote-test.c
 *
 *  Created on: Nov 20, 2011
 *      Author: pom
 *      Base source from: http://wiibrew.org/wiki/How_to_use_the_Wiimote
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <wiiuse/wpad.h>

static GXRModeObj *rmode = NULL;

//-----------------------------------------------------------------------------------

int doreload=0, dooff=0;

void reload(void) {
	doreload=1;
}

void shutdown(void) {
	dooff=1;
}

void drawdot(void *xfb, GXRModeObj *rmode, float w, float h, float fx, float fy, u32 color) {
	u32 *fb;
	int px,py;
	int x,y;
	fb = (u32*)xfb;

	y = fy * rmode->xfbHeight / h;
	x = fx * rmode->fbWidth / w / 2;

	for(py=y-4; py<=(y+4); py++) {
		if(py < 0 || py >= rmode->xfbHeight)
				continue;
		for(px=x-2; px<=(x+2); px++) {
			if(px < 0 || px >= rmode->fbWidth/2)
				continue;
			fb[rmode->fbWidth/VI_DISPLAY_PIX_SZ*py + px] = color;
		}
	}

}

int evctr = 0;

void countevs(int chan, const WPADData *data) {
	evctr++;
}

int main_wiimote_test(int argc, char **argv) {
	int res;

	void *xfb[2];
	u32 type;
	int i;
	int fbi = 0;
	float theta;
	WPADData *wd;

	VIDEO_Init();
	PAD_Init();
	WPAD_Init();

	rmode = VIDEO_GetPreferredMode(NULL);

	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	SYS_SetResetCallback(reload);
	SYS_SetPowerCallback(shutdown);

	WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(0, rmode->fbWidth, rmode->xfbHeight);

	while(!doreload && !dooff) {
		CON_Init(xfb[fbi],0,0,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
		//VIDEO_ClearFrameBuffer(rmode,xfb[fbi],COLOR_BLACK);
		printf("\n\n\n");
		WPAD_ReadPending(WPAD_CHAN_ALL, countevs);
		res = WPAD_Probe(0, &type);
		switch(res) {
			case WPAD_ERR_NO_CONTROLLER:
				printf(" Wiimote not connected\n");
				break;
			case WPAD_ERR_NOT_READY:
				printf(" Wiimote not ready\n");
				break;
			case WPAD_ERR_NONE:
				printf(" Wiimote ready\n");
				break;
			default:
				printf(" Unknown Wimote state %d\n",res);

		}
		printf(" Event count: %d\n",evctr);
		if(res == WPAD_ERR_NONE) {
			wd = WPAD_Data(0);
			printf(" Data->Err: %d\n",wd->err);
			printf(" IR Dots:\n");
			for(i=0; i<4; i++) {
				if(wd->ir.dot[i].visible) {
					printf(" %4d, %3d\n", wd->ir.dot[i].rx, wd->ir.dot[i].ry);
				} else {
					printf(" None\n");
				}
			}
			if(wd->ir.valid) {
				printf(" Cursor: %.02f,%.02f\n",wd->ir.x, wd->ir.y);
				printf(" @ %.02f deg\n",wd->ir.angle);
			} else {
				printf(" No Cursor\n\n");
			}
			if(wd->ir.raw_valid) {
				printf(" Distance: %.02fm\n", wd->ir.z);
				printf(" Yaw: %.02f deg\n", wd->orient.yaw);
			} else {
				printf("\n\n");
			}
			printf(" Accel:\n");
			printf(" XYZ: %3d,%3d,%3d\n",wd->accel.x,wd->accel.y,wd->accel.z);
			printf(" Pitch: %.02f\n",wd->orient.pitch);
			printf(" Roll: %.02f\n",wd->orient.roll);
			printf(" Buttons down:\n ");
			if(wd->btns_h & WPAD_BUTTON_A) printf("A ");
			if(wd->btns_h & WPAD_BUTTON_B) printf("B ");
			if(wd->btns_h & WPAD_BUTTON_1) printf("1 ");
			if(wd->btns_h & WPAD_BUTTON_2) printf("2 ");
			if(wd->btns_h & WPAD_BUTTON_MINUS) printf("MINUS ");
			if(wd->btns_h & WPAD_BUTTON_HOME) printf("HOME ");
			if(wd->btns_h & WPAD_BUTTON_PLUS) printf("PLUS ");
			printf("\n ");
			if(wd->btns_h & WPAD_BUTTON_LEFT) printf("LEFT ");
			if(wd->btns_h & WPAD_BUTTON_RIGHT) printf("RIGHT ");
			if(wd->btns_h & WPAD_BUTTON_UP) printf("UP ");
			if(wd->btns_h & WPAD_BUTTON_DOWN) printf("DOWN ");
			printf("\n");
			for(i=0; i<4; i++) {
				if(wd->ir.dot[i].visible) {
					drawdot(xfb[fbi], rmode, 1024, 768, wd->ir.dot[i].rx, wd->ir.dot[i].ry, COLOR_YELLOW);
				}
			}
			if(wd->ir.raw_valid) {
				for(i=0; i<2; i++) {
					drawdot(xfb[fbi], rmode, 4, 4, wd->ir.sensorbar.rot_dots[i].x+2, wd->ir.sensorbar.rot_dots[i].y+2, COLOR_GREEN);
				}
			}
			if(wd->ir.valid) {
				theta = wd->ir.angle / 180.0 * 3.1415;
				drawdot(xfb[fbi], rmode, rmode->fbWidth, rmode->xfbHeight, wd->ir.x, wd->ir.y, COLOR_RED);
				drawdot(xfb[fbi], rmode, rmode->fbWidth, rmode->xfbHeight, wd->ir.x + 10*sinf(theta), wd->ir.y - 10*cosf(theta), COLOR_BLUE);
			}
			if(wd->btns_h & WPAD_BUTTON_1) doreload=1;
		}
		VIDEO_SetNextFramebuffer(xfb[fbi]);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		fbi ^= 1;
	}
	if(doreload) return 0;
	if(dooff) SYS_ResetSystem(SYS_SHUTDOWN,0,0);

	return 0;
}
