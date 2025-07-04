// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Aquantia PHY
 *
 * Author: Shaohui Xie <Shaohui.Xie@freescale.com>
 *
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/bitfield.h>
#include <linux/phy.h>

#include "aquantia.h"

#define PHY_ID_AQ1202	0x03a1b445
#define PHY_ID_AQ2104	0x03a1b460
#define PHY_ID_AQR105	0x03a1b4a2
#define PHY_ID_AQR106	0x03a1b4d0
#define PHY_ID_AQR107	0x03a1b4e0
#define PHY_ID_AQCS109	0x03a1b5c2
#define PHY_ID_AQR405	0x03a1b4b0
#define PHY_ID_AQR113C	0x31c31c12
#define PHY_ID_AQR115C	0x31c31c33

#define MDIO_PHYXS_VEND_IF_STATUS		0xe812
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_MASK	GENMASK(7, 3)
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_KR	0
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_KX	1
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_XFI	2
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_USXGMII	3
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_XAUI	4
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_SGMII	6
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_RXAUI	7
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_OCSGMII	10
#define MDIO_PHYXS_VEND_IF_STATUS_TYPE_USX_5G	12

#define MDIO_AN_VEND_PROV			0xc400
#define MDIO_AN_VEND_PROV_1000BASET_FULL	BIT(15)
#define MDIO_AN_VEND_PROV_1000BASET_HALF	BIT(14)
#define MDIO_AN_VEND_PROV_5000BASET_FULL	BIT(11)
#define MDIO_AN_VEND_PROV_2500BASET_FULL	BIT(10)
#define MDIO_AN_VEND_PROV_DOWNSHIFT_EN		BIT(4)
#define MDIO_AN_VEND_PROV_DOWNSHIFT_MASK	GENMASK(3, 0)
#define MDIO_AN_VEND_PROV_DOWNSHIFT_DFLT	4

#define MDIO_AN_TX_VEND_STATUS1			0xc800
#define MDIO_AN_TX_VEND_STATUS1_RATE_MASK	GENMASK(3, 1)
#define MDIO_AN_TX_VEND_STATUS1_10BASET		0
#define MDIO_AN_TX_VEND_STATUS1_100BASETX	1
#define MDIO_AN_TX_VEND_STATUS1_1000BASET	2
#define MDIO_AN_TX_VEND_STATUS1_10GBASET	3
#define MDIO_AN_TX_VEND_STATUS1_2500BASET	4
#define MDIO_AN_TX_VEND_STATUS1_5000BASET	5
#define MDIO_AN_TX_VEND_STATUS1_FULL_DUPLEX	BIT(0)

#define MDIO_AN_TX_VEND_INT_STATUS1		0xcc00
#define MDIO_AN_TX_VEND_INT_STATUS1_DOWNSHIFT	BIT(1)

#define MDIO_AN_TX_VEND_INT_STATUS2		0xcc01
#define MDIO_AN_TX_VEND_INT_STATUS2_MASK	BIT(0)

#define MDIO_AN_TX_VEND_INT_MASK2		0xd401
#define MDIO_AN_TX_VEND_INT_MASK2_LINK		BIT(0)

#define MDIO_AN_RX_LP_STAT1			0xe820
#define MDIO_AN_RX_LP_STAT1_1000BASET_FULL	BIT(15)
#define MDIO_AN_RX_LP_STAT1_1000BASET_HALF	BIT(14)
#define MDIO_AN_RX_LP_STAT1_SHORT_REACH		BIT(13)
#define MDIO_AN_RX_LP_STAT1_AQRATE_DOWNSHIFT	BIT(12)
#define MDIO_AN_RX_LP_STAT1_AQ_PHY		BIT(2)

#define MDIO_AN_RX_LP_STAT4			0xe823
#define MDIO_AN_RX_LP_STAT4_FW_MAJOR		GENMASK(15, 8)
#define MDIO_AN_RX_LP_STAT4_FW_MINOR		GENMASK(7, 0)

#define MDIO_AN_RX_VEND_STAT3			0xe832
#define MDIO_AN_RX_VEND_STAT3_AFR		BIT(0)

