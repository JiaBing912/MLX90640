#include <rtthread.h>
#include <rtdevice.h>
#include "MLX90640_I2C_Driver.h"
#define MLX90640i2c_bus_NAME       "sci9i"
static struct rti2c_bus_device *i2c_bus = NULL;

/**
 *
*/
static int _mlx90640_i2cread(struct rti2c_bus_device *bus, uint8_t slave_addr, uint16_t start_addr, uint16_t num, uint16_t *data)
{
    uint8_t *bp;
    rt_ssize_t ret;
    uint8_t start[2];
    struct rt_i2c_msg msg[2];
#if 1
    bp = (uint8_t *)data;
    start[0] = (start_addr) >> 8;
    start[1] = (start_addr) & 0x00FF;
    
    msg[0].addr = slave_addr;
    msg[0].buf = start;
    msg[0].len = 2;
    msg[0].flags = RT_I2C_WR|RT_I2C_NO_STOP;
    msg[1].addr = slave_addr;
    msg[1].buf = bp;
    msg[1].len = num*sizeof(uint16_t);
    msg[1].flags = RT_I2C_RD;

    ret = rt_i2c_transfer(bus, msg, 2);
    if (ret != 2)
    {
        //rt_kprintf("err:%d != 2\r\n", ret);
        return -1;
    }

    for(int cnt=0; cnt < num*sizeof(uint16_t); cnt+=2) {
        uint8_t tmpbytelsb = bp[cnt+1];
        bp[cnt+1] = bp[cnt];
        bp[cnt] = tmpbytelsb;
    }
#else
    uint16_t trans_size;
    for(uint16_t off = 0; off < num;)
    {
        trans_size = num - off;
        if(trans_size > 0x10)
        {
            trans_size = 0x10;
        }
        bp = (uint8_t *)&data[off];
        start[0] = (start_addr + off) >> 8;
        start[1] = (start_addr + off) & 0x00FF;
        
        msg[0].addr = slave_addr;
        msg[0].buf = start;
        msg[0].len = 2;
        msg[0].flags = RT_I2C_WR|RT_I2C_NO_STOP;
        msg[1].addr = slave_addr;
        msg[1].buf = bp;
        msg[1].len = trans_size*sizeof(uint16_t);
        msg[1].flags = RT_I2C_RD;

        ret = rt_i2c_transfer(bus, msg, 2);
        if (ret != 2)
        {
            return -1;
        }

        for(int cnt=0; cnt < trans_size*sizeof(uint16_t); cnt+=2) {
            uint8_t tmpbytelsb = bp[cnt+1];
            bp[cnt+1] = bp[cnt];
            bp[cnt] = tmpbytelsb;
        }
        off += trans_size;
    }
#endif
    return 0;   
} 

static int _mlx90640_i2cwrite(struct rti2c_bus_device *bus, uint8_t slave_addr, uint16_t write_addr, uint16_t data)
{
    rt_ssize_t ret;
    uint8_t buf[4];
    uint16_t data_check;
    struct rt_i2c_msg msg[1];

    buf[0] = write_addr >> 8;
    buf[1] = write_addr & 0x00FF;
    buf[2] = data >> 8;
    buf[3] = data & 0x00FF;

    msg[0].addr = slave_addr;
    msg[0].buf = buf;
    msg[0].len = 4;
    msg[0].flags = RT_I2C_WR;

    ret = rt_i2c_transfer(bus, msg, 1);
    if (ret != 1)
    {
        return -1;
    }

    _mlx90640_i2cread(bus, slave_addr, write_addr, 1, &data_check);

    if ( data_check != data)
    {
        return -2;
    }    

    return 0;
}
int MLX90640_I2CRead(uint8_t slave_addr, uint16_t start_addr, uint16_t num, uint16_t *data)
{
    if(i2c_bus == NULL)
    {
        return -1;
    }
    return _mlx90640_i2cread(i2c_bus, slave_addr, start_addr, num, data);
}
int MLX90640_I2CWrite(uint8_t slave_addr, uint16_t write_addr, uint16_t data)
{
    if(i2c_bus == NULL)
    {
        return -1;
    }
    return _mlx90640_i2cwrite(i2c_bus, slave_addr, write_addr, data);
}


void iray_camera_init(const char *bus)
{		


	
}
static int iray_camera_cmd(int argc, char *argv[])
{
    if(rt_strcmp(argv[1], "start") == 0)
    {
        if(argc > 2)
        {
            const char *bus = argv[2];
            iray_camera_init(bus);
        }
    }

    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(iray_camera_cmd, iray_camera,iray camera);
