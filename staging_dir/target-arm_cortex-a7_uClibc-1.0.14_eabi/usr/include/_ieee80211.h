/*
 *  Copyright (c) 2008 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _NET80211__IEEE80211_H_
#define _NET80211__IEEE80211_H_
#include "ieee80211_rrm.h"
#include "ieee80211_wnm_proto.h"
#include "ieee80211_band_steering_api.h"
#include "ieee80211_phytype.h"
#if ATH_SSID_STEERING
#include "ath_ssid_steering.h"
#endif

/* These defines should match the table from ah_internal.h */
typedef enum {
	DFS_UNINIT_DOMAIN	= 0,	/* Uninitialized dfs domain */
	DFS_FCC_DOMAIN		= 1,	/* FCC3 dfs domain */
	DFS_ETSI_DOMAIN		= 2,	/* ETSI dfs domain */
	DFS_MKK4_DOMAIN		= 3	/* Japan dfs domain */
}HAL_DFS_DOMAIN;


enum ieee80211_mode {
    IEEE80211_MODE_INVALID = 0,
    IEEE80211_MODE_A,
    IEEE80211_MODE_B,
    IEEE80211_MODE_G,
    IEEE80211_MODE_NA,
    IEEE80211_MODE_NG,
    IEEE80211_MODE_N,
    IEEE80211_MODE_AC,
    /* New modes should be added here */
    IEEE80211_MODE_MAX,
};


/* XXX not really a mode; there are really multiple PHY's */
enum ieee80211_phymode {
    IEEE80211_MODE_AUTO             = 0,    /* autoselect */
    IEEE80211_MODE_11A              = 1,    /* 5GHz, OFDM */
    IEEE80211_MODE_11B              = 2,    /* 2GHz, CCK */
    IEEE80211_MODE_11G              = 3,    /* 2GHz, OFDM */
    IEEE80211_MODE_FH               = 4,    /* 2GHz, GFSK */
    IEEE80211_MODE_TURBO_A          = 5,    /* 5GHz, OFDM, 2x clock dynamic turbo */
    IEEE80211_MODE_TURBO_G          = 6,    /* 2GHz, OFDM, 2x clock dynamic turbo */
    IEEE80211_MODE_11NA_HT20        = 7,    /* 5Ghz, HT20 */
    IEEE80211_MODE_11NG_HT20        = 8,    /* 2Ghz, HT20 */
    IEEE80211_MODE_11NA_HT40PLUS    = 9,    /* 5Ghz, HT40 (ext ch +1) */
    IEEE80211_MODE_11NA_HT40MINUS   = 10,   /* 5Ghz, HT40 (ext ch -1) */
    IEEE80211_MODE_11NG_HT40PLUS    = 11,   /* 2Ghz, HT40 (ext ch +1) */
    IEEE80211_MODE_11NG_HT40MINUS   = 12,   /* 2Ghz, HT40 (ext ch -1) */
    IEEE80211_MODE_11NG_HT40        = 13,   /* 2Ghz, Auto HT40 */
    IEEE80211_MODE_11NA_HT40        = 14,   /* 5Ghz, Auto HT40 */
    IEEE80211_MODE_11AC_VHT20       = 15,   /* 5Ghz, VHT20 */
    IEEE80211_MODE_11AC_VHT40PLUS   = 16,   /* 5Ghz, VHT40 (Ext ch +1) */
    IEEE80211_MODE_11AC_VHT40MINUS  = 17,   /* 5Ghz  VHT40 (Ext ch -1) */
    IEEE80211_MODE_11AC_VHT40       = 18,   /* 5Ghz, VHT40 */
    IEEE80211_MODE_11AC_VHT80       = 19,   /* 5Ghz, VHT80 */
    IEEE80211_MODE_11AC_VHT160      = 20,   /* 5Ghz, VHT160 */
    IEEE80211_MODE_11AC_VHT80_80    = 21,   /* 5Ghz, VHT80_80 */
};
#define IEEE80211_MODE_MAX      (IEEE80211_MODE_11AC_VHT80_80 + 1)
#define IEEE80211_MODE_NONE      (IEEE80211_MODE_MAX + 1)

enum ieee80211_opmode {
    IEEE80211_M_STA         = 1,                 /* infrastructure station */
    IEEE80211_M_IBSS        = 0,                 /* IBSS (adhoc) station */
    IEEE80211_M_AHDEMO      = 3,                 /* Old lucent compatible adhoc demo */
    IEEE80211_M_HOSTAP      = 6,                 /* Software Access Point */
    IEEE80211_M_MONITOR     = 8,                 /* Monitor mode */
    IEEE80211_M_WDS         = 2,                 /* WDS link */
    IEEE80211_M_BTAMP       = 9,                 /* VAP for BT AMP */

    IEEE80211_M_P2P_GO      = 33,                /* P2P GO */
    IEEE80211_M_P2P_CLIENT  = 34,                /* P2P Client */
    IEEE80211_M_P2P_DEVICE  = 35,                /* P2P Device */


    IEEE80211_OPMODE_MAX    = IEEE80211_M_BTAMP, /* Highest numbered opmode in the list */

    IEEE80211_M_ANY         = 0xFF               /* Any of the above; used by NDIS 6.x */
};

/*
 * 802.11n
 */
#define IEEE80211_CWM_EXTCH_BUSY_THRESHOLD 30

enum ieee80211_cwm_mode {
    IEEE80211_CWM_MODE20,
    IEEE80211_CWM_MODE2040,         
    IEEE80211_CWM_MODE40,
    IEEE80211_CWM_MODEMAX

};

enum ieee80211_cwm_extprotspacing {
    IEEE80211_CWM_EXTPROTSPACING20,
    IEEE80211_CWM_EXTPROTSPACING25,
    IEEE80211_CWM_EXTPROTSPACINGMAX
};