/* MDIO_MMD_C22EXT */
#define MDIO_C22EXT_STAT_SGMII_RX_GOOD_FRAMES		0xd292
#define MDIO_C22EXT_STAT_SGMII_RX_BAD_FRAMES		0xd294
#define MDIO_C22EXT_STAT_SGMII_RX_FALSE_CARRIER		0xd297
#define MDIO_C22EXT_STAT_SGMII_TX_GOOD_FRAMES		0xd313
#define MDIO_C22EXT_STAT_SGMII_TX_BAD_FRAMES		0xd315
#define MDIO_C22EXT_STAT_SGMII_TX_FALSE_CARRIER		0xd317
#define MDIO_C22EXT_STAT_SGMII_TX_COLLISIONS		0xd318
#define MDIO_C22EXT_STAT_SGMII_TX_LINE_COLLISIONS	0xd319
#define MDIO_C22EXT_STAT_SGMII_TX_FRAME_ALIGN_ERR	0xd31a
#define MDIO_C22EXT_STAT_SGMII_TX_RUNT_FRAMES		0xd31b

/* Vendor specific 1, MDIO_MMD_VEND1 */
#define VEND1_GLOBAL_FW_ID			0x0020
#define VEND1_GLOBAL_FW_ID_MAJOR		GENMASK(15, 8)
#define VEND1_GLOBAL_FW_ID_MINOR		GENMASK(7, 0)

#define VEND1_GLOBAL_GEN_STAT2			0xc831
#define VEND1_GLOBAL_GEN_STAT2_OP_IN_PROG	BIT(15)

/* The following registers all have similar layouts; first the registers... */
#define VEND1_GLOBAL_CFG_10M			0x0310
#define VEND1_GLOBAL_CFG_100M			0x031b
#define VEND1_GLOBAL_CFG_1G			0x031c
#define VEND1_GLOBAL_CFG_2_5G			0x031d
#define VEND1_GLOBAL_CFG_5G			0x031e
#define VEND1_GLOBAL_CFG_10G			0x031f
/* ...and now the fields */
#define VEND1_GLOBAL_CFG_RATE_ADAPT		GENMASK(8, 7)
#define VEND1_GLOBAL_CFG_RATE_ADAPT_NONE	0
#define VEND1_GLOBAL_CFG_RATE_ADAPT_USX		1
#define VEND1_GLOBAL_CFG_RATE_ADAPT_PAUSE	2

#define VEND1_GLOBAL_RSVD_STAT1			0xc885
#define VEND1_GLOBAL_RSVD_STAT1_FW_BUILD_ID	GENMASK(7, 4)
#define VEND1_GLOBAL_RSVD_STAT1_PROV_ID		GENMASK(3, 0)

#define VEND1_GLOBAL_RSVD_STAT9			0xc88d
#define VEND1_GLOBAL_RSVD_STAT9_MODE		GENMASK(7, 0)
#define VEND1_GLOBAL_RSVD_STAT9_1000BT2		0x23

#define VEND1_GLOBAL_INT_STD_STATUS		0xfc00
#define VEND1_GLOBAL_INT_VEND_STATUS		0xfc01

#define VEND1_GLOBAL_INT_STD_MASK		0xff00
#define VEND1_GLOBAL_INT_STD_MASK_PMA1		BIT(15)
#define VEND1_GLOBAL_INT_STD_MASK_PMA2		BIT(14)
#define VEND1_GLOBAL_INT_STD_MASK_PCS1		BIT(13)
#define VEND1_GLOBAL_INT_STD_MASK_PCS2		BIT(12)
#define VEND1_GLOBAL_INT_STD_MASK_PCS3		BIT(11)
#define VEND1_GLOBAL_INT_STD_MASK_PHY_XS1	BIT(10)
#define VEND1_GLOBAL_INT_STD_MASK_PHY_XS2	BIT(9)
#define VEND1_GLOBAL_INT_STD_MASK_AN1		BIT(8)
#define VEND1_GLOBAL_INT_STD_MASK_AN2		BIT(7)
#define VEND1_GLOBAL_INT_STD_MASK_GBE		BIT(6)
#define VEND1_GLOBAL_INT_STD_MASK_ALL		BIT(0)

#define VEND1_GLOBAL_INT_VEND_MASK		0xff01
#define VEND1_GLOBAL_INT_VEND_MASK_PMA		BIT(15)
#define VEND1_GLOBAL_INT_VEND_MASK_PCS		BIT(14)
#define VEND1_GLOBAL_INT_VEND_MASK_PHY_XS	BIT(13)
#define VEND1_GLOBAL_INT_VEND_MASK_AN		BIT(12)
#define VEND1_GLOBAL_INT_VEND_MASK_GBE		BIT(11)
#define VEND1_GLOBAL_INT_VEND_MASK_GLOBAL1	BIT(2)
#define VEND1_GLOBAL_INT_VEND_MASK_GLOBAL2	BIT(1)
#define VEND1_GLOBAL_INT_VEND_MASK_GLOBAL3	BIT(0)

