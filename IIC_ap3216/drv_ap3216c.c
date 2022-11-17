#include "drv_ap3216c.h"


/* 函数声明 */
static int Ap3216cProbe(struct i2c_client *pstI2cClient, const struct i2c_device_id *pstI2cDeviceId);
static int Ap3216cRemove(struct i2c_client *pstI2cClient);

static int MyAp3216cOpen(struct inode *pinode, struct file *pfile);
static int MyAp3216cRelease(struct inode *pinode, struct file *pfile);
static ssize_t MyAp3216cRead(struct file *pfile, char __user *pcUserBuf, size_t ul32Length, loff_t *poffset);

static AP3216C_ATTR_S gstAp3216cNodeInfo;

const struct of_device_id of_match_table[] = 
{
    {.compatible = "my_ap3216c"},
    { /* Sentinel */ }
};

static struct i2c_device_id ap3216c_id[] = 
{
    {"my_ap3216c", 0},
    {}
};

static struct i2c_driver gstAp3216cIICDrv =
{
    .probe  = Ap3216cProbe,
    .remove = Ap3216cRemove,
    .driver =
    {
        .owner  = THIS_MODULE,
        .name   = I2C_SLAVE_NODE_NAME,
        .of_match_table = of_match_table,
    },
    .id_table = ap3216c_id, /* 必须要加id_table这个成员变量，否则无法执行到probe函数 */
};

static struct file_operations my_fops = 
{
    .open       = MyAp3216cOpen,
    .release    = MyAp3216cRelease,
    .read       = MyAp3216cRead,
};

static struct cdev gstAp3216cDev = 
{
    .owner = THIS_MODULE,
};

/**********************************************************************
 * 函数名称：GetAp3216cNodeAttr
 * 函数描述：获取AP3216C设备基本信息
 * 参数说明：无
 * 返回结果：返回AP3216C设备基本属性信息.
 * 特殊说明：无
***********************************************************************/
static AP3216C_ATTR_S *GetAp3216cNodeAttr(VOID)
{
    return &gstAp3216cNodeInfo;
}

/**********************************************************************
 * 函数名称：WriteAp3216cRegs
 * 函数描述：向AP3216C寄存器中写入信息
 * 参数说明：AP3216C_ATTR_S *pstAttr  AP3216C节点属性信息
 *         INT8U u8RegAddr           AP3216C寄存器首地址
 *         INT8U *pu8Buf             待写入信息
 *         INT32U u32Length          写入长度,字节
 * 返回结果：成功返回DRV_SUCCESS; 失败返回DRV_FAILED.
 * 特殊说明：无
***********************************************************************/
static INT32U WriteAp3216cRegs(AP3216C_ATTR_S *pstAttr, INT8U u8RegAddr, INT8U *pu8Buf, INT32U u32Length)
{
    INT32S s32Ret;
    INT8U au8SendBuffer[BUF_LEN_MAX];   /* 保存要通过IIC发给AP3216C的所有内容，包括 寄存器首地址 + 写入的内容 */
    INT32S s32MsgNum = 1;               /* 要发送的iic msg的数量，默认就一条消息 */
    struct i2c_msg stI2cMsg;
    struct i2c_client *pstClient = pstAttr->pstIicClient;

    CHECK_VALID(pstAttr);
    CHECK_VALID(pstAttr->pstIicClient);
    
    if(u32Length > BUF_LEN_MAX - 1)
    {
        KER_PRINT("invalid length[%u] > max[%u]\n", u32Length, (BUF_LEN_MAX - 1));
        return DRV_FAILED;
    }

    /* 构建要发送的IIC信息，从机寄存器首地址 + 待写入的内容 */
    au8SendBuffer[0] = u8RegAddr;
    memcpy(&au8SendBuffer[1], pu8Buf, u32Length);

    /* 构建IIC msg，并发送 */
    stI2cMsg.addr   = pstClient->addr;
    stI2cMsg.flags  = IIC_WRITE;
    stI2cMsg.buf    = &au8SendBuffer[0];
    stI2cMsg.len    = u32Length + 1;    /* 加1是因为还有从机寄存器首地址的一个字节 */
    s32Ret = i2c_transfer(pstClient->adapter, &stI2cMsg, s32MsgNum);
    if(s32Ret < 0)
    {
        KER_PRINT("iic write err\n");
        return DRV_FAILED;
        /* 这里可以看下源代码i2c_imx_xfer，返回值只有 小于0 和 s32MsgNum两种情况，表示成功或失败*/
    }

    return DRV_SUCCESS;
}