enum ieee80211_cwm_width {
    IEEE80211_CWM_WIDTH20,
    IEEE80211_CWM_WIDTH40,
    IEEE80211_CWM_WIDTH80,
    IEEE80211_CWM_WIDTH160,
    IEEE80211_CWM_WIDTH80_80,
    IEEE80211_CWM_WIDTHINVALID = 0xff    /* user invalid value */
};

enum ieee80211_cwm_extprotmode {
    IEEE80211_CWM_EXTPROTNONE,      /* no protection */
    IEEE80211_CWM_EXTPROTCTSONLY,   /* CTS to self */
    IEEE80211_CWM_EXTPROTRTSCTS,    /* RTS-CTS */
    IEEE80211_CWM_EXTPROTMAX
};

enum ieee80211_fixed_rate_mode {
    IEEE80211_FIXED_RATE_NONE  = 0,
    IEEE80211_FIXED_RATE_MCS   = 1,  /* HT rates */
    IEEE80211_FIXED_RATE_LEGACY  = 2,   /* legacy rates */
    IEEE80211_FIXED_RATE_VHT  = 3   /* VHT rates */
};

/* Holds the fixed rate information for each VAP */
struct ieee80211_fixed_rate {
    enum ieee80211_fixed_rate_mode  mode;
    u_int32_t                       series;
    u_int32_t                       retries;
};

/*
 * 802.11g protection mode.
 */
enum ieee80211_protmode {
    IEEE80211_PROT_NONE     = 0,    /* no protection */
    IEEE80211_PROT_CTSONLY  = 1,    /* CTS to self */
    IEEE80211_PROT_RTSCTS   = 2,    /* RTS-CTS */
};

/*
 * Roaming mode is effectively who controls the operation
 * of the 802.11 state machine when operating as a station.
 * State transitions are controlled either by the driver
 * (typically when management frames are processed by the
 * hardware/firmware), the host (auto/normal operation of
 * the 802.11 layer), or explicitly through ioctl requests
 * when applications like wpa_supplicant want control.
 */
enum ieee80211_roamingmode {
    IEEE80211_ROAMING_DEVICE= 0,    /* driver/hardware control */
    IEEE80211_ROAMING_AUTO  = 1,    /* 802.11 layer control */
    IEEE80211_ROAMING_MANUAL= 2,    /* application control */
};

/*
 * Scanning mode controls station scanning work; this is
 * used only when roaming mode permits the host to select
 * the bss to join/channel to use.
 */
enum ieee80211_scanmode {
    IEEE80211_SCAN_DEVICE   = 0,    /* driver/hardware control */
    IEEE80211_SCAN_BEST     = 1,    /* 802.11 layer selects best */
    IEEE80211_SCAN_FIRST    = 2,    /* take first suitable candidate */
};

/*
 * Channels are specified by frequency and attributes.
 */
struct ieee80211_channel {
    u_int16_t       ic_freq;        /* setting in Mhz */
    u_int32_t       ic_flags;       /* see below */
    u_int16_t       ic_flagext;     /* see below */
    u_int8_t        ic_ieee;        /* IEEE channel number */
    int8_t          ic_maxregpower; /* maximum regulatory tx power in dBm */
    int8_t          ic_maxpower;    /* maximum tx power in dBm */
    int8_t          ic_minpower;    /* minimum tx power in dBm */
    u_int8_t        ic_regClassId;  /* regClassId of this channel */ 
    u_int8_t        ic_antennamax;  /* antenna gain max from regulatory */
    u_int8_t        ic_vhtop_ch_freq_seg1;         /* Channel Center frequency */
    u_int8_t        ic_vhtop_ch_freq_seg2;         /* Channel Center frequency applicable
                                                  * for 80+80MHz mode of operation */ 
};

struct ieee80211_vap_info {
    struct ieee80211_channel *chan;
    u_int8_t no_cac;
};

#define IEE80211_MAX_20M_SUB_CH 8 /* In case of VHT160, we can have 8 20Mhz channels */
struct ieee80211_channel_list {
    int                         cl_nchans;
    struct ieee80211_channel    *cl_channels[IEE80211_MAX_20M_SUB_CH];
};

#define	IEEE80211_NWID_LEN	32
#define IEEE80211_CHAN_MAX      1023
#define IEEE80211_CHAN_BYTES    128      /* howmany(IEEE80211_CHAN_MAX, NBBY) */
#define IEEE80211_CHAN_ANY      (-1)    /* token for ``any channel'' */
#define IEEE80211_CHAN_ANYC \
        ((struct ieee80211_channel *) IEEE80211_CHAN_ANY)

#define IEEE80211_CHAN_DEFAULT          11
#define IEEE80211_CHAN_DEFAULT_11A      52
#define IEEE80211_CHAN_ADHOC_DEFAULT1   10
#define IEEE80211_CHAN_ADHOC_DEFAULT2   11

#define IEEE80211_RADAR_11HCOUNT        5
#define IEEE80211_RADAR_TEST_MUTE_CHAN_11A      36      /* Move to channel 36 for mute test */
#define IEEE80211_RADAR_TEST_MUTE_CHAN_11NHT20  36
#define IEEE80211_RADAR_TEST_MUTE_CHAN_11NHT40U 36
#define IEEE80211_RADAR_TEST_MUTE_CHAN_11NHT40D 40      /* Move to channel 40 for HT40D mute test */
#define IEEE80211_RADAR_DETECT_DEFAULT_DELAY    60000   /* STA ignore AP beacons during this period in millisecond */

#define IEEE80211_2GCSA_TBTTCOUNT        3