/* Sleep and timeout for checking if the Processor-Intensive
 * MDIO operation is finished
 */
#define AQR107_OP_IN_PROG_SLEEP		1000
#define AQR107_OP_IN_PROG_TIMEOUT	100000

struct aqr107_hw_stat {
	const char *name;
	int reg;
	int size;
};

#define SGMII_STAT(n, r, s) { n, MDIO_C22EXT_STAT_SGMII_ ## r, s }
static const struct aqr107_hw_stat aqr107_hw_stats[] = {
	SGMII_STAT("sgmii_rx_good_frames",	    RX_GOOD_FRAMES,	26),
	SGMII_STAT("sgmii_rx_bad_frames",	    RX_BAD_FRAMES,	26),
	SGMII_STAT("sgmii_rx_false_carrier_events", RX_FALSE_CARRIER,	 8),
	SGMII_STAT("sgmii_tx_good_frames",	    TX_GOOD_FRAMES,	26),
	SGMII_STAT("sgmii_tx_bad_frames",	    TX_BAD_FRAMES,	26),
	SGMII_STAT("sgmii_tx_false_carrier_events", TX_FALSE_CARRIER,	 8),
	SGMII_STAT("sgmii_tx_collisions",	    TX_COLLISIONS,	 8),
	SGMII_STAT("sgmii_tx_line_collisions",	    TX_LINE_COLLISIONS,	 8),
	SGMII_STAT("sgmii_tx_frame_alignment_err",  TX_FRAME_ALIGN_ERR,	16),
	SGMII_STAT("sgmii_tx_runt_frames",	    TX_RUNT_FRAMES,	22),
};
#define AQR107_SGMII_STAT_SZ ARRAY_SIZE(aqr107_hw_stats)

struct aqr107_priv {
	u64 sgmii_stats[AQR107_SGMII_STAT_SZ];
};

static int aqr107_get_sset_count(struct phy_device *phydev)
{
	return AQR107_SGMII_STAT_SZ;
}

static void aqr107_get_strings(struct phy_device *phydev, u8 *data)
{
	int i;

	for (i = 0; i < AQR107_SGMII_STAT_SZ; i++)
		strscpy(data + i * ETH_GSTRING_LEN, aqr107_hw_stats[i].name,
			ETH_GSTRING_LEN);
}

static u64 aqr107_get_stat(struct phy_device *phydev, int index)
{
	const struct aqr107_hw_stat *stat = aqr107_hw_stats + index;
	int len_l = min(stat->size, 16);
	int len_h = stat->size - len_l;
	u64 ret;
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_C22EXT, stat->reg);
	if (val < 0)
		return U64_MAX;

	ret = val & GENMASK(len_l - 1, 0);
	if (len_h) {
		val = phy_read_mmd(phydev, MDIO_MMD_C22EXT, stat->reg + 1);
		if (val < 0)
			return U64_MAX;

		ret += (val & GENMASK(len_h - 1, 0)) << 16;
	}

	return ret;
}

static void aqr107_get_stats(struct phy_device *phydev,
			     struct ethtool_stats *stats, u64 *data)
{
	struct aqr107_priv *priv = phydev->priv;
	u64 val;
	int i;

	for (i = 0; i < AQR107_SGMII_STAT_SZ; i++) {
		val = aqr107_get_stat(phydev, i);
		if (val == U64_MAX)
			phydev_err(phydev, "Reading HW Statistics failed for %s\n",
				   aqr107_hw_stats[i].name);
		else
			priv->sgmii_stats[i] += val;

		data[i] = priv->sgmii_stats[i];
	}
}

