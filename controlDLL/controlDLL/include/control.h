#ifndef _CONTROL_H_
#define _CONTROL_H_

typedef signed char int8_t;
typedef short int  int16_t;
typedef int int32_t;

typedef struct{
	int32_t slope;
	int32_t speed;
	int8_t data[4];
}waysideInfo;

typedef struct{
	int32_t v;
	int32_t pos;
	int32_t ebv;
	int32_t cmdv;
}ctrlInfo;

void update_ctrl_info(ctrlInfo* ctrl );
void update_wayside_info(waysideInfo* wayside);
int8_t getLevel(ctrlInfo* ctrl,waysideInfo* wayside);

#endif