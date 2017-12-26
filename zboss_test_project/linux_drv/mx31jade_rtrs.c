/*
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *  Copyright 2005-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *		 09/09/2010 	 rachel_lo
 * 				 Register MCU3_22 as power wake up interrupt.
 *		 09/20/2010 	 rachel_lo
 * 				 Remove the MCU3_22 since the touch interrupt is used as power wake up source.
 */
 

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memory.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/smsc911x.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <mach/common.h>
#include <asm/page.h>
#include <asm/setup.h>
#include "iomux.h"
#include <mach/board-mx31jade_rtrs.h>
#include <mach/imx-uart.h>
#include <mach/iomux-mx3.h>
#include <mach/irqs.h>
#include <mach/mxc_nand.h>
#include "devices.h"
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

/*
 * This file contains the board-specific initialization routines.
 */
/*create DM9003 initial function
 *begin from #if defined CONFIG_DM9003
 *end at #endif
*/
#if defined(CONFIG_DM9003) || defined(CONFIG_DM9003_MODULE)
static struct resource dm9003_resources[] = {
         [0] = {
                 .start  = DM9003_BASE_ADDRESS ,
/*because of the dm9003 use the memory map from 0 to 0xff of CS4*/
                 .end    = (DM9003_BASE_ADDRESS + 0xff),
                 .flags  = IORESOURCE_MEM,
         },
         [1] = {
                 .start  = IOMUX_TO_IRQ(MX31_PIN_GPIO3_0) ,
                 .end    = IOMUX_TO_IRQ(MX31_PIN_GPIO3_0) ,
                 .flags  = IORESOURCE_IRQ,
         },
         [2] = {
		 		 .start  = GPIO3_BASE_ADDR + 0x14,
                 .end    = (GPIO3_BASE_ADDR + 0x14 + 0x8),
                 .flags  = IORESOURCE_MEM,
         },
};
         
static struct platform_device dm9003_device = {
         .name           = "dm9003", 
         .id             = 0,
         .num_resources  = ARRAY_SIZE(dm9003_resources),
         .resource       = dm9003_resources,
};

static void jade_init_eth(void)
{
	 /* init irq gpio */
        printk(KERN_INFO "<<jade>> Irq init for eth0\n");
        mxc_request_iomux(MX31_PIN_GPIO3_0, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
		gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_GPIO3_0));
	 	//enable_irq(IOMUX_TO_IRQ(MX31_PIN_GPIO3_0));
        (void)platform_device_register(&dm9003_device);        
}
#else

static void jade_init_eth(void)
{
}

#endif /* end of CONFIG_DM9003  */

static struct i2c_board_info mx31jads_i2c_devices[] = {
#if defined(CONFIG_MXC_ADC_TOUCHSCREEN_HEALTH_I2C) || defined(CONFIG_MXC_ADC_TOUCHSCREEN_HEALTH_I2C_MODULE)
	{
		I2C_BOARD_INFO("pic_i2c", PIC_I2C_ADDRESS),
		.type = "pic_i2c",
	},
#endif
#if defined(CONFIG_TOUCHSCREEN_AR7603) || defined(CONFIG_TOUCHSCREEN_AR7603_MODULE) || defined(CONFIG_TOUCHSCREEN_AR7603_2) || defined(CONFIG_TOUCHSCREEN_AR7603_2_MODULE)
	{															
		I2C_BOARD_INFO("ar7603", AR7603_I2C_ADDRESS),			
		.type = "ar7603",								
	},												
#endif
#if defined(CONFIG_TOUCHSCREEN_ZT2083) || defined(CONFIG_TOUCHSCREEN_ZT2083_MODULE)
	{
		I2C_BOARD_INFO("zt2083", ZT2083_I2C_ADDRESS),
		.type = "zt2083",
	},
#endif
#if defined(CONFIG_RTC_DRV_RX8564LC) || defined(CONFIG_RTC_DRV_RX8564LC_MODULE)
	{
		I2C_BOARD_INFO("rx8564", RX8564_I2C_ADDRESS),
		.type = "rx8564",
	},
#endif
#if defined(CONFIG_SND_IMX31_SOC_JADE) || defined(CONFIG_SND_IMX31_SOC_JADE_MODULE)
	{															
		I2C_BOARD_INFO("alc5620", ALC5620_I2C_ADDRESS),			
		.type = "alc5620",								
	},												
#endif
};