static int aqr_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u16 reg;
	int ret;

	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	/* Clause 45 has no standardized support for 1000BaseT, therefore
	 * use vendor registers for this mode.
	 */
	reg = 0;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			      phydev->advertising))
		reg |= MDIO_AN_VEND_PROV_1000BASET_FULL;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
			      phydev->advertising))
		reg |= MDIO_AN_VEND_PROV_1000BASET_HALF;

	/* Handle the case when the 2.5G and 5G speeds are not advertised */
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			      phydev->advertising))
		reg |= MDIO_AN_VEND_PROV_2500BASET_FULL;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
			      phydev->advertising))
		reg |= MDIO_AN_VEND_PROV_5000BASET_FULL;

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_VEND_PROV,
				     MDIO_AN_VEND_PROV_1000BASET_HALF |
				     MDIO_AN_VEND_PROV_1000BASET_FULL |
				     MDIO_AN_VEND_PROV_2500BASET_FULL |
				     MDIO_AN_VEND_PROV_5000BASET_FULL, reg);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int aqr_config_intr(struct phy_device *phydev)
{
	bool en = phydev->interrupts == PHY_INTERRUPT_ENABLED;
	int err;

	if (en) {
		/* Clear any pending interrupts before enabling them */
		err = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_TX_VEND_INT_STATUS2);
		if (err < 0)
			return err;
	}

	err = phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_TX_VEND_INT_MASK2,
			    en ? MDIO_AN_TX_VEND_INT_MASK2_LINK : 0);
	if (err < 0)
		return err;

	err = phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_GLOBAL_INT_STD_MASK,
			    en ? VEND1_GLOBAL_INT_STD_MASK_ALL : 0);
	if (err < 0)
		return err;

	err = phy_write_mmd(phydev, MDIO_MMD_VEND1, VEND1_GLOBAL_INT_VEND_MASK,
			    en ? VEND1_GLOBAL_INT_VEND_MASK_GLOBAL3 |
			    VEND1_GLOBAL_INT_VEND_MASK_AN : 0);
	if (err < 0)
		return err;

	if (!en) {
		/* Clear any pending interrupts after we have disabled them */
		err = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_TX_VEND_INT_STATUS2);
		if (err < 0)
			return err;
	}

	return 0;
}

static irqreturn_t aqr_handle_interrupt(struct phy_device *phydev)
{
	int irq_status;

	irq_status = phy_read_mmd(phydev, MDIO_MMD_AN,
				  MDIO_AN_TX_VEND_INT_STATUS2);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & MDIO_AN_TX_VEND_INT_STATUS2_MASK))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static int aqr_read_status(struct phy_device *phydev)
{
	int val;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_RX_LP_STAT1);
		if (val < 0)
			return val;

		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
				 phydev->lp_advertising,
				 val & MDIO_AN_RX_LP_STAT1_1000BASET_FULL);
		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
				 phydev->lp_advertising,
				 val & MDIO_AN_RX_LP_STAT1_1000BASET_HALF);
	}

	return genphy_c45_read_status(phydev);
}

static int aqr107_read_rate(struct phy_device *phydev)
{
	u32 config_reg;
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_TX_VEND_STATUS1);
	if (val < 0)
		return val;

	if (val & MDIO_AN_TX_VEND_STATUS1_FULL_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	switch (FIELD_GET(MDIO_AN_TX_VEND_STATUS1_RATE_MASK, val)) {
	case MDIO_AN_TX_VEND_STATUS1_10BASET:
		phydev->speed = SPEED_10;
		config_reg = VEND1_GLOBAL_CFG_10M;
		break;
	case MDIO_AN_TX_VEND_STATUS1_100BASETX:
		phydev->speed = SPEED_100;
		config_reg = VEND1_GLOBAL_CFG_100M;
		break;
	case MDIO_AN_TX_VEND_STATUS1_1000BASET:
		phydev->speed = SPEED_1000;
		config_reg = VEND1_GLOBAL_CFG_1G;
		break;
	case MDIO_AN_TX_VEND_STATUS1_2500BASET:
		phydev->speed = SPEED_2500;
		config_reg = VEND1_GLOBAL_CFG_2_5G;
		break;
	case MDIO_AN_TX_VEND_STATUS1_5000BASET:
		phydev->speed = SPEED_5000;
		config_reg = VEND1_GLOBAL_CFG_5G;
		break;
	case MDIO_AN_TX_VEND_STATUS1_10GBASET:
		phydev->speed = SPEED_10000;
		config_reg = VEND1_GLOBAL_CFG_10G;
		break;
	default:
		phydev->speed = SPEED_UNKNOWN;
		return 0;
	}

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, config_reg);
	if (val < 0)
		return val;

	if (FIELD_GET(VEND1_GLOBAL_CFG_RATE_ADAPT, val) ==
	    VEND1_GLOBAL_CFG_RATE_ADAPT_PAUSE)
		phydev->rate_matching = RATE_MATCH_PAUSE;
	else
		phydev->rate_matching = RATE_MATCH_NONE;

	return 0;
}

