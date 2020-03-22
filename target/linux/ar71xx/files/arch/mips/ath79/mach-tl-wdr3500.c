/*
 *  TP-LINK TL-WDR3500 board support (WR961_V6CN with i2s audio)
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include "common.h"
#include "dev-audio.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define WDR3500_GPIO_LED_USB		11
#define WDR3500_GPIO_LED_WLAN2G		13
#define WDR3500_GPIO_LED_SYSTEM		14
#define WDR3500_GPIO_SPDIF_OUT		15  /* NC */
#define WDR3500_GPIO_I2S_MIC_SD		18  /* NC */
#define WDR3500_GPIO_I2S_SD 		19  /* to LAN1_LED WR941_V6CN*/
#define WDR3500_GPIO_I2S_CLK		20  /* to LAN2_LED WR941_V6CN*/ 
#define WDR3500_GPIO_I2S_WS 		21  /* to LAN3_LED WR941_V6CN*/
#define WDR3500_GPIO_I2S_MCLK		22  /* to LAN4_LED WR941_V6CN*/
#define WDR3500_GPIO_BTN_WPS		16  
#define WDR3500_GPIO_BTN_RFKILL		17
#define WDR3500_GPIO_USB_POWER		12

#define WDR3500_KEYS_POLL_INTERVAL	20	/* msecs */
#define WDR3500_KEYS_DEBOUNCE_INTERVAL	(3 * WDR3500_KEYS_POLL_INTERVAL)

#define WDR3500_MAC0_OFFSET		0
#define WDR3500_MAC1_OFFSET		6
#define WDR3500_WMAC_CALDATA_OFFSET	0x1000
#define WDR3500_PCIE_CALDATA_OFFSET	0x5000

static const char *wdr3500_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data wdr3500_flash_data = {
	.part_probes	= wdr3500_part_probes,
};
static struct platform_device wdr3500_internal_codec = {
	.name		= "ath79-internal-codec",
	.id		= -1,
};

static struct platform_device wdr3500_spdif_codec = {
	.name		= "ak4430-codec",
	.id		= -1,
};
static struct gpio_led wdr3500_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= WDR3500_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb",
		.gpio		= WDR3500_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan2g",
		.gpio		= WDR3500_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wdr3500_gpio_keys[] __initdata = {
	{
		.desc		= "QSS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WDR3500_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR3500_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = WDR3500_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR3500_GPIO_BTN_RFKILL,
	},
};

static void __init wdr3500_audio_setup(void)
{
	u32 t;

	/* Reset I2S internal controller */
	t = ath79_reset_rr(AR71XX_RESET_REG_RESET_MODULE);
	ath79_reset_wr(AR71XX_RESET_REG_RESET_MODULE, t | AR934X_RESET_I2S );
	udelay(1);

	/* GPIO configuration
	   GPIOs 19,20,21,22 are configured as I2S signal - Output
	   GPIO 18 is MIC_SD - Input
	   GPIO 15 is SPDIF - Output
	   Please note that the value in direction_output doesn't really matter
	   here as GPIOs are configured to relay internal data signal
	*/
	gpio_request(WDR3500_GPIO_I2S_CLK, "I2S CLK");
	ath79_gpio_output_select(WDR3500_GPIO_I2S_CLK, AR934X_GPIO_OUT_MUX_I2S_CLK);
	gpio_direction_output(WDR3500_GPIO_I2S_CLK, 0);

	gpio_request(WDR3500_GPIO_I2S_WS, "I2S WS");
	ath79_gpio_output_select(WDR3500_GPIO_I2S_WS, AR934X_GPIO_OUT_MUX_I2S_WS);
	gpio_direction_output(WDR3500_GPIO_I2S_WS, 0);

	gpio_request(WDR3500_GPIO_I2S_SD, "I2S SD");
	ath79_gpio_output_select(WDR3500_GPIO_I2S_SD, AR934X_GPIO_OUT_MUX_I2S_SD);
	gpio_direction_output(WDR3500_GPIO_I2S_SD, 0);

	gpio_request(WDR3500_GPIO_I2S_MCLK, "I2S MCLK");
	ath79_gpio_output_select(WDR3500_GPIO_I2S_MCLK, AR934X_GPIO_OUT_MUX_I2S_MCK);
	gpio_direction_output(WDR3500_GPIO_I2S_MCLK, 0);

	gpio_request(WDR3500_GPIO_SPDIF_OUT, "SPDIF OUT");
	ath79_gpio_output_select(WDR3500_GPIO_SPDIF_OUT, AR934X_GPIO_OUT_MUX_SPDIF_OUT);
	gpio_direction_output(WDR3500_GPIO_SPDIF_OUT, 0);

	gpio_request(WDR3500_GPIO_I2S_MIC_SD, "I2S MIC_SD");
	ath79_gpio_input_select(WDR3500_GPIO_I2S_MIC_SD, AR934X_GPIO_IN_MUX_I2S_MIC_SD);
	gpio_direction_input(WDR3500_GPIO_I2S_MIC_SD);

	/* Init stereo block registers in default configuration */
	ath79_audio_setup();
}
static void __init wdr3500_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&wdr3500_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wdr3500_leds_gpio),
				 wdr3500_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WDR3500_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wdr3500_gpio_keys),
					wdr3500_gpio_keys);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(art + WDR3500_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, 1);
	ap9x_pci_setup_wmac_led_pin(0, 0);
	ap91_pci_init(art + WDR3500_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(1);

	/* WAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 2);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(0);

	gpio_request_one(WDR3500_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();
		/* Audio initialization: PCM/I2S and CODEC */
	wdr3500_audio_setup();
	platform_device_register(&wdr3500_spdif_codec);
	platform_device_register(&wdr3500_internal_codec);
	ath79_audio_device_register();
}

MIPS_MACHINE(ATH79_MACH_TL_WDR3500, "TL-WDR3500",
	     "TP-LINK TL-WDR3500",
	     wdr3500_setup);
