define KernelPackage/usb-phy-qcom-dwc3
  TITLE:=DWC3 USB QCOM PHY driver
  DEPENDS:=@TARGET_ipq
  KCONFIG:= CONFIG_PHY_QCOM_DWC3
  FILES:= $(LINUX_DIR)/drivers/phy/phy-qcom-dwc3.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qcom-dwc3,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-qcom-dwc3/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the QCOM SoCs.
endef

$(eval $(call KernelPackage,usb-phy-qcom-dwc3))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 \
	qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
define KernelPackage/usb-dwc3-of-simple
  TITLE:=DWC3 USB simple OF driver
  DEPENDS:=+kmod-usb-dwc3
  KCONFIG:= CONFIG_USB_DWC3_OF_SIMPLE
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-of-simple.ko \
	$(LINUX_DIR)/drivers/usb/dwc3/dbm.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-of-simple dbm,1)
  $(call AddDepends/usb)
endef
else
define KernelPackage/usb-dwc3-of-simple
  TITLE:=DWC3 USB simple OF driver
  DEPENDS:=+kmod-usb-dwc3
  KCONFIG:= CONFIG_USB_DWC3_OF_SIMPLE
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-of-simple.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-of-simple,1)
  $(call AddDepends/usb)
endef
endif

define KernelPackage/usb-dwc3-of-simple/description
  This driver provides generic platform glue for the integrated DesignWare USB3 IP Core.
endef

$(eval $(call KernelPackage,usb-dwc3-of-simple))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2))
define KernelPackage/usb-dwc3-qcom
  TITLE:=DWC3 USB QCOM controller driver
  DEPENDS:=@TARGET_ipq||TARGET_ipq806x +kmod-usb-dwc3
  KCONFIG:= CONFIG_USB_DWC3_QCOM
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-qcom.ko \
	  $(LINUX_DIR)/drivers/usb/dwc3/dbm.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-qcom dbm,1)
  $(call AddDepends/usb)
endef
else
define KernelPackage/usb-dwc3-qcom
  TITLE:=DWC3 USB QCOM controller driver
  DEPENDS:=@TARGET_ipq +kmod-usb-dwc3 +kmod-usb-phy-dwc3-qcom
  KCONFIG:= CONFIG_USB_DWC3_QCOM
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-qcom.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-qcom,1)
  $(call AddDepends/usb)
endef
endif

define KernelPackage/usb-dwc3-qcom/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the QCOM SoCs.
endef

$(eval $(call KernelPackage,usb-dwc3-qcom))

define KernelPackage/usb-dwc3-ipq40xx
  TITLE:=DWC3 USB IPQ40XX controller driver
  DEPENDS:=@TARGET_ipq +kmod-usb-dwc3 +kmod-usb-phy-dwc3-ipq40xx
  KCONFIG:= CONFIG_USB_DWC3_IPQ40XX
  FILES:= $(LINUX_DIR)/drivers/usb/dwc3/dwc3-ipq40xx.ko
  AUTOLOAD:=$(call AutoLoad,53,dwc3-ipq40xx,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-dwc3-ipq40xx/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the IPQ40xx SoCs.
endef

$(eval $(call KernelPackage,usb-dwc3-ipq40xx))

define KernelPackage/usb-phy-dwc3-qcom
  TITLE:=DWC3 USB QCOM PHY driver
  DEPENDS:=@TARGET_ipq
  KCONFIG:= CONFIG_USB_QCOM_DWC3_PHY
  FILES:= \
	$(LINUX_DIR)/drivers/usb/phy/phy-qcom-hsusb.ko \
	$(LINUX_DIR)/drivers/usb/phy/phy-qcom-ssusb.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qcom-hsusb phy-qcom-ssusb,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-dwc3-qcom/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the QCOM SoCs.
endef

$(eval $(call KernelPackage,usb-phy-dwc3-qcom))

define KernelPackage/usb-phy-dwc3-ipq40xx
  TITLE:=DWC3 USB IPQ40xx PHY driver
  DEPENDS:=@TARGET_ipq
  KCONFIG:= CONFIG_USB_IPQ40XX_PHY
  FILES:= \
	$(LINUX_DIR)/drivers/usb/phy/phy-qca-baldur.ko \
	$(LINUX_DIR)/drivers/usb/phy/phy-qca-uniphy.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qca-baldur phy-qca-uniphy,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-dwc3-ipq40xx/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the IPQ40xx SoCs.
endef

$(eval $(call KernelPackage,usb-phy-dwc3-ipq40xx))

define KernelPackage/usb-phy-dwc3-ipq4019
  TITLE:=DWC3 USB IPQ4019 PHY driver
  DEPENDS:=@TARGET_ipq +kmod-usb-dwc3-of-simple
  KCONFIG:= CONFIG_PHY_IPQ_BALDUR_USB \
                CONFIG_PHY_IPQ_UNIPHY_USB
  FILES:= \
        $(LINUX_DIR)/drivers/phy/phy-qca-baldur.ko \
        $(LINUX_DIR)/drivers/phy/phy-qca-uniphy.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qca-baldur phy-qca-uniphy,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-phy-dwc3-ipq4019/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the IPQ4019 SoCs.
endef

$(eval $(call KernelPackage,usb-phy-dwc3-ipq4019))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
define KernelPackage/usb-phy-ipq807x
  TITLE:=DWC3 USB QCOM PHY driver for IPQ807x
  DEPENDS:=@TARGET_ipq||TARGET_ipq806x
  KCONFIG:= \
	CONFIG_USB_QCOM_QUSB_PHY \
	CONFIG_USB_QCOM_QMP_PHY
  FILES:= \
	$(LINUX_DIR)/drivers/usb/phy/phy-msm-qusb.ko \
	$(LINUX_DIR)/drivers/usb/phy/phy-msm-ssusb-qmp.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-msm-qusb phy-msm-ssusb-qmp,1)
  $(call AddDepends/usb)
