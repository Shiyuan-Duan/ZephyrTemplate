/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>

#include "ble.h"
#include <max30003.h>

LOG_MODULE_REGISTER(MAXBLE, LOG_LEVEL_INF);

#define SPIOP	SPI_WORD_SET(8) | SPI_TRANSFER_MSB

const struct gpio_dt_spec ledspec = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(max30003));

static const struct gpio_dt_spec max30003_interrupt = GPIO_DT_SPEC_GET(DT_NODELABEL(max30003), int_gpios);
static uint32_t dummy_data = 100;

static void simulate_data(void)
{
	dummy_data++;
	if (dummy_data == 200) {
		dummy_data = 100;
	}
}






static int on_button_pressed(void){
	max30003_print_cnfg(spi_dev);
	return 0;
}

struct ble_cb callbacks = {
	.dbg_cb = on_button_pressed,
};

void on_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // printk("Interrupt triggered!!!! \n");

	// max30003_fifo_read(spi_dev, NULL);

}
static struct gpio_callback cb_data;
int main(void)
{
	printk("Initializing....\n");
	
	LOG_INF("MAX30003 Interrupt GPIO Info:");
	LOG_INF("Device name: %s", max30003_interrupt.port->name);
	LOG_INF("Pin number: %d", max30003_interrupt.pin);
	LOG_INF("Flags: 0x%x", max30003_interrupt.dt_flags);

	gpio_pin_configure_dt(&max30003_interrupt, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&max30003_interrupt, GPIO_INT_EDGE_FALLING);
	gpio_init_callback(&cb_data, on_interrupt, BIT(max30003_interrupt.pin));
	gpio_add_callback(max30003_interrupt.port, &cb_data);

	max30003_synch_ecg(spi_dev);
	while (1){

		max30003_fifo_read(spi_dev, NULL);



		// Populate your array



		struct max30003_data *maxdata = spi_dev->data;
		int32_t ecg_buffer[32]={0};
		memcpy(ecg_buffer, maxdata->ecg_data, 128);
		for(int i=0; i<32; i++){
			printk("hex: %x ECG_VALUE: %d\n", ecg_buffer[i],ecg_buffer[i]);
		}
		
		int err = stream_sensor_data(ecg_buffer, 128);

		// printk("Error #: %d\n", err);
		k_msleep(30);
		
		
	}
	return 0;
}