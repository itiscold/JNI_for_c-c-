//ʹ�ñ��ļ���Ϊc�س�������jni�Ľӿڣ������޸�ԭc�ļ�
#include "JNITest.h"
#include "control.h"

JNIEXPORT jint JNICALL Java_JNITest_getGear
  (JNIEnv *env, jclass clazz, jobject c, jobject w){
	ctrlInfo ctrl;
	waysideInfo way;
	int8_t  res;
	jint *p ;
	jsize len;
	jintArray data_array;
	int i;
	
	jclass cClass = (*env)->FindClass(env,"Control");
	jclass wClass = (*env)->FindClass(env,"Wayside");
	jfieldID vField = (*env)->GetFieldID(env,cClass,"v","I");
	jfieldID posField = (*env)->GetFieldID(env,cClass,"pos","I");
	jfieldID ebvField = (*env)->GetFieldID(env,cClass,"ebv","I");
	jfieldID cmdvField = (*env)->GetFieldID(env,cClass,"cmdv","I");
	jfieldID slopeField = (*env)->GetFieldID(env,wClass,"slope","I");
	jfieldID speedField = (*env)->GetFieldID(env,wClass,"speed","I");
	jfieldID dataField = (*env)->GetFieldID(env,wClass,"data","[I");


	ctrl.v = (int32_t)(*env)->GetIntField(env,c,vField);
	ctrl.ebv = (int32_t)(*env)->GetIntField(env,c,ebvField);
	ctrl.pos = (int32_t) (*env)->GetIntField(env,c,posField);
	ctrl.cmdv = (int32_t)(*env)->GetIntField(env,c,cmdvField);

	way.slope = (int32_t)(*env)->GetIntField(env,w,slopeField);
	way.speed = (int32_t)(*env)->GetIntField(env,w,speedField);
	//ȡ������
	data_array = (jintArray)(*env)->GetObjectField(env,w,dataField);
	//��java����ת����c����
	p = (*env)->GetIntArrayElements(env,data_array,NULL);
	//��ȡ���鳤��
	len = (*env)->GetArrayLength(env,data_array);
	for(i = 0;i<len;i++){
		way.data[i] = p[i];
	}
	res = getLevel(&ctrl,&way);
	//�ͷ��������
	(*env)->ReleaseIntArrayElements(env,data_array,p,0); 
	(*env)->SetIntField(env,c,vField,10);
	return (jint)res;
}

/*
extern ctrlInfo c;
extern waysideInfo w;

void main(){
	ctrlInfo tmpc;
	waysideInfo tmpw;
	tmpc.cmdv=120;
	tmpc.ebv= 130;
	tmpc.v= 110;
	tmpw.data[0] = 1;
	tmpw.data[1] = 2;
	tmpw.data[2] = 3;
	tmpw.slope = -22;
	tmpw.speed  = 130;
	update_ctrl_info(&tmpc);
	update_wayside_info(&tmpw);
	printf("ret = %d  \r\n",getLevel(&c,&w));
}
*/