/**********************************************************************
 * 函数名称：ReadAp3216cRegs
 * 函数描述：从AP3216C寄存器中读取信息
 * 参数说明：AP3216C_ATTR_S *pstAttr  AP3216C节点属性信息
 *         INT8U u8RegAddr           AP3216C寄存器首地址
 *         INT8U *pu8Buf             保存读取到的信息
 *         INT32U u32Length          读取长度,字节
 * 返回结果：成功返回DRV_SUCCESS; 失败返回DRV_FAILED.
 * 特殊说明：无
***********************************************************************/
static INT32U ReadAp3216cRegs(AP3216C_ATTR_S *pstAttr, INT8U u8RegAddr, INT32U u32Length, INT8U *pu8Buf)
{
    INT32S s32Ret;
    INT32S s32MsgNum = 2;       /* 通过IIC读取从机信息时，要发送的iic msg的数量，默认需要2条消息 */
    struct i2c_msg astMsg[2];   /* 从从机读取寄存器信息时，要发送两条消息，首先发送待读取寄存器的首地址，
                                    这是发送一条写消息，再开始从寄存器读取数据，这是是读消息 */
    struct i2c_client *pstClient = pstAttr->pstIicClient;

    CHECK_VALID(pstAttr);
    CHECK_VALID(pstAttr->pstIicClient);

    /* 构建I2C msg */
    /* msg0，发送要读取打从机寄存器首地址的消息 */
    astMsg[0].addr  = pstClient->addr;
    astMsg[0].flags = IIC_WRITE;
    astMsg[0].buf   = &u8RegAddr;
    astMsg[0].len   = 1;

    astMsg[1].addr  = pstClient->addr;
    astMsg[1].flags = I2C_M_RD; /* 读取信息标志 */
    astMsg[1].buf   = pu8Buf;
    astMsg[1].len   = u32Length;

    s32Ret = i2c_transfer(pstClient->adapter, &astMsg[0], s32MsgNum);
    if(s32Ret < 0)
    {
        KER_PRINT("iic write err\n");
        return DRV_FAILED;
        /* 这里可以看下源代码i2c_imx_xfer，返回值只有 小于0 和 s32MsgNum两种情况，表示成功或失败*/
    }

    return DRV_SUCCESS;
}

/**********************************************************************
 * 函数名称：ReadAp3216cOneReg
 * 函数描述：从AP3216C一个寄存器中读取信息
 * 参数说明：AP3216C_ATTR_S *pstAttr  AP3216C节点属性信息
 *         INT8U u8RegAddr           AP3216C寄存器地址
 *         INT8U *pu8Buf             待写入信息
 * 返回结果：成功返回DRV_SUCCESS; 失败返回DRV_FAILED.
 * 特殊说明：无
***********************************************************************/
static INT32U ReadAp3216cOneReg(AP3216C_ATTR_S *pstAttr, INT8U u8RegAddr, INT8U *pu8Value)
{
    return ReadAp3216cRegs(pstAttr, u8RegAddr, 1, pu8Value);
}

/**********************************************************************
 * 函数名称：WriteAp3216cOneReg
 * 函数描述：向AP3216C一个寄存器中写入信息
 * 参数说明：AP3216C_ATTR_S *pstAttr  AP3216C节点属性信息
 *         INT8U u8RegAddr           AP3216C寄存器地址
 *         INT8U *pu8Buf             待写入信息
 * 返回结果：成功返回DRV_SUCCESS; 失败返回DRV_FAILED.
 * 特殊说明：无
***********************************************************************/
static INT32U WriteAp3216cOneReg(AP3216C_ATTR_S *pstAttr, INT8U u8RegAddr, INT8U u8Value)
{
    return WriteAp3216cRegs(pstAttr, u8RegAddr, &u8Value, 1);
}

static int MyAp3216cOpen(struct inode *pinode, struct file *pfile)
{
    AP3216C_ATTR_S *pstAttr = GetAp3216cNodeAttr();

    /* 初始化AP3216C */
	WriteAp3216cOneReg(pstAttr, AP3216C_SYSTEMCONG, 0x04);		/* 复位AP3216C 			*/
	mdelay(50);														/* AP3216C复位最少10ms 	*/
	WriteAp3216cOneReg(pstAttr, AP3216C_SYSTEMCONG, 0X03);		/* 开启ALS、PS+IR 		*/
    
    printk("open Ap3216c Dev success\n");
    return 0;
}

