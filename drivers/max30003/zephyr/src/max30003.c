#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "max30003.h"


#define PRINT_BIT(val, n) ((val & BIT(n)) ? '1' : '0')
#define SPIOP	SPI_WORD_SET(8)


LOG_MODULE_REGISTER(MAX30003, LOG_LEVEL_DBG);
struct max30003_data
{
    uint8_t chip_id;
};

struct max30003_config
{
    struct spi_dt_spec spi;
};


static int max30003_read_reg(const struct device *dev, uint8_t reg, uint8_t *data, int size)
{
    int err;
    uint8_t tx_buffer[4]; 
    tx_buffer[0] = (reg << 1) | 0x01;
    tx_buffer[1] = 0x00;
    tx_buffer[2] = 0x00;
    tx_buffer[3] = 0x00;



    struct spi_buf tx_spi_buf = {.buf = (void *)&tx_buffer, .len = sizeof(tx_buffer)};
    struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
    struct spi_buf rx_spi_buf = {.buf = data, .len = size};
    struct spi_buf_set rx_spi_buf_set = {.buffers = &rx_spi_buf, .count=1};

    const struct max30003_config *cfg = dev->config;
    
    err = spi_transceive_dt(&cfg->spi, &tx_spi_buf_set, &rx_spi_buf_set);
    if (err < 0) {
        printk("spi_transceive_dt() failed, err: %d", err);
        return err;
    }

    return 0;

}



static int max30003_write_reg(const struct device *dev, uint8_t reg, uint8_t *values, size_t size)
{
	int err;
    const struct max30003_config *cfg = dev->config;
    

	/* Bit7 is 0 for the write command */
	uint8_t tx_buf[size+1];
    tx_buf[0] = (reg << 1);
    memcpy(&tx_buf[1], values, size);

	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = sizeof(tx_buf)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	err = spi_write_dt(&cfg->spi, &tx_spi_buf_set);
	if (err < 0) {
		LOG_ERR("spi_write_dt() failed, err %d", err);
		return err;
	}

	return 0;
}

static int check_info(const struct device *dev)
{
    int err;
    uint8_t data[4];
    err = max30003_read_reg(dev, INFO, data, sizeof(data));

    if (err<0){
        LOG_ERR("Error reading max30003 reg\n");
        return err;
    }

    if ((data[1]>>4) != 0b0101)
    {
        LOG_ERR("Error communicating to MAX30003, expecting 0b0101 but got 0x%x\n", data[1]>>4);
        return -1;
    }
    LOG_INF("INFO REVID: 0x%x\n", data[1]);
    return 0;
}

static int print_status(const struct device *dev)
{
    int err;
    uint8_t data[4];
    err = max30003_read_reg(dev, STATUS, data, sizeof(data));
    if (err < 0){
        LOG_ERR("Error print status\n");
        return err;
    }
    LOG_INF("Printing status:\n");
    LOG_INF("EINT : %c\n", PRINT_BIT(data[1], 7));
    LOG_INF("EOVF : %c\n", PRINT_BIT(data[1], 6));
    LOG_INF("FSTINT : %c\n", PRINT_BIT(data[1], 5));
    LOG_INF("DCLOFF_INT : %c\n", PRINT_BIT(data[1], 4));
    LOG_INF("********************\n");
    LOG_INF("LONINT : %c\n", PRINT_BIT(data[2], 3));
    LOG_INF("RRINT : %c\n", PRINT_BIT(data[2], 2));
    LOG_INF("SAMP : %c\n", PRINT_BIT(data[2], 1));
    LOG_INF("PLLINT : %c\n", PRINT_BIT(data[2], 0));



    return 0;
}

static int print_cnfg(const struct device *dev)
{
    int err;
    uint8_t data[4];
    err = max30003_read_reg(dev, CNFG_GEN, data, sizeof(data));
    if (err < 0){
        LOG_ERR("Error print cnfg\n");
        return err;
    }
    LOG_INF("Printing configs:\n");
    LOG_INF("data[0]:0x%x, data[1]:0x%x, data[2]:0x%x, data[3]:0x%x\n", data[0], data[1], data[2], data[3]);
    LOG_INF("EN_ULP_LON : %c%c\n", PRINT_BIT(data[1], 7),PRINT_BIT(data[1], 6));
    LOG_INF("FMSTR : %c%c\n", PRINT_BIT(data[1], 5), PRINT_BIT(data[1], 4));
    LOG_INF("EN_ECG : %c\n", PRINT_BIT(data[1], 3));


    LOG_INF("RBIASV: %c%c\n", PRINT_BIT(data[3], 3), PRINT_BIT(data[3], 2));
    LOG_INF("********************\n");


    return 0;
}

static int max30003_reset(const struct device *dev)
{
    int err;
    uint8_t data[3] = {0, 0, 0};
    err = max30003_write_reg(dev, SW_RST, data, 3);
    if (err<0){
        LOG_ERR("Error in writing to SW_RST\n");
    }
    return 0;
}


static int init_max30003(const struct device *dev)
{   

    int err;
    max30003_reset(dev);
    check_info(dev);
    print_status(dev);
    // print_cnfg(dev);


    
    uint8_t cnfg_gen_buf[4];
    err = max30003_read_reg(dev, CNFG_GEN, cnfg_gen_buf, sizeof(cnfg_gen_buf));
    if (err < 0){
        LOG_ERR("Error reading cnfg_gen\n");
        return err;
    }


    // CNFG_GEN: Enable ECG
    cnfg_gen_buf[1] = cnfg_gen_buf[1] | BIT(3);
    // CNFG_GEN: FMSTR (Default 00)

    // CNFG_GEN: EN_RBIAS
    cnfg_gen_buf[4] = cnfg_gen_buf[4] | BIT(4);

    // CNFG_GEN: RBIASP/N
    cnfg_gen_buf[4] = (cnfg_gen_buf[4] | BIT(0)) | BIT(1);

    max30003_write_reg(dev, CNFG_GEN, &cnfg_gen_buf[1], 3);


    print_cnfg(dev);


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