endef
else
define KernelPackage/usb-phy-ipq807x
  TITLE:=DWC3 USB QCOM PHY driver for IPQ807x
  DEPENDS:=@TARGET_ipq +kmod-usb-dwc3-of-simple
  KCONFIG:= \
	CONFIG_USB_QCOM_QUSB_PHY \
	CONFIG_USB_QCOM_QMP_PHY
  FILES:= \
	$(LINUX_DIR)/drivers/usb/phy/phy-msm-qusb.ko \
	$(LINUX_DIR)/drivers/usb/phy/phy-msm-ssusb-qmp.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-msm-qusb phy-msm-ssusb-qmp,1)
  $(call AddDepends/usb)
endef
endif

define KernelPackage/usb-phy-ipq807x/description
 This driver provides support for the USB PHY drivers
 within the IPQ807x SoCs.
endef

$(eval $(call KernelPackage,usb-phy-ipq807x))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 \
	qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2 qca_spf_11_2))
define KernelPackage/qrtr_mproc
  TITLE:= Ath11k Specific kernel configs for IPQ807x and IPQ60xx
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64||TARGET_ipq_ipq50xx||TARGET_ipq_ipq50xx_64
  KCONFIG:= \
	  CONFIG_QRTR=y \
	  CONFIG_QCOM_APCS_IPC=y \
	  CONFIG_QCOM_GLINK_SSR=y \
	  CONFIG_QCOM_Q6V5_WCSS=y \
	  CONFIG_MSM_RPM_RPMSG=y \
	  CONFIG_RPMSG_QCOM_GLINK_RPM=y \
	  CONFIG_REGULATOR_RPM_GLINK=y \
	  CONFIG_QCOM_SYSMON=y \
	  CONFIG_RPMSG=y \
	  CONFIG_RPMSG_CHAR=y \
	  CONFIG_RPMSG_QCOM_GLINK_SMEM=y \
	  CONFIG_RPMSG_QCOM_SMD=y \
	  CONFIG_QRTR_SMD=y \
	  CONFIG_QCOM_QMI_HELPERS=y \
	  CONFIG_SAMPLES=y \
	  CONFIG_SAMPLE_QMI_CLIENT=m \
	  CONFIG_SAMPLE_TRACE_EVENTS=n \
	  CONFIG_SAMPLE_KOBJECT=n \
	  CONFIG_SAMPLE_KPROBES=n \
	  CONFIG_SAMPLE_KRETPROBES=n \
	  CONFIG_SAMPLE_HW_BREAKPOINT=n \
	  CONFIG_SAMPLE_KFIFO=n \
	  CONFIG_SAMPLE_CONFIGFS=n \
	  CONFIG_SAMPLE_RPMSG_CLIENT=n \
	  CONFIG_USB_GADGET=m \
	  CONFIG_USB_CONFIGFS=m \
	  CONFIG_USB_CONFIGFS_F_FS=y \
	  CONFIG_MAILBOX=y \
	  CONFIG_DIAG_OVER_QRTR=y
