/***********************************************************************
Author: 9crk 2015-11-13 in ShenZhen
All right reserved.
mail:admin@9crk.com
***********************************************************************/
#include "ringfifo.h"

frameRing::frameRing(int size)
{
	int i;
	fput = 0; /* 环形缓冲区的当前放入位置 */
	fget = 0; /* 缓冲区的当前取出位置 */
	fcount = 0; /* 环形缓冲区中的元素总数量 */

    for(i =0; i<NMAX; i++)
    {
        frameData[i].buffer = (unsigned char*)malloc(size);
				if(frameData[i].buffer==NULL){
					printf("no mem left!\n");
					return;
				}
        frameData[i].size = 0;
        frameData[i].frame_type = 0;
        //printf("FIFO INFO:idx:%d,len:%d,ptr:%x\n",i,frameData[i].size,(int)(frameData[i].buffer));
    }
    fput = 0; /* 环形缓冲区的当前放入位置 */
    fget = 0; /* 缓冲区的当前取出位置 */
    fcount = 0; /* 环形缓冲区中的元素总数量 */
	if(pthread_mutex_init(&fmutex,NULL) != 0 )  
    {  
	   printf("Init mutex %d error.",i);
    }  
}
int frameRing::nextOne(int thisOne)
{
	return (thisOne+1) == NMAX ? 0 : thisOne+1;
}
int frameRing::getFrame(struct frameBuf *getinfo)
{
	int Pos;
	pthread_mutex_lock(&fmutex);
    if(fcount>0)
    {
		Pos = fget;
        fget = nextOne(fget);
        fcount--;
        getinfo->buffer 	= 	(frameData[Pos].buffer);
        getinfo->frame_type = 	frameData[Pos].frame_type;
        getinfo->size 		= 	frameData[Pos].size;
        //printf("Get FIFO INFO:idx:%d,len:%d,ptr:%x,type:%d\n",Pos,getinfo->size,(int)(getinfo->buffer),getinfo->frame_type);
        pthread_mutex_unlock(&fmutex);
        return frameData[Pos].size;
    }
    else
    {
        printf("Buffer is empty\n");
        pthread_mutex_unlock(&fmutex);
        return 0;
    }
}
int frameRing::putFrame(unsigned char *buffer,int size,int encode_type)
{
	int ret;
	pthread_mutex_lock(&fmutex);
    if(fcount < NMAX)
    {
        memcpy(frameData[fput].buffer, buffer, size);
        frameData[fput].size= size;
        frameData[fput].frame_type = encode_type;
        //printf("Put FIFO INFO:idx:%d,len:%d,ptr:%x,type:%d\n",fput,frameData[fput].size,(int)(frameData[fput].buffer),frameData[fput].frame_type);
        fput = nextOne(fput);
        fcount++;
		ret = size;
	}
    else
    {
        printf("Buffer is full\n");
		ret = 0;
    }
	printf("--------------------%d\n",fcount);
	pthread_mutex_unlock(&fmutex);
	return  ret;
}

frameRing::~frameRing(void)
{
    int i;
    printf("free\n");
    for(i =0; i<NMAX; i++)
    {
       // printf("FREE FIFO INFO:idx:%d,len:%d,ptr:%x\n",i,frameData[i].size,(int)(frameData[i].buffer));
        free(frameData[i].buffer);
        frameData[i].size = 0;
    }
	pthread_mutex_destroy(&fmutex);
}
int frameRing::ringreset()
{
    fput = 0; /* 环形缓冲区的当前放入位置 */
    fget = 0; /* 缓冲区的当前取出位置 */
    fcount = 0; /* 环形缓冲区中的元素总数量 */
	printf("cleared!\n");
	return 0;
}