/* bits 0-3 are for private use by drivers */
/* channel attributes */
#define IEEE80211_CHAN_TURBO            0x00000010 /* Turbo channel */
#define IEEE80211_CHAN_CCK              0x00000020 /* CCK channel */
#define IEEE80211_CHAN_OFDM             0x00000040 /* OFDM channel */
#define IEEE80211_CHAN_2GHZ             0x00000080 /* 2 GHz spectrum channel. */
#define IEEE80211_CHAN_5GHZ             0x00000100 /* 5 GHz spectrum channel */
#define IEEE80211_CHAN_PASSIVE          0x00000200 /* Only passive scan allowed */
#define IEEE80211_CHAN_DYN              0x00000400 /* Dynamic CCK-OFDM channel */
#define IEEE80211_CHAN_GFSK             0x00000800 /* GFSK channel (FHSS PHY) */
#define IEEE80211_CHAN_RADAR            0x00001000 /* Radar found on channel */
#define IEEE80211_CHAN_STURBO           0x00002000 /* 11a static turbo channel only */
#define IEEE80211_CHAN_HALF             0x00004000 /* Half rate channel */
#define IEEE80211_CHAN_QUARTER          0x00008000 /* Quarter rate channel */
#define IEEE80211_CHAN_HT20             0x00010000 /* HT 20 channel */
#define IEEE80211_CHAN_HT40PLUS         0x00020000 /* HT 40 with extension channel above */
#define IEEE80211_CHAN_HT40MINUS        0x00040000 /* HT 40 with extension channel below */
#define IEEE80211_CHAN_HT40INTOL        0x00080000 /* HT 40 Intolerant */
#define IEEE80211_CHAN_VHT20            0x00100000 /* VHT 20 channel */
#define IEEE80211_CHAN_VHT40PLUS        0x00200000 /* VHT 40 with extension channel above */
#define IEEE80211_CHAN_VHT40MINUS       0x00400000 /* VHT 40 with extension channel below */
#define IEEE80211_CHAN_VHT80            0x00800000 /* VHT 80 channel */
#define IEEE80211_CHAN_HT40INTOLMARK    0x01000000 /* HT 40 Intolerant mark bit for ACS use */
#define IEEE80211_CHAN_BLOCKED          0x02000000 /* channel temporarily blocked due to noise */
#define IEEE80211_CHAN_VHT160           0x04000000 /* VHT 160 channel */
#define IEEE80211_CHAN_VHT80_80         0x08000000 /* VHT 80_80 channel */

/* flagext */
#define	IEEE80211_CHAN_RADAR_FOUND    0x01
#define IEEE80211_CHAN_DFS              0x0002  /* DFS required on channel */
#define IEEE80211_CHAN_DFS_CFREQ2       0x0004  /* DFS required on channel for 2nd band of 80+80*/
#define IEEE80211_CHAN_DFS_CLEAR        0x0008  /* if channel has been checked for DFS */
#define IEEE80211_CHAN_11D_EXCLUDED     0x0010  /* excluded in 11D */
#define IEEE80211_CHAN_CSA_RECEIVED     0x0020  /* Channel Switch Announcement received on this channel */
#define IEEE80211_CHAN_DISALLOW_ADHOC   0x0040  /* ad-hoc is not allowed */
#define IEEE80211_CHAN_DISALLOW_HOSTAP  0x0080  /* Station only channel */
#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
#define IEEE80211_CHAN_HISTORY_RADAR    0x0100  /* DFS radar history for slave device(STA mode) */
#define IEEE80211_CHAN_CAC_VALID        0x0200  /* DFS CAC valid for  slave device(STA mode) */
#endif

/*
 * Useful combinations of channel characteristics.
 */
#define IEEE80211_CHAN_FHSS \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_GFSK)
#define IEEE80211_CHAN_A \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define IEEE80211_CHAN_B \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_CCK)
#define IEEE80211_CHAN_PUREG \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
#define IEEE80211_CHAN_G \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN)
#define IEEE80211_CHAN_108A \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_TURBO)
#define IEEE80211_CHAN_108G \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_TURBO)
#define IEEE80211_CHAN_ST \
    (IEEE80211_CHAN_108A | IEEE80211_CHAN_STURBO)

#define IEEE80211_CHAN_11NG_HT20 \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_HT20)
#define IEEE80211_CHAN_11NA_HT20 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_HT20)
#define IEEE80211_CHAN_11NG_HT40PLUS \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_HT40PLUS)
#define IEEE80211_CHAN_11NG_HT40MINUS \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_HT40MINUS)
#define IEEE80211_CHAN_11NA_HT40PLUS \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_HT40PLUS)
#define IEEE80211_CHAN_11NA_HT40MINUS \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_HT40MINUS)

#define IEEE80211_CHAN_ALL \
    (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_GFSK | \
    IEEE80211_CHAN_CCK | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_DYN | \
    IEEE80211_CHAN_HT20 | IEEE80211_CHAN_HT40PLUS | IEEE80211_CHAN_HT40MINUS | \
    IEEE80211_CHAN_VHT20 | IEEE80211_CHAN_VHT40PLUS | IEEE80211_CHAN_VHT40MINUS | IEEE80211_CHAN_VHT80 | IEEE80211_CHAN_VHT160 | IEEE80211_CHAN_VHT80_80 | \
    IEEE80211_CHAN_HALF | IEEE80211_CHAN_QUARTER)
#define IEEE80211_CHAN_ALLTURBO \
    (IEEE80211_CHAN_ALL | IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)

#define IEEE80211_IS_CHAN_FHSS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_FHSS) == IEEE80211_CHAN_FHSS)
#define IEEE80211_IS_CHAN_A(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)
#define IEEE80211_IS_CHAN_B(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_B) == IEEE80211_CHAN_B)
#define IEEE80211_IS_CHAN_PUREG(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_PUREG) == IEEE80211_CHAN_PUREG)
#define IEEE80211_IS_CHAN_G(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)
#define IEEE80211_IS_CHAN_ANYG(_c) \
    (IEEE80211_IS_CHAN_PUREG(_c) || IEEE80211_IS_CHAN_G(_c))