/* 
 * Create I2C Touch Controller ar7603 resources and init function
 */
#if defined(CONFIG_TOUCHSCREEN_AR7603) || defined(CONFIG_TOUCHSCREEN_AR7603_MODULE)
static struct resource ar7603_resources[] = {
         [0] = {
                 .start  = AR7603_GPIO2_IMR_ADDRESS,
                 .end    = (AR7603_GPIO2_IMR_ADDRESS + 0x8),
                 .flags  = IORESOURCE_MEM,
         },
         [1] = {
                 .start  = IOMUX_TO_IRQ(MX31_PIN_SVEN0) ,
                 .end    = IOMUX_TO_IRQ(MX31_PIN_SVEN0) ,
                 .flags  = IORESOURCE_IRQ,
         },

};
static struct platform_device ar7603_device = {

         .name           = "ar7603", 
         .id             = 0,
         .num_resources  = ARRAY_SIZE(ar7603_resources),
         .resource       = ar7603_resources,
};

static void jade_init_ar7603(void)
{
	 /* init irq gpio */
    printk(KERN_INFO "<<JADE>> ar7603 irq init \n");
	mxc_request_iomux(MX31_PIN_SVEN0, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_SVEN0));
	enable_irq(IOMUX_TO_IRQ(MX31_PIN_SVEN0));
    (void)platform_device_register(&ar7603_device);
}
#endif


/*
 * Create I2C Touch Controller ar7603(2.On Transfer board) resources and init function
*/
#if defined(CONFIG_TOUCHSCREEN_AR7603_2) || defined(CONFIG_TOUCHSCREEN_AR7603_2_MODULE)
static struct resource ar7603_resources[] = {
         [0] = {
                 .start  = AR7603_2_GPIO2_IMR_ADDRESS,
                 .end    = (AR7603_2_GPIO2_IMR_ADDRESS + 0x8),
                 .flags  = IORESOURCE_MEM,
         },
         [1] = {
                 .start  = IOMUX_TO_IRQ(MX31_PIN_BATT_LINE) ,
                 .end    = IOMUX_TO_IRQ(MX31_PIN_BATT_LINE) ,
                 .flags  = IORESOURCE_IRQ,
         },

};
static struct platform_device ar7603_device = {

         .name           = "ar7603", 
         .id             = 0,
         .num_resources  = ARRAY_SIZE(ar7603_resources),
         .resource       = ar7603_resources,
};

static void jade_init_ar7603_2(void)
{
	/* init irq gpio */
	printk("<<JADE>> AR7603_2 irq init \n");
	mxc_request_iomux(MX31_PIN_BATT_LINE, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);	
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_BATT_LINE));
	enable_irq(IOMUX_TO_IRQ(MX31_PIN_BATT_LINE));	
    (void)platform_device_register(&ar7603_device);
}
#endif


/*
 * Create I2C Touch Controller ar7603 resources and init function
 */
#if defined(CONFIG_TOUCHSCREEN_ZT2083) || defined(CONFIG_TOUCHSCREEN_ZT2083_MODULE)
static struct resource zt2083_resources[] = {
         [0] = {
                 .start  = ZT2083_GPIO2_IMR_ADDRESS,
                 .end    = (ZT2083_GPIO2_IMR_ADDRESS + 0x8),
                 .flags  = IORESOURCE_MEM,
         },
         [1] = {
                 .start  = IOMUX_TO_IRQ(MX31_PIN_SVEN0) ,
                 .end    = IOMUX_TO_IRQ(MX31_PIN_SVEN0) ,
                 .flags  = IORESOURCE_IRQ,
         },

};
static struct platform_device zt2083_device = {

         .name           = "zt2083",
         .id             = 0,
         .num_resources  = ARRAY_SIZE(zt2083_resources),
         .resource       = zt2083_resources,
};

static void jade_init_zt2083(void)
{
	 /* init irq gpio */
    printk(KERN_INFO "<<JADE>> zt2083 irq init \n");
	mxc_request_iomux(MX31_PIN_SVEN0, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_SVEN0));
    (void)platform_device_register(&zt2083_device);
}
#endif


#if defined(CONFIG_MXC_ADC_HEALTH) || defined(CONFIG_MXC_ADC_HEALTH_MODULE)
/* Health Monitor */
static struct resource jade_health_resource[] = {
	[0] = {
			.start = HEALTH_ADDRESS,
			.end = HEALTH_ADDRESS,
			.flags = IORESOURCE_MEM,
	      },
};

