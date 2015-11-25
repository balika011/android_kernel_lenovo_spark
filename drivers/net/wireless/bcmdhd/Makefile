DHDCFLAGS = \
	-Wall \
	-Wstrict-prototypes \
	-Dlinux \
	-DBCMDRIVER \
	-DBCMDONGLEHOST \
	-DUNRELEASEDCHIP \
	-DBCMDMA32 \
	-DBCMFILEIMAGE \
	-DDHDTHREAD \
	-DSHOW_EVENTS \
	-DBCMDBG \
	-DCUSTOMER_HW2 \
	-DBCMPLATFORM_BUS \
	-DWLP2P \
	-DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT \
	-DKEEP_ALIVE -DGET_CUSTOM_MAC_ENABLE -DPKT_FILTER_SUPPORT \
	-DEMBEDDED_PLATFORM \
	-DENABLE_INSMOD_NO_FW_LOAD \
	-DPNO_SUPPORT \
	-DDHD_USE_IDLECOUNT \
	-DROAM_ENABLE \
	-DVSDB \
	-DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST \
	-DESCAN_RESULT_PATCH \
	-DSUPPORT_PM2_ONLY \
	-DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT \
	-DMIRACAST_AMPDU_SIZE=8 \
	-DWL_CFG80211_GON_COLLISION \
	-DMAX_DTIM_ALLOWED_INTERVAL=300 \
	-Idrivers/net/wireless/bcmdhd \
	-Idrivers/net/wireless/bcmdhd/include \
	-Idrivers/net/wireless/bcmdhd/common/include

DHDOFILES = aiutils.o \
	siutils.o \
	sbutils.o \
	bcmutils.o \
	bcmwifi_channels.o \
	dhd_linux.o \
	dhd_linux_sched.o \
	dhd_pno.o \
	dhd_common.o \
	dhd_ip.o \
	dhd_custom_gpio.o \
	bcmevent.o \
	hndpmu.o \
	linux_osl.o \
	wldev_common.o \
	wl_android.o \

obj-$(CONFIG_BCMDHD) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcmdhd-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP -DWL_WIRELESS_EXT -DUSE_IW
endif
ifneq ($(CONFIG_CFG80211),)
bcmdhd-objs += wl_cfg80211.o wl_cfgp2p.o wl_linux_mon.o dhd_cfg80211.o
DHDCFLAGS += -DWL_CFG80211 -DWL_CFG80211_STA_EVENT
DHDCFLAGS += -DCUSTOM_ROAM_TRIGGER_SETTING=-65
DHDCFLAGS += -DCUSTOM_ROAM_DELTA_SETTING=15
DHDCFLAGS += -DCUSTOM_KEEP_ALIVE_SETTING=10000
DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=7
endif
ifneq ($(CONFIG_DHD_USE_SCHED_SCAN),)
DHDCFLAGS += -DWL_SCHED_SCAN
endif

DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

################################
# INTEL platform specific features
#################################
CONFIG_INTEL_PLATFORM=y
ifneq ($(CONFIG_INTEL_PLATFORM),)
	DHDCFLAGS += -DSUPPORT_AUTO_CHANNEL
	DHDCFLAGS += -DSUPPORT_MULTIPLE_REVISION -DSUPPORT_MULTIPLE_CHIPS
	DHDCFLAGS += -DCUSTOM_LISTEN_INTERVAL=3
	DHDCFLAGS += -DBOARD_INTEL
	DHDCFLAGS += -DUSE_KTHREAD_API
	DHDCFLAGS += -DCUSTOM_RXCHAIN=1
	DHDCFLAGS += -DDHD_USE_EARLYSUSPEND
	DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
endif

#########################
# Chip dependent feature
#########################

# Lenovo Spark (Tab S8-50x)
DHDCFLAGS += -DBCM4330_CHIP

bcmdhd-$(CONFIG_BCMDHD_SDIO) += \
	bcmsdh.o \
	bcmsdh_linux.o \
	bcmsdh_sdmmc.o \
	bcmsdh_sdmmc_linux.o \
	dhd_sdio.o \
	dhd_cdc.o \
	dhd_wlfc.o

ccflags-$(CONFIG_BCMDHD_SDIO) += \
	-DBDC \
	-DDHD_BCMEVENTS \
	-DPROP_TXSTATUS \
	-DPROP_TXSTATUS_VSDB \
	-DOOB_INTR_ONLY \
	-DHW_OOB \
	-DMMC_SDIO_ABORT \
	-DBCMSDIO \
	-DBCMLXSDMMC \

EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCMDHD),m)
EXTRA_LDFLAGS += --strip-debug
endif