#define IEEE80211_IS_CHAN_ST(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_ST) == IEEE80211_CHAN_ST)
#define IEEE80211_IS_CHAN_108A(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_108A) == IEEE80211_CHAN_108A)
#define IEEE80211_IS_CHAN_108G(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_108G) == IEEE80211_CHAN_108G)

#define IEEE80211_IS_CHAN_2GHZ(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_2GHZ) != 0)
#define IEEE80211_IS_CHAN_5GHZ(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_5GHZ) != 0)
#define IEEE80211_IS_CHAN_OFDM(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_OFDM) != 0)
#define IEEE80211_IS_CHAN_CCK(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_CCK) != 0)
#define IEEE80211_IS_CHAN_GFSK(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_GFSK) != 0)
#define IEEE80211_IS_CHAN_TURBO(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_TURBO) != 0)
#define IEEE80211_IS_CHAN_WEATHER_RADAR(_c) \
    ((((_c)->ic_freq >= 5600) && ((_c)->ic_freq <= 5650)) \
     || (((_c)->ic_flags & IEEE80211_CHAN_HT40PLUS) && (5580 == (_c)->ic_freq)))
#define IEEE80211_IS_CHAN_STURBO(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_STURBO) != 0)
#define IEEE80211_IS_CHAN_DTURBO(_c) \
    (((_c)->ic_flags & \
    (IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)) == IEEE80211_CHAN_TURBO)
#define IEEE80211_IS_CHAN_HALF(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HALF) != 0)
#define IEEE80211_IS_CHAN_QUARTER(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_QUARTER) != 0)
#define IEEE80211_IS_CHAN_PASSIVE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_PASSIVE) != 0)

#define IEEE80211_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(_c) \
    (IEEE80211_IS_CHAN_DFS(_c->ic_curchan) ||       \
     ((IEEE80211_IS_CHAN_11AC_VHT160(_c->ic_curchan) || IEEE80211_IS_CHAN_11AC_VHT80_80(_c->ic_curchan)) \
      && IEEE80211_IS_CHAN_DFS_CFREQ2(_c->ic_curchan)))

#define IEEE80211_IS_CHAN_DFS(_c) \
    (((_c)->ic_flagext & (IEEE80211_CHAN_DFS|IEEE80211_CHAN_DFS_CLEAR)) == IEEE80211_CHAN_DFS)
#define IEEE80211_IS_CHAN_DFS_CFREQ2(_c) \
    (((_c)->ic_flagext & (IEEE80211_CHAN_DFS_CFREQ2|IEEE80211_CHAN_DFS_CLEAR)) == IEEE80211_CHAN_DFS_CFREQ2)
#define IEEE80211_IS_CHAN_DFSFLAG(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_DFS) == IEEE80211_CHAN_DFS)
#define IEEE80211_IS_CHAN_DFSFLAG_CFREQ2(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_DFS_CFREQ2) == IEEE80211_CHAN_DFS_CFREQ2)
#define IEEE80211_IS_CHAN_DISALLOW_ADHOC(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_DISALLOW_ADHOC) != 0)
#define IEEE80211_IS_CHAN_11D_EXCLUDED(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_11D_EXCLUDED) != 0)
#define IEEE80211_IS_CHAN_CSA(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_CSA_RECEIVED) != 0)
#define IEEE80211_IS_CHAN_ODD(_c) \
    (((_c)->ic_freq == 5170) || ((_c)->ic_freq == 5190) || \
     ((_c)->ic_freq == 5210) || ((_c)->ic_freq == 5230))
#define IEEE80211_IS_CHAN_DISALLOW_HOSTAP(_c) \
    (((_c)->ic_flagext & IEEE80211_CHAN_DISALLOW_HOSTAP) != 0)

#define IEEE80211_IS_CHAN_11NG_HT20(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NG_HT20) == IEEE80211_CHAN_11NG_HT20)
#define IEEE80211_IS_CHAN_11NA_HT20(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NA_HT20) == IEEE80211_CHAN_11NA_HT20)
#define IEEE80211_IS_CHAN_11NG_HT40PLUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NG_HT40PLUS) == IEEE80211_CHAN_11NG_HT40PLUS)
#define IEEE80211_IS_CHAN_11NG_HT40MINUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NG_HT40MINUS) == IEEE80211_CHAN_11NG_HT40MINUS)
#define IEEE80211_IS_CHAN_11NA_HT40PLUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NA_HT40PLUS) == IEEE80211_CHAN_11NA_HT40PLUS)
#define IEEE80211_IS_CHAN_11NA_HT40MINUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11NA_HT40MINUS) == IEEE80211_CHAN_11NA_HT40MINUS)

#define IEEE80211_IS_CHAN_11N(_c) \
    (((_c)->ic_flags & (IEEE80211_CHAN_HT20 | IEEE80211_CHAN_HT40PLUS | IEEE80211_CHAN_HT40MINUS)) != 0)
#define IEEE80211_IS_CHAN_11N_HT20(_c) \
    (((_c)->ic_flags & (IEEE80211_CHAN_HT20)) != 0)
#define IEEE80211_IS_CHAN_11N_HT40(_c) \
    (((_c)->ic_flags & (IEEE80211_CHAN_HT40PLUS | IEEE80211_CHAN_HT40MINUS)) != 0)
#define IEEE80211_IS_CHAN_11NG(_c) \
    (IEEE80211_IS_CHAN_2GHZ((_c)) && IEEE80211_IS_CHAN_11N((_c)))
#define IEEE80211_IS_CHAN_11NA(_c) \
    (IEEE80211_IS_CHAN_5GHZ((_c)) && IEEE80211_IS_CHAN_11N((_c)))