static struct platform_device jade_health_device = {
	.name = "jade-health",
	.id = 0,
	.num_resources = ARRAY_SIZE(jade_health_resource),
	.resource = jade_health_resource,
};


static void jade_init_healthmonitor(void)
{
	printk(KERN_INFO "<<jade>> Health Monitoring init\n");
	(void) platform_device_register(&jade_health_device);
}

/* Health Monitor */
#endif

#if defined(CONFIG_SPI_ZIGBEE_TESTING) || defined(CONFIG_SPI_ZIGBEE_TESTING_MODULE)
static struct spi_board_info mxc_spi_board_info[] __initdata = {
	{
	 .modalias = "zigbee",
	 .irq = IOMUX_TO_IRQ(MX31_PIN_SD_D_CLK),
	 .max_speed_hz = 3000000,
	 .bus_num = 2,		
	 .chip_select = 1,					
	 .mode = SPI_MODE_2,						
	 },
};

static void jade_init_zigbee(void)
{
	 /* init irq gpio */
    printk(KERN_INFO "<<JADE>> zigbee reset pin init \n");
	//"reset" GPIO pin setup - set GPO1 pin to high 
	mxc_request_iomux(MX31_PIN_SRXD4, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_output(IOMUX_TO_GPIO(MX31_PIN_SRXD4), 0);
	udelay(250);
	//"interrupt" GPIO pin setup - Input pin
	mxc_request_iomux(MX31_PIN_SD_D_CLK, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_SD_D_CLK));
	//"wake" pin setup - set GPO2 pin to low
	mxc_request_iomux(MX31_PIN_GPIO1_1, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_output(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1), 1);
}
#endif

#if defined(CONFIG_SPI_SPIDEV)

static struct spi_board_info mxc_spi_board_info[] __initdata = {
	{
	 .modalias = "spidev",
	 .irq = IOMUX_TO_IRQ(MX31_PIN_SD_D_CLK),
	 .max_speed_hz = 3000000,
	 .bus_num = 2,		
	 .chip_select = 1,					
	 .mode = SPI_MODE_2,						
	 },
};

static void jade_init_zigbee(void)
{
	 /* init irq gpio */
    printk(KERN_INFO "<<JADE>> zigbee/spidev reset pin init \n");
	//"reset" GPIO pin setup - set GPO1 pin to high 
	mxc_request_iomux(MX31_PIN_SRXD4, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_output(IOMUX_TO_GPIO(MX31_PIN_SRXD4), 0);
	udelay(250);
	//"interrupt" GPIO pin setup - Input pin
	mxc_request_iomux(MX31_PIN_SD_D_CLK, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_SD_D_CLK));
	//"wake" pin setup - set GPO2 pin to low
	mxc_request_iomux(MX31_PIN_GPIO1_1, OUTPUTCONFIG_GPIO,INPUTCONFIG_GPIO);
	gpio_direction_output(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1), 1);
}

#endif


void __init jade_rtrs_init(void)
{

#if defined(CONFIG_DM9003) || defined(CONFIG_DM9003_MODULE)
	jade_init_eth();
#endif

	i2c_register_board_info(BUS_NUMBER, mx31jads_i2c_devices, ARRAY_SIZE(mx31jads_i2c_devices));

#if defined(CONFIG_TOUCHSCREEN_AR7603) || defined(CONFIG_TOUCHSCREEN_AR7603_MODULE)
	jade_init_ar7603();
#endif

#if defined(CONFIG_TOUCHSCREEN_AR7603_2) || defined(CONFIG_TOUCHSCREEN_AR7603_2_MODULE)
	jade_init_ar7603_2();
#endif

#if defined(CONFIG_TOUCHSCREEN_ZT2083) || defined(CONFIG_TOUCHSCREEN_ZT2083_MODULE)
	jade_init_zt2083();
#endif

#if defined(CONFIG_MXC_ADC_HEALTH) || defined(CONFIG_MXC_ADC_HEALTH_MODULE)
	jade_init_healthmonitor();
#endif


#if defined(CONFIG_SPI_ZIGBEE_TESTING) || defined(CONFIG_SPI_ZIGBEE_TESTING_MODULE) || defined(CONFIG_SPI_SPIDEV)
	spi_register_board_info(mxc_spi_board_info, ARRAY_SIZE(mxc_spi_board_info));
	jade_init_zigbee();
#endif


}

