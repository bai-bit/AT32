#include <rtdevice.h>


#define SDA_PIN (9)
#define SCL_PIN (8)

static void at32_set_sda(void *data, rt_int32_t state)
{
    rt_pin_write(SDA_PIN, state);
}

static void at32_set_scl(void *data, rt_int32_t state)
{
    rt_pin_write(SCL_PIN, state);
}

static rt_int32_t at32_get_sda(void *data)
{
    return rt_pin_read(SDA_PIN);
}

static rt_int32_t at32_get_scl(void *data)
{
    return rt_pin_read(SCL_PIN);
}

static void at32_udelay(rt_uint32_t us)
{
	rt_int32_t i;
	for (; us > 0; us--)
	{
		i = 5;
		while (i > 0)
		{
			i--;
		}
	}
}


static const struct rt_i2c_bit_ops bit_ops = {
	RT_NULL,
	at32_set_sda,
	at32_set_scl,
	at32_get_sda,
	at32_get_scl,

	at32_udelay,

	1,
	100
};


int rt_i2c_init(const char *name)
{
	struct rt_i2c_bus_device *bus;

	bus = rt_malloc(sizeof(struct rt_i2c_bus_device));

	rt_memset((void *)bus, 0, sizeof(struct rt_i2c_bus_device));

	bus->priv = (void *)&bit_ops;

	rt_i2c_bit_add_bus(bus, name);

    rt_pin_mode(SCL_PIN, PIN_MODE_OUTPUT_OD); /* SCL */
    rt_pin_mode(SDA_PIN, PIN_MODE_OUTPUT_OD); /* SDA */
    
	return RT_EOK;
}