#define IEEE80211_IS_CHAN_11N_HT40PLUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40PLUS) != 0)
#define IEEE80211_IS_CHAN_11N_HT40MINUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40MINUS) != 0)

#define IEEE80211_IS_CHAN_HT20_CAPABLE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT20) == IEEE80211_CHAN_HT20)
#define IEEE80211_IS_CHAN_HT40PLUS_CAPABLE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40PLUS) == IEEE80211_CHAN_HT40PLUS)
#define IEEE80211_IS_CHAN_HT40MINUS_CAPABLE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40MINUS) == IEEE80211_CHAN_HT40MINUS)
#define IEEE80211_IS_CHAN_HT40_CAPABLE(_c) \
    (IEEE80211_IS_CHAN_HT40PLUS_CAPABLE(_c) || IEEE80211_IS_CHAN_HT40MINUS_CAPABLE(_c))
#define IEEE80211_IS_CHAN_HT_CAPABLE(_c) \
    (IEEE80211_IS_CHAN_HT20_CAPABLE(_c) || IEEE80211_IS_CHAN_HT40_CAPABLE(_c))
#define IEEE80211_IS_CHAN_11N_CTL_CAPABLE(_c)  IEEE80211_IS_CHAN_HT20_CAPABLE(_c)
#define IEEE80211_IS_CHAN_11N_CTL_U_CAPABLE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40PLUS) == IEEE80211_CHAN_HT40PLUS)
#define IEEE80211_IS_CHAN_11N_CTL_L_CAPABLE(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_HT40MINUS) == IEEE80211_CHAN_HT40MINUS)
#define IEEE80211_IS_CHAN_11N_CTL_40_CAPABLE(_c) \
    (IEEE80211_IS_CHAN_11N_CTL_U_CAPABLE((_c)) || IEEE80211_IS_CHAN_11N_CTL_L_CAPABLE((_c)))


#define IEEE80211_IS_CHAN_VHT(_c) \
    (((_c)->ic_flags & (IEEE80211_CHAN_VHT20 | \
      IEEE80211_CHAN_VHT40PLUS | IEEE80211_CHAN_VHT40MINUS | IEEE80211_CHAN_VHT80 | IEEE80211_CHAN_VHT160 | IEEE80211_CHAN_VHT80_80)) != 0)
#define IEEE80211_IS_CHAN_11AC(_c) \
    ( IEEE80211_IS_CHAN_5GHZ((_c)) && IEEE80211_IS_CHAN_VHT((_c)) )
#define IEEE80211_CHAN_11AC_VHT20 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT20)
#define IEEE80211_CHAN_11AC_VHT40 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT40PLUS | IEEE80211_CHAN_VHT40MINUS )
#define IEEE80211_CHAN_11AC_VHT40PLUS \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT40PLUS)
#define IEEE80211_CHAN_11AC_VHT40MINUS \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT40MINUS)    
#define IEEE80211_CHAN_11AC_VHT80 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT80)
#define IEEE80211_CHAN_11AC_VHT160 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT160)
#define IEEE80211_CHAN_11AC_VHT80_80 \
    (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT80_80)
#define IEEE80211_IS_CHAN_11AC_VHT20(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT20) == IEEE80211_CHAN_11AC_VHT20)
    
#define IEEE80211_IS_CHAN_11AC_VHT40(_c) \
    (((_c)->ic_flags & (IEEE80211_CHAN_VHT40PLUS | IEEE80211_CHAN_VHT40MINUS)) !=0)
