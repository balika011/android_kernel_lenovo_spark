/*
 *  intel_mid_vibra_acpi.c - Intel vibrator driver
 *
 *  Copyright (C) 2013 Intel Corp
 *  Author: B, Jayachandran <jayachandran.b@intel.com>
 *  Author: Omair Md Abdullah <omair.m.abdullah@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#include <linux/acpi.h>
#include <linux/acpi_gpio.h>
#include <linux/mfd/intel_mid_pmic.h>
#include <linux/input/intel_mid_vibra.h>
#include "mid_vibra.h"

#define CRYSTALCOVE_PMIC_PWM_EN_GPIO_REG        0x2F
/* PWM enable gpio register settings: drive type = CMOS; pull disabled */
#define CRYSTALCOVE_PMIC_PWM_EN_GPIO_VALUE	0x22
#define CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG	0x4C
#define CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG	0x4F
#define CRYSTALCOVE_PMIC_PWM_ENABLE		0x80

#define CRYSTALCOVE_PMIC_VIBRA_MAX_BASEUNIT	0x7F

struct mid_vibra_pdata pmic_vibra_data_byt_ffrd8 = {
	.time_divisor	= 0xff,
	.base_unit	= 0x0,
	.gpio_pwm	= -1,
	.name		= "drv8601",
};

static int vibra_pmic_pwm_configure(struct vibra_info *info, bool enable)
{
	u8 clk_div;
	u8 duty_cyc;

	if (enable) {
		/* disable PWM before updating clock div*/
		intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG, 0);

		/* validate the input values */
		if (*info->base_unit > info->max_base_unit) {
			*info->base_unit = info->max_base_unit;
			pr_err("%s: base_unit input is greater than max, using max", __func__);
		}
		if (*info->duty_cycle > info->max_duty_cycle) {
			*info->duty_cycle = info->max_duty_cycle;
			pr_err("%s: duty_cycle input is greater than max, using max", __func__);
		}

		clk_div = *info->base_unit;
		//duty_cyc = *info->duty_cycle;  /*wqf delete*/
		duty_cyc = pmic_vibra_data_byt_ffrd8.time_divisor;
		//printk("wqf %d: %s:duty_cyc=%x\n",__LINE__,__func__,duty_cyc);//wqf add

		clk_div = clk_div | CRYSTALCOVE_PMIC_PWM_ENABLE;
		intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG, duty_cyc);
		intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG, clk_div);
		/*wqf add start*/
		duty_cyc = intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG);
		//printk("wqf %d: %s:duty_cyc=%x\n",__LINE__,__func__,duty_cyc);//wqf add
		clk_div =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG);
		//printk("wqf %d %s:clk_div=%x\n",__LINE__,__func__,clk_div);//wqf add
		/*wqf add end*/
	} else {
		/*disable PWM block */
		clk_div =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG);
		intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG,
				      (clk_div & ~CRYSTALCOVE_PMIC_PWM_ENABLE));
	}
	clk_div =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG);
	duty_cyc =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG);
	pr_debug("%s: clk_div_reg = %#x, duty_cycle_reg = %#x\n", __func__, clk_div, duty_cyc);
	return 0;
}

#if IS_ENABLED(CONFIG_ACPI)
int intel_mid_plat_vibra_probe(struct platform_device *pdev)
{
	struct vibra_info *info;
	struct device *dev = &pdev->dev;
	acpi_handle handle = ACPI_HANDLE(dev);
	struct acpi_device *device;
	const char *hid;
	struct mid_vibra_pdata *data;
	int ret;

	ret = acpi_bus_get_device(handle, &device);
	if (ret) {
		pr_err("%s: could not get acpi device - %d\n", __func__, ret);
		return -ENODEV;
	}
	hid = acpi_device_hid(device);
	pr_debug("%s for %s", __func__, hid);

	data = mid_vibra_acpi_get_drvdata(hid);
	printk("wqf %s:time_divisor=%x\n",__func__,data->time_divisor);
	if (!data) {
		pr_err("Invalid driver data\n");
		return -ENODEV;
	}

	data->gpio_en = acpi_get_gpio_by_index(&pdev->dev, 0, NULL);
	if (data->gpio_en < 0) {
		pr_err("Invalid gpio number from acpi\n");
		return -ENODEV;
	}

	info = mid_vibra_setup(dev, data);
	if (!info)
		return -ENODEV;

	info->pwm_configure = vibra_pmic_pwm_configure;
	info->max_base_unit = CRYSTALCOVE_PMIC_VIBRA_MAX_BASEUNIT;
	info->max_duty_cycle = INTEL_VIBRA_MAX_TIMEDIVISOR;

	pr_debug("%s: using gpio_en: %d", __func__, info->gpio_en);
	ret = gpio_request_one(info->gpio_en, GPIOF_DIR_OUT, "VIBRA ENABLE");
	if (ret != 0) {
		pr_err("gpio_request(%d) fails:%d\n", info->gpio_en, ret);
		return ret;
	}
	/* Re configure the PWM EN GPIO to have drive type as CMOS and pull disable*/
	intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM_EN_GPIO_REG,
			CRYSTALCOVE_PMIC_PWM_EN_GPIO_VALUE);

	ret = sysfs_create_group(&dev->kobj, info->vibra_attr_group);
	if (ret) {
		pr_err("could not register sysfs files\n");
		gpio_free(info->gpio_en);
		return ret;
	}

	platform_set_drvdata(pdev, info);
	pm_runtime_allow(&pdev->dev);
	pm_runtime_put_noidle(&pdev->dev);
	pr_info("%s: vibra probe success\n", __func__);
	return ret;
}

int intel_mid_plat_vibra_remove(struct platform_device *pdev)
{
	struct vibra_info *info = platform_get_drvdata(pdev);
	gpio_free(info->gpio_en);
	sysfs_remove_group(&info->dev->kobj, info->vibra_attr_group);
	platform_set_drvdata(pdev, NULL);
	return 0;
}
/*wqf add for fix vibrator shutdown problem start*/
void  intel_mid_plat_vibra_shutdown(struct platform_device *pdev)
{
	u8 clk_div;
	u8 duty_cyc = 0;
	printk(" %s called \n",__func__);	
	/*disable PWM block */
	clk_div =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG);
	intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG,
			      (clk_div & ~CRYSTALCOVE_PMIC_PWM_ENABLE));
	intel_mid_pmic_writeb(CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG, duty_cyc);
	
	duty_cyc = intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_DUTYCYC_REG);
	printk("wqf %d: %s:duty_cyc=%x\n",__LINE__,__func__,duty_cyc);//wqf add
	clk_div =  intel_mid_pmic_readb(CRYSTALCOVE_PMIC_PWM1_CLKDIV_REG);
	printk("wqf %d %s:clk_div=%x\n",__LINE__,__func__,clk_div);//wqf add
}
/*wqf add for fix vibrator shutdown problem end*/

#else
int intel_mid_plat_vibra_probe(struct platform_device *pdev)
{
	return -EINVAL;
}

int intel_mid_plat_vibra_remove(struct platform_device *pdev)
{
	return -EINVAL;
}
void  intel_mid_plat_vibra_shutdown(struct platform_device *pdev)
{
	
}
#endif

MODULE_ALIAS("platform:intel_mid_vibra");
MODULE_DESCRIPTION("Intel(R) MID ACPI Vibra driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jayachandran.B <jayachandran.b@intel.com>");
MODULE_AUTHOR("Omair Md Abdullah <omair.m.abdullah@intel.com>");