static int MyAp3216cRelease(struct inode *pinode, struct file *pfile)
{
    printk("release Ap3216c Dev success\n");
    return 0;
}

static ssize_t MyAp3216cRead(struct file *pfile, char __user *pcUserBuf, size_t ul32Length, loff_t *poffset)
{
    INT32U u32Ret = 0;
    INT32U i = 0;
    INT8U au8RecvBuf[BUF_LEN_MAX];
    AP3216C_ATTR_S *pstAttr = NULL;
    AP3216C_INFO_S stAp3216cInfo;

    pstAttr = GetAp3216cNodeAttr();

    /* 从AP3216C寄存器中读取数据 */
    for(i = 0; i < BUF_LEN_MAX; i++)
    {
        u32Ret |= ReadAp3216cOneReg(pstAttr, AP3216C_IRDATALOW + i, &au8RecvBuf[i]);
    }
    if(DRV_SUCCESS != u32Ret)
    {
        KER_PRINT("read ap3216c reg fail, ret[%u]\n", u32Ret);
        return DRV_FAILED;
    }

    /* 拼接数据 */
    if(au8RecvBuf[0] & 0X80) 	/* IR_OF位为1,则数据无效 */
		stAp3216cInfo.u16Ir = 0;					
	else 				/* 读取IR传感器的数据   		*/
		stAp3216cInfo.u16Ir = ((unsigned short)au8RecvBuf[1] << 2) | (au8RecvBuf[0] & 0X03); 			
	
	stAp3216cInfo.u16Als = ((unsigned short)au8RecvBuf[3] << 8) | au8RecvBuf[2];	/* 读取ALS传感器的数据 			 */  
	
    if(au8RecvBuf[4] & 0x40)	/* IR_OF位为1,则数据无效 			*/
		stAp3216cInfo.u16Ps = 0;    													
	else 				/* 读取PS传感器的数据    */
		stAp3216cInfo.u16Ps = ((unsigned short)(au8RecvBuf[5] & 0X3F) << 4) | (au8RecvBuf[4] & 0X0F); 

    /* AP3216C寄存器数据拷贝至用户空间 */
    (void)copy_to_user(pcUserBuf, &stAp3216cInfo, sizeof(stAp3216cInfo));

    return DRV_SUCCESS;
}

/**********************************************************************
 * 函数名称：AllocDevNum
 * 函数描述：申请字符设备号
 * 参数说明：INT32U Major 主设备号, 若为0, 则动态申请字符设备号;否则申请指定打主设备号
 * 返回结果：成功返回DRV_SUCCESS; 失败返回DRV_FAILED.
 * 特殊说明：无
***********************************************************************/
static INT32U AllocDevNum(INT32U Major)
{
    INT32S s32Ret;
    dev_t u32DevNum = 0;
    AP3216C_ATTR_S *pstAp3216cInfo = GetAp3216cNodeAttr();

    if(0 == Major)
    {
        /* 主设备号为0, 动态申请字符设备号 */
        s32Ret = alloc_chrdev_region(&u32DevNum, MY_MINOR_BASE, MINOR_DEV_NUM, CHAR_DEV_NAME);
        if(DRV_SUCCESS != s32Ret)
        {
            printk("alloc dev num dynamic fail,ret[%d].\n", s32Ret);
            return DRV_FAILED;
        }
    }
    else
    {
        /* 主设备号不为0, 申请制定打字符设备号 */
        u32DevNum = MKDEV(Major, MY_MINOR_BASE);

        s32Ret = register_chrdev_region(u32DevNum, MINOR_DEV_NUM, CHAR_DEV_NAME);
        if(DRV_SUCCESS != s32Ret)
        {
            printk("alloc dev num[%u] fail,ret[%d].\n", Major, s32Ret);
            return DRV_FAILED;
        }
    }
    
    /* 保存设备号 */
    pstAp3216cInfo->u32DevNum  = u32DevNum;
    pstAp3216cInfo->u32Major   = MAJOR(u32DevNum);
    pstAp3216cInfo->u32Minor   = MINOR(u32DevNum);

    printk("allocate dev num success, DevNum[%u], Major[%u], Minor[%u].\n", pstAp3216cInfo->u32DevNum, pstAp3216cInfo->u32Major, pstAp3216cInfo->u32Minor);
    return DRV_SUCCESS;
}

