/***********************************************************************
Author: 9crk 2015-11-13 in ShenZhen
All right reserved.
mail:admin@9crk.com
***********************************************************************/
#ifndef __RINGFIFO_H_
#define __RINGFIFO_H_

#define NMAX 32
#include <pthread.h>
extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
	
}

typedef struct frameBuf {
    unsigned char *buffer;
	int frame_type;//0 ,I frame, 1 p frame
    int size;
}frameBuf;

class frameRing{
	public:
		int fput; /* 环形缓冲区的当前放入位置 */
		int fget; /* 缓冲区的当前取出位置 */
		int fcount; /* 环形缓冲区中的元素总数量 */
		frameBuf frameData[NMAX];
		pthread_mutex_t fmutex;
	
		frameRing(int size);
		int getFrame(struct frameBuf *getinfo);
		int putFrame(unsigned char *buffer,int size,int encode_type);//key frame or not 0: I   1: P
		int ringreset();//clear
		~frameRing();
	private:
		int nextOne(int thisOne);
};
//frameRing aRing(1024),vRing(400000);
#endif