static int aqr107_read_status(struct phy_device *phydev)
{
	int val, ret;

	ret = aqr_read_status(phydev);
	if (ret)
		return ret;

	if (!phydev->link || phydev->autoneg == AUTONEG_DISABLE)
		return 0;

	val = phy_read_mmd(phydev, MDIO_MMD_PHYXS, MDIO_PHYXS_VEND_IF_STATUS);
	if (val < 0)
		return val;

	switch (FIELD_GET(MDIO_PHYXS_VEND_IF_STATUS_TYPE_MASK, val)) {
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_KR:
		phydev->interface = PHY_INTERFACE_MODE_10GKR;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_KX:
		phydev->interface = PHY_INTERFACE_MODE_1000BASEKX;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_XFI:
		phydev->interface = PHY_INTERFACE_MODE_10GBASER;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_USXGMII:
		phydev->interface = PHY_INTERFACE_MODE_USXGMII;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_XAUI:
		phydev->interface = PHY_INTERFACE_MODE_XAUI;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_SGMII:
		phydev->interface = PHY_INTERFACE_MODE_SGMII;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_RXAUI:
		phydev->interface = PHY_INTERFACE_MODE_RXAUI;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_OCSGMII:
		phydev->interface = PHY_INTERFACE_MODE_2500BASEX;
		break;
	case MDIO_PHYXS_VEND_IF_STATUS_TYPE_USX_5G:
		phydev->interface = PHY_INTERFACE_MODE_5GBASER;
		break;
	default:
		phydev->interface = PHY_INTERFACE_MODE_NA;
		break;
	}

	/* Read possibly downshifted rate from vendor register */
	return aqr107_read_rate(phydev);
}

static int aqr107_get_downshift(struct phy_device *phydev, u8 *data)
{
	int val, cnt, enable;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_VEND_PROV);
	if (val < 0)
		return val;

	enable = FIELD_GET(MDIO_AN_VEND_PROV_DOWNSHIFT_EN, val);
	cnt = FIELD_GET(MDIO_AN_VEND_PROV_DOWNSHIFT_MASK, val);

	*data = enable && cnt ? cnt : DOWNSHIFT_DEV_DISABLE;

	return 0;
}

static int aqr107_set_downshift(struct phy_device *phydev, u8 cnt)
{
	int val = 0;

	if (!FIELD_FIT(MDIO_AN_VEND_PROV_DOWNSHIFT_MASK, cnt))
		return -E2BIG;

	if (cnt != DOWNSHIFT_DEV_DISABLE) {
		val = MDIO_AN_VEND_PROV_DOWNSHIFT_EN;
		val |= FIELD_PREP(MDIO_AN_VEND_PROV_DOWNSHIFT_MASK, cnt);
	}

	return phy_modify_mmd(phydev, MDIO_MMD_AN, MDIO_AN_VEND_PROV,
			      MDIO_AN_VEND_PROV_DOWNSHIFT_EN |
			      MDIO_AN_VEND_PROV_DOWNSHIFT_MASK, val);
}

static int aqr107_get_tunable(struct phy_device *phydev,
			      struct ethtool_tunable *tuna, void *data)
{
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		return aqr107_get_downshift(phydev, data);
	default:
		return -EOPNOTSUPP;
	}
}

static int aqr107_set_tunable(struct phy_device *phydev,
			      struct ethtool_tunable *tuna, const void *data)
{
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		return aqr107_set_downshift(phydev, *(const u8 *)data);
	default:
		return -EOPNOTSUPP;
	}
}

/* If we configure settings whilst firmware is still initializing the chip,
 * then these settings may be overwritten. Therefore make sure chip
 * initialization has completed. Use presence of the firmware ID as
 * indicator for initialization having completed.
 * The chip also provides a "reset completed" bit, but it's cleared after
 * read. Therefore function would time out if called again.
 */
static int aqr107_wait_reset_complete(struct phy_device *phydev)
{
	int val;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					 VEND1_GLOBAL_FW_ID, val, val != 0,
					 20000, 2000000, false);
}

static void aqr107_chip_info(struct phy_device *phydev)
{
	u8 fw_major, fw_minor, build_id, prov_id;
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, VEND1_GLOBAL_FW_ID);
	if (val < 0)
		return;

	fw_major = FIELD_GET(VEND1_GLOBAL_FW_ID_MAJOR, val);
	fw_minor = FIELD_GET(VEND1_GLOBAL_FW_ID_MINOR, val);

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, VEND1_GLOBAL_RSVD_STAT1);
	if (val < 0)
		return;

	build_id = FIELD_GET(VEND1_GLOBAL_RSVD_STAT1_FW_BUILD_ID, val);
	prov_id = FIELD_GET(VEND1_GLOBAL_RSVD_STAT1_PROV_ID, val);

	phydev_dbg(phydev, "FW %u.%u, Build %u, Provisioning %u\n",
		   fw_major, fw_minor, build_id, prov_id);
}