static INT32U MyAp3216cDevInit(void)
{
    INT32S s32Ret = DRV_SUCCESS;
    AP3216C_ATTR_S *pstAp3216cInfo = GetAp3216cNodeAttr();

    /* 1、字符设备初始化 */
    cdev_init(&gstAp3216cDev, &my_fops);

    /* 2、向系统添加字符设备 */
    s32Ret = cdev_add(&gstAp3216cDev, pstAp3216cInfo->u32DevNum, MINOR_DEV_NUM);
    if(DRV_SUCCESS != s32Ret)
    {
        printk("add char dev fail.\n");
        goto ERROR1;
    }

    /* 3、创建类class */
    pstAp3216cInfo->pstMyClass = class_create(THIS_MODULE, MY_CLASS_NAME);
    if(NULL == pstAp3216cInfo->pstMyClass)
    {
        printk("create class[%s] fail.\n", MY_CLASS_NAME);
        goto ERROR2;
    }

    /* 4、创建设备节点 */
    pstAp3216cInfo->pstMyDevice = device_create(pstAp3216cInfo->pstMyClass, NULL, pstAp3216cInfo->u32DevNum, NULL, CHAR_DEV_NAME);
    if(NULL == CHAR_DEV_NAME)
    {
        printk("create device node fail.\n");
        goto ERROR3;
    }

    return DRV_SUCCESS;

ERROR3:
    class_destroy(pstAp3216cInfo->pstMyClass);

ERROR2:
    cdev_del(&gstAp3216cDev);

ERROR1:
    unregister_chrdev_region(pstAp3216cInfo->u32DevNum, MINOR_DEV_NUM);
    return DRV_FAILED;
}

static void MyAp3216cDevDeInit(void)
{
    AP3216C_ATTR_S *pstAp3216cInfo = GetAp3216cNodeAttr();

    /* 销毁设备节点 */
    device_destroy(pstAp3216cInfo->pstMyClass, pstAp3216cInfo->u32DevNum);

    /* 删除类class */
    class_destroy(pstAp3216cInfo->pstMyClass);

    /* 删除添加的设备 */
    cdev_del(&gstAp3216cDev);

    return;
}


static int Ap3216cProbe(struct i2c_client *pstI2cClient, const struct i2c_device_id *pstI2cDeviceId)
{
    INT32U u32Ret;
    AP3216C_ATTR_S *pstAp3216cInfo = GetAp3216cNodeAttr();

    /* 参考正点原子, 保存iic_client信息 */
    pstAp3216cInfo->pstIicClient = pstI2cClient;
    
    /* 1、申请字符设备号 */
    u32Ret = AllocDevNum(0);
    if(DRV_SUCCESS != u32Ret)
    {
        printk("alloc char dev num fail.\n");
        return DRV_FAILED;
    }

    /* 2、字符设备初始化 */
    u32Ret = MyAp3216cDevInit();
    if(DRV_SUCCESS != u32Ret)
    {
        printk("led dev init fail.\n");
        goto ERR1;
    }

    printk("my ap3216c probe success\n");
    return DRV_SUCCESS;

ERR1:
    unregister_chrdev_region(pstAp3216cInfo->u32DevNum, MINOR_DEV_NUM);
    pstAp3216cInfo->pstIicClient = NULL;
    return DRV_FAILED;
}

static int Ap3216cRemove(struct i2c_client *pstI2cClient)
{
    AP3216C_ATTR_S *pstAp3216cInfo = GetAp3216cNodeAttr();

    /* Ap3216c字符设备反初始化 */
    MyAp3216cDevDeInit();
    
    /* 释放之前申请的设备号 */
    unregister_chrdev_region(pstAp3216cInfo->u32DevNum, MINOR_DEV_NUM);

    printk("my ap3216c dev node remove.\n");
    return DRV_SUCCESS;
}

/* 加载KO模块 */
static int __init MyAp3216cInit(void)
{
    INT32S s32Ret;
    
    s32Ret = i2c_add_driver(&gstAp3216cIICDrv);
    if(DRV_SUCCESS != s32Ret)
    {
        KER_PRINT("add iic drv ap3216c fail\n");
        return DRV_FAILED;
    }

    KER_PRINT("add iic drv ap3216c success.\n");
    return DRV_SUCCESS;
}

/* 卸载KO模块 */
static void __exit MyAp3216cExit(void)
{
    i2c_del_driver(&gstAp3216cIICDrv);
    printk("my ap3216c drv unregister.\n");
    return ;
}

module_init(MyAp3216cInit);
module_exit(MyAp3216cExit);

MODULE_LICENSE("GPL");
