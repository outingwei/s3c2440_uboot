/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <s3c2410.h>

DECLARE_GLOBAL_DATA_PTR;

#define FCLK_SPEED 1

#if FCLK_SPEED==0		/* Fout = 203MHz, Fin = 12MHz for Audio */
#define M_MDIV	0xC3
#define M_PDIV	0x4
#define M_SDIV	0x1
#elif FCLK_SPEED==1		/* Fout = 202.8MHz */
#define M_MDIV	0xA1
#define M_PDIV	0x3
#define M_SDIV	0x1
#endif

#define S3C2440_MPLL_400MHZ ((0x5c<<12)|(0x10<<4)|(0x01))
#define S3C2440_UPLL_48MHZ  ((0x38<<12)|(0X02<<4)|(0X02))
#define S3C2440_CLKDIV      0x05

#define S3C2410_MPLL_200MHZ ((0x5c<<12)|(0x04<<4)|(0x00))
#define S3C2410_UPLL_48MHZ  ((0x28<<12)|(0X01<<4)|(0X02))
#define S3C2410_CLKDIV      0x03

#define USB_CLOCK 1

#if USB_CLOCK==0
#define U_M_MDIV	0xA1
#define U_M_PDIV	0x3
#define U_M_SDIV	0x1
#elif USB_CLOCK==1
#define U_M_MDIV	0x48
#define U_M_PDIV	0x3
#define U_M_SDIV	0x2
#endif

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	
	gpio->GPACON = 0x007FFFFF;
	gpio->GPBCON = 0X00044555;
	gpio->GPBUP  = 0X000007FF;
	gpio->GPCCON = 0XAAAAAAAA;
	gpio->GPCUP  = 0C0000FFFF;
	gpio->GPDCON = 0XAAAAAAAA;
	gpio->GPDUP  = 0X0000FFFF;
	gpio->GPECON = 0XAAAAAAAA;
	gpio->GPEUP  = 0X0000FFFF;
	gpio->GPFCON = 0X000055AA;
	gpio->GPFUP  = 0X000000FF;
	gpio->GPFCON = 0XFF95FFBA;
	gpio->GPGUP  = 0X0000FFFF;
	gpio->GPHCON = 0X002AFAAA;
	gpio->GPHUP  = 0X000007FF;

	if((gpio->GSTATUS1 == 0X32410000) || (gpio->GSTATUS1 == 0X32410002))
	{
		clk_power->CLKDIVN = S3C2410_CLKDIV;
		__asm__("mrc p15, 0, r1, c1, c0, 0\n"
				"orr r1, r1, #0xc0000000\n"
				"mcr p15, 0, r1, c1, c0, 0\n"
				:::"r1"
		);
		clk_power->LOCKTIME = 0XFFFFFF;
		clk_power->MPLLCON  = S3C2410_MPLL_200MHZ; 

		delay (4000);
		clk_power->UPLLCON = S3C2410_UPLL_48MHZ;

		delay(8000);

		gd->bd->bi_arch_number = MACH_TYPE_SMDK2410;
		
	}else{
		clk_power->CLKDIVN = S3C2440_CLKDIV;
		__asm__("mrc p15, 0, r1, c1, c0, 0\n"
				"orr r1, r1, #0xc0000000\n"
				"mcr p15, 0, r1, c1, c0, 0\n"
				:::"r1"
		);
		clk_power->LOCKTIME = 0XFFFFFF;
		clk_power->MPLLCON  = S3C2440_MPLL_400MHZ; 

		delay (4000);
		clk_power->UPLLCON = S3C2440_UPLL_48MHZ;

		delay(8000);

		gd->bd->bi_arch_number = MACH_TYPE_SMDK2410;

	}


	/* set up the I/O ports */
	/*
	gpio->GPACON = 0x007FFFFF;
	gpio->GPBCON = 0x00044555;
	gpio->GPBUP = 0x000007FF;
	gpio->GPCCON = 0xAAAAAAAA;
	gpio->GPCUP = 0x0000FFFF;
	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;
	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;
	gpio->GPFCON = 0x000055AA;
	gpio->GPFUP = 0x000000FF;
	gpio->GPGCON = 0xFF95FFBA;
	gpio->GPGUP = 0x0000FFFF;
	gpio->GPHCON = 0x002AFAAA;
	gpio->GPHUP = 0x000007FF;*/



	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}
