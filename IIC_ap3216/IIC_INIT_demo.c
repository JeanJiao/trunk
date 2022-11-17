/****************************************************************************************************************
 * 文件说明：IIC从设备驱动程序初始化模板，以AP3216C驱动程序为例
 * 
 * 驱动初始化流程：
 *  (1)、加载KO模块, 会调用到I2cNodeInit函数, 在此函数中注册IIC从设备驱动
 *  (2)、注册从设备驱动前, 需先将IIC驱动需要的几个结构体变量定义并填好：
 *          struct i2c_driver gstAp3216cIICDrv
 *          struct i2c_device_id ap3216c_id[]       这个结构体不能少, 否则dev和drv不匹配, 执行不到probe函数
 *          struct of_device_id of_match_table[]
 *  (3)、设备dev和驱动drv匹配成功后, 执行probe函数, 在probe函数中, 申请设备号, 创建字符设备节点
 *  (4)、编写open, release, write, read的函数的具体实现
 * 
 * 
 * 驱动模块卸载流程：
 *  (5)、rmmod KO, 调用I2cNodeExit函数, 删除之前注册的IIC从设备驱动
 *  (6)、删除之前注册的驱动时, 会调用remove函数, 卸载创建字符设备时申请的资源
 * 
 * 
 * 补充说明：
 * 1、(1)和(2)之间是相互联系的, 注册驱动前, 需要先定义并初始化好这几个结构体变量, 因此既可以先定义好几个结构体变量, 再注册;
 *      也可以先注册IIC驱动, 根据函数的入参需要, 定义并填充相应的结构体变量。
 * 3、必须要创建一个字符设备节点, 因为这是内核空间和用户空间交互的通道, 但不一定必须在probe函数中创建, 可以在其他步骤中创建, 
 *      比如在加载模块调用I2cNodeInit函数时, 就在I2cNodeInit函数中创建也可以。
 *      同理, (6)中释放资源时, 也不是必须调用remove函数时才释放, 可以在I2cNodeExit函数中释放资源
 * 4、编写程序时, 在创建字符设备节点时, 需要定义以及填充struct file_operations my_fops结构体变量, 在创建设备节点时, 可以
 *      先把MyAp3216cOpen, MyAp3216cRelease, MyAp3216cRead等这几个函数的函数原型框架先写好, 创建字符设备节点的函数写完
 *      后, 在编写open, release, write, read对应的MyAp3216cOpen, MyAp3216cRelease, MyAp3216cRead 这几个函数的具体实
 *      现, 包括实现从设备初始化、通过IIC读写从设备寄存器信息的功能等。
****************************************************************************************************************/


/* 4、编写实现具体的open, release, write, read等函数的具体实现 */
static int MyAp3216cOpen(struct inode *pinode, struct file *pfile)
{
}

static int MyAp3216cRelease(struct inode *pinode, struct file *pfile)
{
}

static ssize_t MyAp3216cRead(struct file *pfile, char __user *pcUserBuf, size_t ul32Length, loff_t *poffset)
{
}



/* 3、设备与驱动匹配后，系统执行probe函数，初始化资源；卸载KO时，会执行remove函数释放资源 */
static int Ap3216cProbe(struct i2c_client *pstI2cClient, const struct i2c_device_id *pstI2cDeviceId)
{
	/* probe函数中的任务，一般按照之前的流程，申请设备号，创建一个字符设备节点 */
	/* 在创建字符设备节点时，需要会用到 struct file_operations my_fops 结构体变量，
		这个变量中的函数，一般是先把open, release, write, read等函数原型框架先定义好，
		后面再编写具体函数内容 */
	/* 需要说明的是，在probe函数中创建字符设备节点，只是驱动的一种流程，不一定非要在probe函数中创建，
		可以在加载模块，执行INIT函数时就创建，但是一定要创建字符设备节点，因为这是内核与用户空间交互
		数据的一个通道*/
}


static int Ap3216cRemove(struct i2c_client *pstI2cClient)
{
	/* 释放创建设备节点时，申请的资源 */
}

/* 2、填充结构体变量，包括i2c_driver，of_device_id，i2c_device_id */
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

/* 1、模块加载、卸载 */

/* 加载KO模块 */
static int __init I2cNodeInit(void)
{
	i2c_add_driver(&gstAp3216cIICDrv);

    KER_PRINT("add iic drv ap3216c success.\n");
    return DRV_SUCCESS;

}

/* 卸载KO模块 */
static void __exit I2cNodeExit(void)
{
    i2c_del_driver(&gstAp3216cIICDrv);

	KER_PRINT("my ap3216c drv unregister.\n");
    return ;

}

module_init(I2cNodeInit);
module_exit(I2cNodeExit);
MODULE_LICENSE("GPL");