endef
else
define KernelPackage/qrtr_mproc
  TITLE:= Ath11k Specific kernel configs for IPQ807x
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64
  KCONFIG:= \
	  CONFIG_QRTR=y \
	  CONFIG_QCOM_APCS_IPC=y \
	  CONFIG_QCOM_GLINK_SSR=y \
	  CONFIG_QCOM_Q6V5_WCSS=y \
	  CONFIG_QCOM_SYSMON=y \
	  CONFIG_RPMSG=y \
	  CONFIG_RPMSG_CHAR=y \
	  CONFIG_RPMSG_QCOM_GLINK_SMEM=y \
	  CONFIG_RPMSG_QCOM_SMD=y \
	  CONFIG_QRTR_SMD=y \
	  CONFIG_QCOM_QMI_HELPERS=y \
	  CONFIG_SAMPLES=y \
	  CONFIG_SAMPLE_QMI_CLIENT=m \
	  CONFIG_SAMPLE_TRACE_EVENTS=n \
	  CONFIG_SAMPLE_KOBJECT=n \
	  CONFIG_SAMPLE_KPROBES=n \
	  CONFIG_SAMPLE_KRETPROBES=n \
	  CONFIG_SAMPLE_HW_BREAKPOINT=n \
	  CONFIG_SAMPLE_KFIFO=n \
	  CONFIG_SAMPLE_CONFIGFS=n \
	  CONFIG_SAMPLE_RPMSG_CLIENT=n \
	  CONFIG_USB_GADGET=m \
	  CONFIG_USB_CONFIGFS=m \
	  CONFIG_USB_CONFIGFS_F_FS=y \
	  CONFIG_MAILBOX=y \
	  CONFIG_DIAG_OVER_QRTR=y
endef
endif

define KernelPackage/qrtr_mproc/description
Kernel configs for ath11k support specific to ipq807x.
endef

$(eval $(call KernelPackage,qrtr_mproc))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2 qca_spf_11_2))
define KernelPackage/msm-mproc
  TITLE:= Default kernel configs
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  KCONFIG:= \
	  CONFIG_QRTR=y \
	  CONFIG_QCOM_APCS_IPC=y \
	  CONFIG_QCOM_GLINK_SSR=y \
	  CONFIG_QCOM_Q6V5_WCSS=y \
	  CONFIG_QTI_Q6V5_ADSP=y \
	  CONFIG_MSM_RPM_RPMSG=y \
	  CONFIG_RPMSG_QCOM_GLINK_RPM=y \
	  CONFIG_REGULATOR_RPM_GLINK=y \
	  CONFIG_IPQ_SUBSYSTEM_RESTART=y \
	  CONFIG_QCOM_SYSMON=y \
	  CONFIG_RPMSG=y \
	  CONFIG_RPMSG_CHAR=y \
	  CONFIG_RPMSG_QCOM_GLINK_SMEM=y \
	  CONFIG_RPMSG_QCOM_SMD=y \
	  CONFIG_QRTR_SMD=y \
	  CONFIG_QCOM_QMI_HELPERS=y \
	  CONFIG_SAMPLES=y \
	  CONFIG_SAMPLE_QMI_CLIENT=m \
	  CONFIG_SAMPLE_TRACE_EVENTS=n \
	  CONFIG_SAMPLE_KOBJECT=n \
	  CONFIG_SAMPLE_KPROBES=n \
	  CONFIG_SAMPLE_KRETPROBES=n \
	  CONFIG_SAMPLE_HW_BREAKPOINT=n \
	  CONFIG_SAMPLE_KFIFO=n \
	  CONFIG_SAMPLE_CONFIGFS=n \
	  CONFIG_SAMPLE_RPMSG_CLIENT=n \
	  CONFIG_MAILBOX=y \
	  CONFIG_DIAG_OVER_QRTR=y