static int aqr107_config_init(struct phy_device *phydev)
{
	int ret;

	/* Check that the PHY interface type is compatible */
	if (phydev->interface != PHY_INTERFACE_MODE_SGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_1000BASEKX &&
	    phydev->interface != PHY_INTERFACE_MODE_2500BASEX &&
	    phydev->interface != PHY_INTERFACE_MODE_XGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_USXGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_10GKR &&
	    phydev->interface != PHY_INTERFACE_MODE_10GBASER &&
	    phydev->interface != PHY_INTERFACE_MODE_XAUI &&
	    phydev->interface != PHY_INTERFACE_MODE_RXAUI &&
	    phydev->interface != PHY_INTERFACE_MODE_5GBASER)
		return -ENODEV;

	WARN(phydev->interface == PHY_INTERFACE_MODE_XGMII,
	     "Your devicetree is out of date, please update it. The AQR107 family doesn't support XGMII, maybe you mean USXGMII.\n");

	ret = aqr107_wait_reset_complete(phydev);
	if (!ret)
		aqr107_chip_info(phydev);

	return aqr107_set_downshift(phydev, MDIO_AN_VEND_PROV_DOWNSHIFT_DFLT);
}

static int aqcs109_config_init(struct phy_device *phydev)
{
	int ret;

	/* Check that the PHY interface type is compatible */
	if (phydev->interface != PHY_INTERFACE_MODE_SGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_2500BASEX)
		return -ENODEV;

	ret = aqr107_wait_reset_complete(phydev);
	if (!ret)
		aqr107_chip_info(phydev);

	/* AQCS109 belongs to a chip family partially supporting 10G and 5G.
	 * PMA speed ability bits are the same for all members of the family,
	 * AQCS109 however supports speeds up to 2.5G only.
	 */
	phy_set_max_speed(phydev, SPEED_2500);

	return aqr107_set_downshift(phydev, MDIO_AN_VEND_PROV_DOWNSHIFT_DFLT);
}

static void aqr107_link_change_notify(struct phy_device *phydev)
{
	u8 fw_major, fw_minor;
	bool downshift, short_reach, afr;
	int mode, val;

	if (phydev->state != PHY_RUNNING || phydev->autoneg == AUTONEG_DISABLE)
		return;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_RX_LP_STAT1);
	/* call failed or link partner is no Aquantia PHY */
	if (val < 0 || !(val & MDIO_AN_RX_LP_STAT1_AQ_PHY))
		return;

	short_reach = val & MDIO_AN_RX_LP_STAT1_SHORT_REACH;
	downshift = val & MDIO_AN_RX_LP_STAT1_AQRATE_DOWNSHIFT;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_RX_LP_STAT4);
	if (val < 0)
		return;

	fw_major = FIELD_GET(MDIO_AN_RX_LP_STAT4_FW_MAJOR, val);
	fw_minor = FIELD_GET(MDIO_AN_RX_LP_STAT4_FW_MINOR, val);

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_RX_VEND_STAT3);
	if (val < 0)
		return;

	afr = val & MDIO_AN_RX_VEND_STAT3_AFR;

	phydev_dbg(phydev, "Link partner is Aquantia PHY, FW %u.%u%s%s%s\n",
		   fw_major, fw_minor,
		   short_reach ? ", short reach mode" : "",
		   downshift ? ", fast-retrain downshift advertised" : "",
		   afr ? ", fast reframe advertised" : "");

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, VEND1_GLOBAL_RSVD_STAT9);
	if (val < 0)
		return;

	mode = FIELD_GET(VEND1_GLOBAL_RSVD_STAT9_MODE, val);
	if (mode == VEND1_GLOBAL_RSVD_STAT9_1000BT2)
		phydev_info(phydev, "Aquantia 1000Base-T2 mode active\n");
}

