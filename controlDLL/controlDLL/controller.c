#include <stdio.h>
#include "control.h"

ctrlInfo c = {0,0,0,0};
waysideInfo w; 

void update_ctrl_info(ctrlInfo* ctrl){
	c.cmdv = ctrl->cmdv;
	c.ebv = ctrl->ebv;
	c.pos = ctrl->pos;
	c.v = ctrl->v;
}
void update_wayside_info(waysideInfo *wayside){
	w.data[0] = wayside->data[0];
	w.data[1] = wayside->data[1];
	w.data[2] = wayside->data[2];
	w.data[3] = wayside->data[3];
	w.slope = wayside->slope;
	w.speed = wayside->speed;
}
int8_t getLevel(ctrlInfo* ctrl,waysideInfo* wayside){
	int8_t ret = 0;
	update_ctrl_info(ctrl);
	update_wayside_info(wayside);
	ret = (c.v - w.speed)%10
		;
	printf("in get level: %d\r\n",w.data[0]);
	return ret;
}