endef
else
define KernelPackage/msm-mproc
  TITLE:= Default kernel configs
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  KCONFIG:= \
	  CONFIG_IPC_ROUTER=y \
	  CONFIG_MSM_GLINK=y \
	  CONFIG_MSM_GLINK_SMEM_NATIVE_XPRT=y \
	  CONFIG_MSM_GLINK_PKT=y \
	  CONFIG_MSM_IPC_ROUTER_GLINK_XPRT=y \
	  CONFIG_MSM_QMI_INTERFACE=y \
	  CONFIG_IPQ_SUBSYSTEM_RESTART=y \
	  CONFIG_IPQ807X_REMOTEPROC=y \
	  CONFIG_REGULATOR_RPM_GLINK=y \
	  CONFIG_MSM_IPC_ROUTER_MHI_XPRT=y \
	  CONFIG_MSM_RPM_GLINK=y \
	  CONFIG_MAILBOX=y
endef
endif

define KernelPackage/msm-mproc/description
Default kernel configs.
endef

$(eval $(call KernelPackage,msm-mproc))

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_0_csu1 qca_spf_11_0_csu3))

define KernelPackage/mhi-qrtr-mproc
  TITLE:= Default kernel configs for QCCI to work with QRTR.
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64
  KCONFIG:= \
	  CONFIG_QRTR=y \
	  CONFIG_QRTR_MHI=y \
	  CONFIG_MHI_BUS=y \
	  CONFIG_MHI_QTI=y \
	  CONFIG_MHI_NETDEV=y \
	  CONFIG_MHI_DEBUG=y \
	  CONFIG_MHI_UCI=y
endef

define KernelPackage/mhi-qrtr-mproc/description
Default kernel configs for QCCI to work with QRTR.
endef

$(eval $(call KernelPackage,mhi-qrtr-mproc))

define KernelPackage/qmi_sample_client
  TITLE:= Sample qmi test application.
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64
  KCONFIG:= \
	  CONFIG_SAMPLE_QMI_CLIENT
  FILES:= \
	  $(LINUX_DIR)/samples/qmi/qmi_sample_client.ko
  AUTOLOAD:=$(call AutoLoad,53,qmi_sample_client,1)
endef

define KernelPackage/qmi_sample_client/description
Add QMI ping_pong test application
endef

$(eval $(call KernelPackage,qmi_sample_client))

endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2 qca_spf_11_2))
define KernelPackage/mhi-qrtr-mproc
  TITLE:= Default kernel configs for QCCI to work with QRTR.
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  KCONFIG:= \
	  CONFIG_QRTR=y \
	  CONFIG_QRTR_MHI=y \
	  CONFIG_MHI_BUS=y \
	  CONFIG_MHI_QTI=y \
	  CONFIG_MHI_NETDEV=y \
	  CONFIG_MHI_DEBUG=y \
	  CONFIG_MHI_UCI=y
endef

define KernelPackage/mhi-qrtr-mproc/description
Default kernel configs for QCCI to work with QRTR.
endef

$(eval $(call KernelPackage,mhi-qrtr-mproc))

define KernelPackage/qmi_sample_client
  TITLE:= Sample qmi test application.
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  KCONFIG:= \
	  CONFIG_SAMPLE_QMI_CLIENT
  FILES:= \
	  $(LINUX_DIR)/samples/qmi/qmi_sample_client.ko
  AUTOLOAD:=$(call AutoLoad,53,qmi_sample_client,1)
endef

define KernelPackage/qmi_sample_client/description
Add QMI ping_pong test application
endef

$(eval $(call KernelPackage,qmi_sample_client))
endif

endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_spf_11_1_csu2 qca_spf_11_2))

define KernelPackage/mhitest_mod
  TITLE:= Mhi test module for Pine.
  DEPENDS+= @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  FILES:= \
         $(LINUX_DIR)/drivers/bus/mhi/test/mhitest_mod.ko
endef

define KernelPackage/mhitest_mod/description
ADD mhitest module.
endef

$(eval $(call KernelPackage,mhitest_mod))

define KernelPackage/bt_tty
  TITLE:= BT Inter-processor Communication
  DEPENDS+= @TARGET_ipq_ipq50xx||TARGET_ipq_ipq50xx_64
  KCONFIG:= \
	  CONFIG_QTI_BT_TTY=y \
	  CONFIG_QCOM_MDT_LOADER=y

endef

define KernelPackage/bt_tty/description
BT Interprocessor Communication support specific to IPQ50xx
endef

$(eval $(call KernelPackage,bt_tty))

endif
