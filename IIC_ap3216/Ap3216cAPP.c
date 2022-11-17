#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: ap3216cApp.c
作者	  	: 左忠凯
版本	   	: V1.0
描述	   	: ap3216c设备测试APP。
其他	   	: 无
使用方法	 ：./ap3216cApp /dev/ap3216c
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/9/20 左忠凯创建
***************************************************************/

#define FILE_PATH   "/dev/iic_ap3216c"

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(void)
{
	int fd;
	unsigned short databuf[3];
	unsigned short ir, als, ps;
	int ret = 0;




	fd = open(FILE_PATH, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", FILE_PATH);
		return -1;
	}

	while (1) {
		ret = read(fd, databuf, sizeof(databuf));
		if(ret == 0) { 			/* 数据读取成功 */
			ir =  databuf[0]; 	/* ir传感器数据 */
			als = databuf[1]; 	/* als传感器数据 */
			ps =  databuf[2]; 	/* ps传感器数据 */
			printf("ir = %d, als = %d, ps = %d\r\n", ir, als, ps);
		}
		usleep(200000); /*100ms */
	}
	close(fd);	/* 关闭文件 */	
	return 0;
}