static int aqr107_wait_processor_intensive_op(struct phy_device *phydev)
{
	int val, err;

	/* The datasheet notes to wait at least 1ms after issuing a
	 * processor intensive operation before checking.
	 * We cannot use the 'sleep_before_read' parameter of read_poll_timeout
	 * because that just determines the maximum time slept, not the minimum.
	 */
	usleep_range(1000, 5000);

	err = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					VEND1_GLOBAL_GEN_STAT2, val,
					!(val & VEND1_GLOBAL_GEN_STAT2_OP_IN_PROG),
					AQR107_OP_IN_PROG_SLEEP,
					AQR107_OP_IN_PROG_TIMEOUT, false);
	if (err) {
		phydev_err(phydev, "timeout: processor-intensive MDIO operation\n");
		return err;
	}

	return 0;
}

static int aqr107_get_rate_matching(struct phy_device *phydev,
				    phy_interface_t iface)
{
	if (iface == PHY_INTERFACE_MODE_10GBASER ||
	    iface == PHY_INTERFACE_MODE_2500BASEX ||
	    iface == PHY_INTERFACE_MODE_NA ||
	    iface == PHY_INTERFACE_MODE_5GBASER)
		return RATE_MATCH_PAUSE;
	return RATE_MATCH_NONE;
}

static int aqr107_suspend(struct phy_device *phydev)
{
	int err;

	err = phy_set_bits_mmd(phydev, MDIO_MMD_VEND1, MDIO_CTRL1,
			       MDIO_CTRL1_LPOWER);
	if (err)
		return err;

	return aqr107_wait_processor_intensive_op(phydev);
}

static int aqr107_resume(struct phy_device *phydev)
{
	int err;

	err = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND1, MDIO_CTRL1,
				 MDIO_CTRL1_LPOWER);
	if (err)
		return err;

	return aqr107_wait_processor_intensive_op(phydev);
}

static int aqr107_probe(struct phy_device *phydev)
{
	phydev->priv = devm_kzalloc(&phydev->mdio.dev,
				    sizeof(struct aqr107_priv), GFP_KERNEL);
	if (!phydev->priv)
		return -ENOMEM;

	return aqr_hwmon_probe(phydev);
}

static int aqr107_read_downshift_event(struct phy_device *phydev)
{
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_TX_VEND_INT_STATUS1);
	if (val < 0)
		return val;

	return !!(val & MDIO_AN_TX_VEND_INT_STATUS1_DOWNSHIFT);
}

static int aqr113_fix_provisioning(struct phy_device *phydev)
{
	int config_regs[] = {0x31B, 0x31C, 0x31D, 0x31E, 0x31F};
	int i, val = 0;

	for (i = 0; i < ARRAY_SIZE(config_regs); i++) {
		val = phy_read_mmd(phydev, MDIO_MMD_VEND1, config_regs[i]);

#if IS_ENABLED(CONFIG_AQUANTIA_MACSEC)
		/* Enabling MACSEC provisioning */
		val |= BIT(9);
#endif
		/* Enabling EEE provisioning */
		val |= BIT(11);

		phy_write_mmd(phydev, MDIO_MMD_VEND1, config_regs[i], val);
	}

	return 0;
}

static int aqr113_config_init(struct phy_device *phydev)
{
	int ret;

#if IS_ENABLED(CONFIG_AQUANTIA_MACSEC)
	struct aqr107_priv *priv = phydev->priv;
	struct aqr_port *port = &priv->port;
#endif

	/* Check that the PHY interface type is compatible */
	if (phydev->interface != PHY_INTERFACE_MODE_SGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_2500BASEX &&
	    phydev->interface != PHY_INTERFACE_MODE_XGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_USXGMII &&
	    phydev->interface != PHY_INTERFACE_MODE_10GKR)
		return -ENODEV;

	WARN(phydev->interface == PHY_INTERFACE_MODE_XGMII,
	     "Your devicetree is out of date, please update it. The AQR107 family doesn't support XGMII, maybe you mean USXGMII.\n");

	phydev->is_c45 = true;

	aqr113_fix_provisioning(phydev);

#if IS_ENABLED(CONFIG_AQUANTIA_MACSEC)
	port->device = aqr_gen_4;
	port->priv = phydev;
	port->mdio_ops.aqr_mdio_write = &aqr_mdio_write;
	port->mdio_ops.aqr_mdio_read = &aqr_mdio_read;

	phydev->macsec_ops = &aqr_macsec_ops;
#endif

#ifdef MDIO_LOAD
	aquantia_upload_firmware(phydev);
#endif

	ret = aqr107_wait_reset_complete(phydev);
	if (!ret)
		aqr107_chip_info(phydev);

	linkmode_copy(phydev->advertising, phydev->supported);
	/* ensure that a latched downshift event is cleared */
	aqr107_read_downshift_event(phydev);

	return aqr107_set_downshift(phydev, MDIO_AN_VEND_PROV_DOWNSHIFT_DFLT);
}