#define IEEE80211_IS_CHAN_11AC_VHT40PLUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT40PLUS) == IEEE80211_CHAN_11AC_VHT40PLUS)
#define IEEE80211_IS_CHAN_11AC_VHT40MINUS(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT40MINUS) == IEEE80211_CHAN_11AC_VHT40MINUS)    
#define IEEE80211_IS_CHAN_11AC_VHT80(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT80) == IEEE80211_CHAN_11AC_VHT80)
#define IEEE80211_IS_CHAN_11AC_VHT160(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT160) == IEEE80211_CHAN_11AC_VHT160)
#define IEEE80211_IS_CHAN_11AC_VHT80_80(_c) \
    (((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT80_80) == IEEE80211_CHAN_11AC_VHT80_80)
#define IEEE80211_CH_HOPPING_SET_CHAN_BLOCKED(_c)    \
    ((_c)->ic_flags |= IEEE80211_CHAN_BLOCKED)
#define IEEE80211_CH_HOPPING_IS_CHAN_BLOCKED(_c)    \
    (((_c)->ic_flags & IEEE80211_CHAN_BLOCKED) == IEEE80211_CHAN_BLOCKED)
#define IEEE80211_CH_HOPPING_CLEAR_CHAN_BLOCKED(_c)    \
    ((_c)->ic_flags &= ~IEEE80211_CHAN_BLOCKED)
#define IEEE80211_IS_CHAN_RADAR(_c)    \
    (((_c)->ic_flags & IEEE80211_CHAN_RADAR) == IEEE80211_CHAN_RADAR)
#define IEEE80211_CHAN_SET_RADAR(_c)    \
    ((_c)->ic_flags |= IEEE80211_CHAN_RADAR)
#define IEEE80211_CHAN_CLR_RADAR(_c)    \
    ((_c)->ic_flags &= ~IEEE80211_CHAN_RADAR)
#define IEEE80211_CHAN_SET_DISALLOW_ADHOC(_c)   \
    ((_c)->ic_flagext |= IEEE80211_CHAN_DISALLOW_ADHOC)
#define IEEE80211_CHAN_SET_DISALLOW_HOSTAP(_c)   \
    ((_c)->ic_flagext |= IEEE80211_CHAN_DISALLOW_HOSTAP)
#define IEEE80211_CHAN_SET_DFS(_c)  \
    ((_c)->ic_flagext |= IEEE80211_CHAN_DFS)
#define IEEE80211_CHAN_SET_DFS_CLEAR(_c)  \
    ((_c)->ic_flagext |= IEEE80211_CHAN_DFS_CLEAR)
#define IEEE80211_CHAN_EXCLUDE_11D(_c)  \
    ((_c)->ic_flagext |= IEEE80211_CHAN_11D_EXCLUDED)

#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
#define IEEE80211_IS_CHAN_HISTORY_RADAR(_c)    \
    (((_c)->ic_flagext & IEEE80211_CHAN_HISTORY_RADAR) == IEEE80211_CHAN_HISTORY_RADAR)
#define IEEE80211_CHAN_SET_HISTORY_RADAR(_c)    \
    ((_c)->ic_flagext |= IEEE80211_CHAN_HISTORY_RADAR)
#define IEEE80211_CHAN_CLR_HISTORY_RADAR(_c)    \
    ((_c)->ic_flagext &= ~IEEE80211_CHAN_HISTORY_RADAR)

#define IEEE80211_IS_CHAN_CAC_VALID(_c)    \
    (((_c)->ic_flagext & IEEE80211_CHAN_CAC_VALID) == IEEE80211_CHAN_CAC_VALID)
#define IEEE80211_CHAN_SET_CAC_VALID(_c)    \
    ((_c)->ic_flagext |= IEEE80211_CHAN_CAC_VALID)
#define IEEE80211_CHAN_CLR_CAC_VALID(_c)    \
    ((_c)->ic_flagext &= ~IEEE80211_CHAN_CAC_VALID)
#endif

/* channel encoding for FH phy */
#define IEEE80211_FH_CHANMOD            80
#define IEEE80211_FH_CHAN(set,pat)      (((set)-1)*IEEE80211_FH_CHANMOD+(pat))
#define IEEE80211_FH_CHANSET(chan)      ((chan)/IEEE80211_FH_CHANMOD+1)
#define IEEE80211_FH_CHANPAT(chan)      ((chan)%IEEE80211_FH_CHANMOD)

/*
 * 802.11 rate set.
 */
#define IEEE80211_RATE_SIZE     8               /* 802.11 standard */
#define IEEE80211_RATE_MAXSIZE  44              /* max rates we'll handle */
#define IEEE80211_HT_RATE_SIZE  128
#define IEEE80211_RATE_SINGLE_STREAM_MCS_MAX     7  /* MCS7 */

#define IEEE80211_RATE_MCS      0x8000
#define IEEE80211_RATE_MCS_VAL  0x7FFF

#define IEEE80211_RATE_IDX_ENTRY(val, idx) (((val&(0xff<<(idx*8)))>>(idx*8)))

/*
 * RSSI range
 */
#define IEEE80211_RSSI_MAX           -10   /* in db */
#define IEEE80211_RSSI_MIN           -200

/*
 * 11n A-MPDU & A-MSDU limits
 */
#define IEEE80211_AMPDU_LIMIT_MIN           (1 * 1024)
#define IEEE80211_AMPDU_LIMIT_MAX           (64 * 1024 - 1)
#define IEEE80211_AMPDU_LIMIT_DEFAULT       IEEE80211_AMPDU_LIMIT_MAX
#define IEEE80211_AMPDU_SUBFRAME_MIN        2 
#define IEEE80211_AMPDU_SUBFRAME_MAX        64 
#define IEEE80211_AMPDU_SUBFRAME_DEFAULT    32 
#define IEEE80211_AMSDU_LIMIT_MAX           4096
#define IEEE80211_RIFS_AGGR_DIV             10
#define IEEE80211_MAX_AMPDU_MIN             0
#define IEEE80211_MAX_AMPDU_MAX             3

/*
 * 11ac A-MPDU limits
 */
#define IEEE80211_VHT_MAX_AMPDU_MIN         0
#define IEEE80211_VHT_MAX_AMPDU_MAX         7

#define WAPI_MCAST_SUITE_POLICY     BIT(0)
#define WAPI_UCAST_SUITE_POLICY     BIT(1)
#define WAPI_KEYMGT_ALGS_POLICY     (BIT(2)|BIT(3))
#define WAPI_PREAUTH_POLICY         BIT(4)

/*
 * Chipsets for which 80+80 EMI WAR is applicable.
 */
#define IS_EMIWAR_80P80_APPLICABLE(scn) (((scn)->target_type == TARGET_TYPE_QCA9984) || ((scn)->target_type == TARGET_TYPE_QCA9888))

enum emiwar_80p80_mode {
    EMIWAR_80P80_DISABLE    = 0,    /* disable EMIWAR 80P80 */
    EMIWAR_80P80_BANDEDGE   = 1,    /* Enable EMIWAR for BandEdge(i.e fc1:5775, and fc2:5210) frequency limitation */
    EMIWAR_80P80_FC1GTFC2   = 2,    /* Enable EMIWAR for fc1 > fc2 frequency limitation */
    EMIWAR_80P80_MAX,
};
/* Default value of 80+80 EMI WAR for QCA9984 */
#define EMIWAR_80P80_DEFVAL_QCA9984     (EMIWAR_80P80_DISABLE)
/* Default value of 80+80 EMI WAR for QCA9888 */
#define EMIWAR_80P80_DEFVAL_QCA9888     (EMIWAR_80P80_FC1GTFC2)

/* Max Number of supported class/channel for MBO */
#define MAX_NUM_CHANNEL_SUPPORTED       255
#define MAX_NUM_OPCLASS_SUPPORTED       64

 /*
  * Check if revised signalling is being used for VHT160 in vhtop
  */
#define IS_REVSIG_VHT160(vhtop) (((vhtop)->vht_op_chwidth == IEEE80211_VHTOP_CHWIDTH_REVSIG_160) && \
        ((vhtop)->vht_op_ch_freq_seg2 != 0) && \
        (abs((vhtop)->vht_op_ch_freq_seg2 - (vhtop)->vht_op_ch_freq_seg1) == 8))

