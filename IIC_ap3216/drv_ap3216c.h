#ifndef _AP3216C_H_
#define _AP3216C_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include "drv_common.h"

/* 次设备数量 */
#define MINOR_DEV_NUM   1
/* 字符设备的名称 */
#define CHAR_DEV_NAME   "iic_ap3216c"
/*  类名称 */
#define MY_CLASS_NAME   "my_iic_ap3216c"

#define I2C_SLAVE_NODE_NAME "my_ap3216c"

/* 申请缓存最大长度 */
#define BUF_LEN_MAX     6

/* 显式声明IIC的写标记, 其余标记已在i2c_msg结构体中定义，不需要再额外定义 */
#define IIC_WRITE   0


#define AP3216C_ADDR    	0X1E	/* AP3216C器件地址  */

/* AP3316C寄存器 */
#define AP3216C_SYSTEMCONG	0x00	/* 配置寄存器       */
#define AP3216C_INTSTATUS	0X01	/* 中断状态寄存器   */
#define AP3216C_INTCLEAR	0X02	/* 中断清除寄存器   */
#define AP3216C_IRDATALOW	0x0A	/* IR数据低字节     */
#define AP3216C_IRDATAHIGH	0x0B	/* IR数据高字节     */
#define AP3216C_ALSDATALOW	0x0C	/* ALS数据低字节    */
#define AP3216C_ALSDATAHIGH	0X0D	/* ALS数据高字节    */
#define AP3216C_PSDATALOW	0X0E	/* PS数据低字节     */
#define AP3216C_PSDATAHIGH	0X0F	/* PS数据高字节     */


typedef struct tagAp3216cAttr
{
    dev_t u32DevNum;    /* 总的字符设备号 */
    INT32U u32Major;    /* 主设备号 */
    INT32U u32Minor;    /* 次设备号, 暂时只考虑有一个次设备的情况，多个次设备的情况后面再考虑扩展 */
    struct class *pstMyClass;
    struct device *pstMyDevice;
    struct i2c_client *pstIicClient;
}AP3216C_ATTR_S;

/* 三个光传感器数据 */
typedef struct tagAp3216cInfo
{
    INT16U u16Ir;
    INT16U u16Als;
    INT16U u16Ps;
}AP3216C_INFO_S;

#endif