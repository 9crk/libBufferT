/***********************************************************************
Author: 9crk 2015-11-13 in ShenZhen
All right reserved.
mail:admin@9crk.com
***********************************************************************/
#include <sys/shm.h>  
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

#define TRUE 1
#define FALSE 0
typedef struct zfifo{
	unsigned int readIndex;
	unsigned int writeIndex;
	pthread_mutex_t mutex;
	char* buffer;
	unsigned int dataLen;
	int allowWrite;
}zfifo;

extern int zfifo_init(char** handle, unsigned int len);
extern int zfifo_destroy(char* handle);
extern int zfifo_read(char* handle, char* data, unsigned int datalen);
extern int zfifo_readEx(char* handle, char* data, unsigned int datalen);
extern int zfifo_write(char* handle, char* data, unsigned int length);
extern int zfifo_check(char *handle);
extern int zfifo_clear(char* handle);
extern int zfifo_clearA(char* handle);

int zfifo_init(char** handle, unsigned int len)
{
		zfifo *p = malloc(sizeof(zfifo));
		if(p==NULL){printf("TaiHenDa!!\n");}
		memset((void*)p, 0, sizeof(zfifo));
		p->buffer = (char*)malloc(len);
		if(p->buffer==NULL){printf("TaiHenDa!!\n");}
		p->dataLen = len;
		p->allowWrite = TRUE;
		*handle = (char*)p;
}
int zfifo_destroy(char* handle)
{
	free(((zfifo*)handle)->buffer);
	free(handle);
}
int zfifo_read(char* handle, char* data, unsigned int datalen)
{
	int readIndex,writeIndex;
	int ret;
	zfifo* circleBuf = (zfifo*)handle;
	pthread_mutex_lock(&circleBuf->mutex);
	
	readIndex = circleBuf->readIndex;
	writeIndex = circleBuf->writeIndex;
	
	if (readIndex < writeIndex){								
		if (readIndex + datalen < writeIndex){					//------------r---l---w-------------
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													//------------r------w----l--------------
			memcpy(data, circleBuf->buffer + readIndex, writeIndex - readIndex);
			circleBuf->readIndex = writeIndex;
			circleBuf->allowWrite = TRUE;
			//printf("no data...\n");
			ret = writeIndex - readIndex;
		}
	}else if (readIndex > writeIndex){
		if (readIndex + datalen < circleBuf->dataLen){					//----w----------------------r-----l----
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													
			if (readIndex + datalen - circleBuf->dataLen < writeIndex){//---l----w------------------------r----
				memcpy(data, circleBuf->buffer + readIndex, circleBuf->dataLen - readIndex);
				memcpy(data + circleBuf->dataLen - readIndex, circleBuf->buffer, readIndex + datalen - circleBuf->dataLen);		
				circleBuf->readIndex = readIndex + datalen - circleBuf->dataLen;
				ret = datalen;
			}else{											  //--w----l--------------------------r---
				//printf("no data...\n");			
				circleBuf->allowWrite = TRUE;
				memcpy(data, circleBuf->buffer + readIndex, circleBuf->dataLen - readIndex);
				memcpy(data + circleBuf->dataLen - readIndex, circleBuf->buffer, writeIndex);
				circleBuf->readIndex = writeIndex;
				ret = writeIndex + circleBuf->dataLen - readIndex;
			}
		}
	}else{													//-----------w==r--------------------------
		circleBuf->allowWrite = TRUE;
		ret = 0;
	}
	pthread_mutex_unlock(&circleBuf->mutex);
	return ret;
}
int zfifo_readEx(char* handle, char* data, unsigned int datalen)
{
	int readIndex,writeIndex;
	int ret;
	zfifo* circleBuf = (zfifo*)handle;
	pthread_mutex_lock(&circleBuf->mutex);
	
	readIndex = circleBuf->readIndex;
	writeIndex = circleBuf->writeIndex;
	
	if (readIndex < writeIndex){								
		if (readIndex + datalen < writeIndex){					//------------r---l---w-------------
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													//------------r------w----l--------------
			//memcpy(data, circleBuf->buffer + readIndex, writeIndex - readIndex);
			//circleBuf->readIndex = writeIndex;
			//circleBuf->allowWrite = TRUE;
			//printf("no data...\n");
			ret = 0;//writeIndex - readIndex;
		}
	}else if (readIndex > writeIndex){
		if (readIndex + datalen < circleBuf->dataLen){					//----w----------------------r-----l----
			memcpy(data, circleBuf->buffer + readIndex, datalen);
			circleBuf->readIndex += datalen;
			ret = datalen;
		}else{													
			if (readIndex + datalen - circleBuf->dataLen < writeIndex){//---l----w------------------------r----
				memcpy(data, circleBuf->buffer + readIndex, circleBuf->dataLen - readIndex);
				memcpy(data + circleBuf->dataLen - readIndex, circleBuf->buffer, readIndex + datalen - circleBuf->dataLen);		
				circleBuf->readIndex = readIndex + datalen - circleBuf->dataLen;
				ret = datalen;
			}else{											  //--w----l--------------------------r---
				//printf("no data...\n");			
				//circleBuf->allowWrite = TRUE;
				//memcpy(data, circleBuf->buffer + readIndex, circleBuf->dataLen - readIndex);
				//memcpy(data + circleBuf->dataLen - readIndex, circleBuf->buffer, writeIndex);
				//circleBuf->readIndex = writeIndex;
				ret = 0;//writeIndex + circleBuf->dataLen - readIndex;
			}
		}
	}else{													//-----------w==r--------------------------
		circleBuf->allowWrite = TRUE;
		ret = 0;
	}
	pthread_mutex_unlock(&circleBuf->mutex);
	return ret;
}
int zfifo_write(char* handle, char* data, unsigned int length)
{
	int writeIndex,readIndex;
	int ret=0;
	zfifo* circleBuf = (zfifo*)handle;
	pthread_mutex_lock(&circleBuf->mutex);
	
	writeIndex  = circleBuf->writeIndex;
	readIndex = circleBuf->readIndex;
	
	if (writeIndex >= readIndex){
		if (writeIndex == readIndex && circleBuf->allowWrite == FALSE){
			//printf("full, r==w\n");
			ret = 0;
		}
		if ((writeIndex + length) > circleBuf->dataLen){				
			if (writeIndex + length - circleBuf->dataLen < readIndex){		//----l--r-------------------w--
				memcpy(circleBuf->buffer + writeIndex, data, circleBuf->dataLen - writeIndex);
				memcpy(circleBuf->buffer, data + circleBuf->dataLen - writeIndex, writeIndex + length - circleBuf->dataLen);
				circleBuf->writeIndex = writeIndex + length - circleBuf->dataLen;
				ret = length;
			}else{													//---r--l---------------------w-
				//printf("full, w+l > r\n");
				ret = 0;
			}
		}else{														//-----r-----------------w----l--
			memcpy(circleBuf->buffer + writeIndex, data, length);
			circleBuf->writeIndex += length;
			ret = length;
		}
	}else if (writeIndex < readIndex){								
		if (writeIndex + length >= readIndex){						//------------w--r--l-----------应当丢失，报错，返回0
			//printf("full, w+l > r\n");
			ret = 0;
		}else{														//------------w---l---r----------正常，返回l
			memcpy(circleBuf->buffer + writeIndex, data, length);
			circleBuf->writeIndex += length;
			ret = length;
		}
	}
	pthread_mutex_unlock(&circleBuf->mutex);
	
	return ret;
}
int zfifo_check(char *handle)
{
	int len;
	zfifo* p = (zfifo*)handle;
	int write = p->writeIndex;
	int read = p->readIndex;
	if(write >= read)len = write - read;
	else{
		len = p->dataLen - (read - write);
	}
	return len;
}
int zfifo_clear(char* handle)
{
	zfifo* circleBuf = (zfifo*)handle;
	pthread_mutex_lock(&circleBuf->mutex);
	circleBuf->readIndex = 0;
	circleBuf->writeIndex = 0;
	circleBuf->allowWrite = TRUE;
	pthread_mutex_unlock(&circleBuf->mutex);
	return 0;
}
int zfifo_clearA(char* handle)
{
	zfifo* circleBuf = (zfifo*)handle;
	pthread_mutex_lock(&circleBuf->mutex);
	//read from writeIndex to readIndex while the data == 0x11111 then stop 
	int writepos = circleBuf->writeIndex;
	int readpos = circleBuf->readIndex;
	int all = circleBuf->dataLen;
	int i;
	char header[2];
	if(writepos >= readpos){
		for(i=writepos;i>(readpos+1);i--){
			header[0] = circleBuf->buffer[i-1];
			header[1] = circleBuf->buffer[i];
			if (header[0] == 0xFF && header[1]  == 0xF1){
				circleBuf->readIndex = i-1;
				break;
			}			
		}
	}else{
		for(i=writepos;i>1;i--){
			header[0] = circleBuf->buffer[i-1];
			header[1] = circleBuf->buffer[i];
			if (header[0] == 0xFF && header[1]  == 0xF1){
				circleBuf->readIndex = i-1;
				break;
			}
		}
		for(i=all;i>readpos+1;i--){
			header[0] = circleBuf->buffer[i-1];
			header[1] = circleBuf->buffer[i];
			if (header[0] == 0xFF && header[1]  == 0xF1){
				circleBuf->readIndex = i-1;
				break;
			}			
		}
	}
	circleBuf->allowWrite = TRUE;
	printf("clear = %02x %02x\n",circleBuf->buffer[circleBuf->readIndex],circleBuf->buffer[circleBuf->readIndex+1]);
	pthread_mutex_unlock(&circleBuf->mutex);
	return 0;
}
/*

int main(int argc, char* argv[])
{
	char* buff;
	char data[20];
	zfifo_init(&buff, 100000);
	zfifo_write(buff, "131213232424",10);
	zfifo_read(buff,data,9);
	data[19] = '\0';
	printf("%s",data);
	zfifo_destroy(buff);
}*/