/*
 * Check if revised signalling is being used for VHT80p80 in vhtop
 */
#define IS_REVSIG_VHT80_80(vhtop) (((vhtop)->vht_op_chwidth == IEEE80211_VHTOP_CHWIDTH_REVSIG_80_80) && \
        ((vhtop)->vht_op_ch_freq_seg2 != 0) && \
        (abs((vhtop)->vht_op_ch_freq_seg2 - (vhtop)->vht_op_ch_freq_seg1) > 16))

#define HTINFO_CCFS2_GET(htinfo) ((((htinfo)->hi_ccfs2_2)  << IEEE80211_HTINFO_CCFS2_GET_S) | (htinfo)->hi_ccfs2_1)
#define HTINFO_CCFS2_SET(x, htinfo) (htinfo)->hi_ccfs2_1 = ((x) & 0x7); (htinfo)->hi_ccfs2_2 = (((x) >> IEEE80211_HTINFO_CCFS2_SET_S) & 0x1F)

struct ieee80211_rateset {
    u_int8_t                rs_nrates;
    u_int8_t                rs_rates[IEEE80211_RATE_MAXSIZE];
};

#define IEEE80211_MBO_CHAN_BITMAP_WORD_SIZE  (32) /* needs to be a power of 2 */
#define IEEE80211_MBO_CHAN_BITMAP_LOG2_WORD_SIZE 5 /* log2(32) == 5 */
#define IEEE80211_MBO_CHAN_BITMAP_WORD_MASK (IEEE80211_MBO_CHAN_BITMAP_WORD_SIZE-1)
/* IEEE80211_MBO_CHAN_BITMAP_SET, IEEE80211_MBO_CHAN_BITMAP_CLR -
 * Find the word by dividing the index by the number of bits per word.
 * Find the bit within the word by masking out all but the LSBs of the index.
 * Set or clear the relevant bit within the relevant word.
 */
#define IEEE80211_MBO_CHAN_BITMAP_SET(bitmap, chan)           \
    (bitmap[chan >> IEEE80211_MBO_CHAN_BITMAP_LOG2_WORD_SIZE] |= \
    (1 << (chan & IEEE80211_MBO_CHAN_BITMAP_WORD_MASK)))
#define IEEE80211_MBO_CHAN_BITMAP_CLR(bitmap, chan)              \
    (bitmap[chan >> IEEE80211_MBO_CHAN_BITMAP_LOG2_WORD_SIZE] &= \
    ~(1 << (chan & IEEE80211_MBO_CHAN_BITMAP_WORD_MASK)))
#define IEEE80211_MBO_CHAN_BITMAP_IS_SET(bitmap, chan)    \
    ((bitmap[chan >> IEEE80211_MBO_CHAN_BITMAP_LOG2_WORD_SIZE] & \
    (1 << (chan & IEEE80211_MBO_CHAN_BITMAP_WORD_MASK))) != 0)
/*
 * We expect IEEE80211_MBO_CHAN_WORDS to be a multiple of IEEE80211_MBO_CHAN_BITMAP_WORD_SIZE,
 * but just in case it isn't, round up.
 */
#define IEEE80211_MBO_CHAN_WORDS \
    ((IEEE80211_CHAN_MAX + IEEE80211_MBO_CHAN_BITMAP_WORD_SIZE - 1) / IEEE80211_MBO_CHAN_BITMAP_WORD_SIZE)

/* MBO :- supported class */
struct ieee80211_supp_op_class {
    u_int8_t               curr_op_class;
    u_int8_t               num_of_supp_class;
    u_int8_t               supp_class[MAX_NUM_OPCLASS_SUPPORTED];
    u_int8_t               num_chan_supported;
    u_int32_t              channels_supported[IEEE80211_MBO_CHAN_WORDS];
};
struct ieee80211_beacon_info{
    u_int8_t    essid[IEEE80211_NWID_LEN+1];
    u_int8_t    esslen;
    u_int8_t	rssi_ctl_0;
    u_int8_t	rssi_ctl_1;
    u_int8_t	rssi_ctl_2;
    int         numchains;
};

struct ieee80211_ibss_peer_list{
    u_int8_t    bssid[6];
};

struct ieee80211_roam {
    int8_t                  rssi11a;        /* rssi thresh for 11a bss */
    int8_t                  rssi11b;        /* for 11g sta in 11b bss */
    int8_t                  rssi11bOnly;    /* for 11b sta */
    u_int8_t                pad1;
    u_int8_t                rate11a;        /* rate thresh for 11a bss */
    u_int8_t                rate11b;        /* for 11g sta in 11b bss */
    u_int8_t                rate11bOnly;    /* for 11b sta */
    u_int8_t                pad2;
};

#define IEEE80211_TID_SIZE      17 /* total number of TIDs */
#define IEEE80211_NON_QOS_SEQ   16 /* index for non-QoS (including management) sequence number space */
#define IEEE80211_SEQ_MASK      0xfff   /* sequence generator mask*/
#define MIN_SW_SEQ              0x100   /* minimum sequence for SW generate packect*/

#define IEEE80211_ADDR_LEN  6       /* size of 802.11 address */
#define IEEE80211_SHORT_SSID_LEN 4   /* size of shortssid length */

/* crypto related defines*/
#define IEEE80211_KEYBUF_SIZE   32
#if SENAO_SUPPORT_NAWDS_AES
#define IEEE80211_KEYBUF_SIZE_NAWDS_AES   16
#endif
#define IEEE80211_MICBUF_SIZE   (8+8)   /* space for both tx+rx keys */
#define IEEE80211_KEY_WEP40_LEN  5    /* 40 bit key */
#define IEEE80211_KEY_WEP104_LEN  13  /* 104 bit key */
#define IEEE80211_KEY_WEP128_LEN  16  /* 128 bit key */

