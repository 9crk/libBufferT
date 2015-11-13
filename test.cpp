/***********************************************************************
Author: 9crk 2015-11-13 in ShenZhen
All right reserved.
mail:admin@9crk.com
***********************************************************************/
#include<stdio.h>

#include"ringfifo.h"

int main(int argc, char* argv[])
{
	frameRing myRing(100);
	unsigned char data[10];
	while(1)
	{
		scanf("%s\n",&data);
		if(data[0]=='q')break;
		myRing.putFrame(data,10,0);
	}
}