static int aqr113c_get_features(struct phy_device *phydev)
{
	unsigned long *supported = phydev->supported;

	/* PHY supports speeds up to 10G with autoneg. PMA capabilities
	 * are not useful.
	 */
	linkmode_or(supported, supported, phy_gbit_features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT, supported);

	return 0;
}

static struct phy_driver aqr_driver[] = {
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQ1202),
	.name		= "Aquantia AQ1202",
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr_read_status,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQ2104),
	.name		= "Aquantia AQ2104",
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr_read_status,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR105),
	.name		= "Aquantia AQR105",
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr_read_status,
	.suspend	= aqr107_suspend,
	.resume		= aqr107_resume,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR106),
	.name		= "Aquantia AQR106",
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr_read_status,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR107),
	.name		= "Aquantia AQR107",
	.probe		= aqr107_probe,
	.get_rate_matching = aqr107_get_rate_matching,
	.config_init	= aqr107_config_init,
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr107_read_status,
	.get_tunable    = aqr107_get_tunable,
	.set_tunable    = aqr107_set_tunable,
	.suspend	= aqr107_suspend,
	.resume		= aqr107_resume,
	.get_sset_count	= aqr107_get_sset_count,
	.get_strings	= aqr107_get_strings,
	.get_stats	= aqr107_get_stats,
	.link_change_notify = aqr107_link_change_notify,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQCS109),
	.name		= "Aquantia AQCS109",
	.probe		= aqr107_probe,
	.get_rate_matching = aqr107_get_rate_matching,
	.config_init	= aqcs109_config_init,
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr107_read_status,
	.get_tunable    = aqr107_get_tunable,
	.set_tunable    = aqr107_set_tunable,
	.suspend	= aqr107_suspend,
	.resume		= aqr107_resume,
	.get_sset_count	= aqr107_get_sset_count,
	.get_strings	= aqr107_get_strings,
	.get_stats	= aqr107_get_stats,
	.link_change_notify = aqr107_link_change_notify,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR405),
	.name		= "Aquantia AQR405",
	.config_aneg    = aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr_read_status,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR113C),
	.name           = "Aquantia AQR113C",
	.probe          = aqr107_probe,
	.get_rate_matching = aqr107_get_rate_matching,
	.config_init    = aqr107_config_init,
	.config_aneg    = aqr_config_aneg,
	.config_intr    = aqr_config_intr,
	.handle_interrupt       = aqr_handle_interrupt,
	.read_status    = aqr107_read_status,
	.get_tunable    = aqr107_get_tunable,
	.set_tunable    = aqr107_set_tunable,
	.suspend        = aqr107_suspend,
	.resume         = aqr107_resume,
	.get_sset_count = aqr107_get_sset_count,
	.get_strings    = aqr107_get_strings,
	.get_stats      = aqr107_get_stats,
	.get_features   = aqr113c_get_features,
	.link_change_notify = aqr107_link_change_notify,
},
{
	PHY_ID_MATCH_MODEL(PHY_ID_AQR115C),
	.name		= "Aquantia AQR115c",
	.probe		= aqr107_probe,
	.config_init	= aqr113_config_init,
	.config_aneg	= aqr_config_aneg,
	.config_intr	= aqr_config_intr,
	.handle_interrupt = aqr_handle_interrupt,
	.read_status	= aqr107_read_status,
	.get_tunable	= aqr107_get_tunable,
	.set_tunable	= aqr107_set_tunable,
	.suspend	    = aqr107_suspend,
	.resume		    = aqr107_resume,
	.get_sset_count = aqr107_get_sset_count,
	.get_strings	= aqr107_get_strings,
	.get_stats	= aqr107_get_stats,
	.link_change_notify = aqr107_link_change_notify,
},
};

module_phy_driver(aqr_driver);

static struct mdio_device_id __maybe_unused aqr_tbl[] = {
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQ1202) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQ2104) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR105) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR106) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR107) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQCS109) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR405) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR113C) },
	{ PHY_ID_MATCH_MODEL(PHY_ID_AQR115C) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, aqr_tbl);

MODULE_DESCRIPTION("Aquantia PHY driver");
MODULE_AUTHOR("Shaohui Xie <Shaohui.Xie@freescale.com>");
MODULE_LICENSE("GPL v2");