#if SENAO_SUPPORT_MESH_EASYSETUP
#define ENGENIUS_NAWDS_EASYSETUP_PWD_LEN   66//28
#endif

enum ieee80211_clist_cmd {
    CLIST_UPDATE,
    CLIST_DFS_UPDATE,
    CLIST_NEW_COUNTRY,
    CLIST_NOL_UPDATE
};

enum ieee80211_nawds_param {
    IEEE80211_NAWDS_PARAM_NUM = 0,
    IEEE80211_NAWDS_PARAM_MODE,
    IEEE80211_NAWDS_PARAM_DEFCAPS,
    IEEE80211_NAWDS_PARAM_OVERRIDE,
#if SENAO_SUPPORT_NAWDS_MESH
#if UMAC_SUPPORT_NAWDS //senao add, rd3's nawds patch for mesh network
    //Fixed 2014/11 add for WDS Mesh auto generate WDS Node
    IEEE80211_NAWDS_PARAM_ENWDSCOUNT,
    IEEE80211_NAWDS_PARAM_ENWDSCOUNTDOWN,
    IEEE80211_NAWDS_PARAM_ENWDSDEFAESKEY,
    IEEE80211_NAWDS_PARAM_ENWDSCHECKTIMER,
#if SENAO_SUPPORT_MESH_TQ_ASSIST
    //Fixed 2015/07 Add batctl TQ Calculate Assist
    IEEE80211_NAWDS_PARAM_ENWDSTQASSIST,
#endif
#endif
#if SENAO_SUPPORT_MESH_EASYSETUP
    IEEE80211_NAWDS_PARAM_ENWDS_PWD,
    IEEE80211_NAWDS_PARAM_ESINFO,
    IEEE80211_NAWDS_PARAM_ENCRYTYPE,
    IEEE80211_NAWDS_PARAM_WEPKEYID,
    IEEE80211_NAWDS_PARAM_WEPKEYLEN,
#endif
#endif
};

struct ieee80211_mib_cycle_cnts {
    u_int32_t   tx_frame_count;
    u_int32_t   rx_frame_count;
    u_int32_t   rx_clear_count;
    u_int32_t   cycle_count;
    u_int8_t    is_rx_active;
    u_int8_t    is_tx_active;
    u_int32_t   my_bss_rx_cycle_count;
};

struct ieee80211_chanutil_info {
    u_int32_t    rx_clear_count;
    u_int32_t    cycle_count;
    u_int32_t    beacon_count;
    u_int8_t     value;
    u_int8_t     beacon_intervals;
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
    // Flag indicating if the util value is new since the last read
    u_int8_t     new_value;
#endif
#if SENAO_FEATURE_ADD
    u_int8_t     sn_value;
#endif
};

enum ieee80211_vendor_ie_param {
    IEEE80211_VENDOR_IE_PARAM_ADD = 0,
    IEEE80211_VENDOR_IE_PARAM_UPDATE,
    IEEE80211_VENDOR_IE_PARAM_REMOVE,
    IEEE80211_VENDOR_IE_PARAM_LIST,
};

enum ieee80211_nac_param {
    IEEE80211_NAC_PARAM_ADD = 1,
    IEEE80211_NAC_PARAM_DEL ,
    IEEE80211_NAC_PARAM_LIST ,
};
enum ieee80211_nac_rssi_param {
    IEEE80211_NAC_RSSI_PARAM_ADD = 1,
    IEEE80211_NAC_RSSI_PARAM_DEL ,
    IEEE80211_NAC_RSSI_PARAM_LIST ,
};
static __inline int
ieee80211_check_weather_radar_channel(struct ieee80211_channel *chan)
{
    int32_t mode_mask = (IEEE80211_CHAN_11NA_HT20 |
            IEEE80211_CHAN_11NA_HT40PLUS |
            IEEE80211_CHAN_11NA_HT40MINUS |
            IEEE80211_CHAN_11AC_VHT20 |
            IEEE80211_CHAN_11AC_VHT40PLUS |
            IEEE80211_CHAN_11AC_VHT40MINUS |
            IEEE80211_CHAN_11AC_VHT80 |
            IEEE80211_CHAN_11AC_VHT160 |
            IEEE80211_CHAN_11AC_VHT80_80);

    switch ((chan->ic_flags) & mode_mask)
    {
        case IEEE80211_CHAN_11NA_HT40PLUS:
        case IEEE80211_CHAN_11AC_VHT40PLUS:
        case IEEE80211_CHAN_11NA_HT40MINUS:
        case IEEE80211_CHAN_11AC_VHT40MINUS:
        case IEEE80211_CHAN_11AC_VHT80:
        case IEEE80211_CHAN_11AC_VHT80_80:
            return ((chan->ic_freq >= 5580) && (chan->ic_freq <= 5650));

        case IEEE80211_CHAN_11AC_VHT160:
            return ((chan->ic_freq >= 5500) && (chan->ic_freq <= 5650));

        case IEEE80211_CHAN_11NA_HT20:
        case IEEE80211_CHAN_11AC_VHT20:
        default: /* neither HT40+ nor HT40-, finish this call */
            return ((chan->ic_freq >= 5600) && (chan->ic_freq <= 5650));
    }

    return 0;

}

u_int8_t ieee80211_sec_chan_offset(struct ieee80211_channel *chan);

#define ATF_DISABLED                         0
#define ATF_AIRTIME_BASED                    1
#define ATF_TPUT_BASED                       2

#endif /* __NET80211__IEEE80211_H_ */
