SUBTARGET:=infinity6
BOARDNAME:=SAV528
FEATURES:=ubifs squashfs jffs2
CPU_TYPE:=cortex-a7

#KERNELNAME:=zImage Image dtbs

LINUX_VERSION:=4.9.84
#LINUX_SRC_VERSION:=4.9.84

CONFIG_KERNEL_GIT_LOCAL_REPOSITORY=""
CONFIG_KERNEL_GIT_CLONE_URI:="git@atlantis.senao.com/senao_codeaurora/SigmaStar/SAV528/kernel.git"
CONFIG_KERNEL_GIT_URI:="git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV528/kernel.git"
CONFIG_KERNEL_GIT_BRANCH="master"
#CONFIG_KERNEL_GIT_REVISION="187f00df199084e0a971a8a893fa13d37daa0b0c"

define Target/Description
	Build firmware images for SigmaStar DoorBell & AC Camera - SAV528
endef

define Kernel/CompileModules/Default_Sstar
	rm -f $(LINUX_DIR)/vmlinux $(LINUX_DIR)/System.map
	+$(MAKE) $(SSTAR_KERNEL_MAKEOPTS) modules
endef

define Kernel/CompileImage/Default_Sstar
	rm -f $(TARGET_DIR)/init
	+$(MAKE) $(SSTAR_KERNEL_MAKEOPTS)
	$(call Kernel/CopyImage)
endef

