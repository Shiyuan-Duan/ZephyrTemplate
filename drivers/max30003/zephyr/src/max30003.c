#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include "max30003.h"
#define SPIOP	SPI_WORD_SET(8)

struct max30003_data
{
    uint8_t chip_id;
};

struct max30003_config
{
    struct spi_dt_spec spi;
};


static int init_max30003(const struct device *dev)
{

    printk("Initializing MAX30003!\n");
    return 0;
}

#define MAX30003_CONFIG_SPI(inst) {.spi = SPI_DT_SPEC_INST_GET(inst, SPIOP, 0)}

#define MAX30003_DEFINE(inst)						\
	static struct max30003_data max30003_data_##inst;			\
	static const struct max30003_config max30003_config_##inst = MAX30003_CONFIG_SPI(inst);	\
	DEVICE_DT_INST_DEFINE(inst,			\
				init_max30003,							\
				NULL,							\
				&max30003_data_##inst,	\
				&max30003_config_##inst,\
				POST_KERNEL, \
				CONFIG_MAX30003_INIT_PRIORITY, \
				NULL);

/* STEP 5.2 - Create the struct device for every status "okay" node in the devicetree */
DT_INST_FOREACH_STATUS_OKAY(MAX30003_DEFINE)




