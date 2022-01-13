#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/ion/mtk
ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/
ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/misc/mediatek/ccu/src/$(MTK_PLATFORM)/
ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/misc/mediatek/ccu/src/$(MTK_PLATFORM)/ccu_ext_interface/
ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/i2c/busses

ccflags-$(CONFIG_MTK_CCU) += -I$(srctree)/drivers/iommu/
$(warning CCU_INC=$(srctree)/drivers/misc/mediatek/ccu/src/$(MTK_PLATFORM)/ccu_ext_interface)

#obj-$(CONFIG_MTK_VPU) += ccu_drv.o ccu_algo.o ccu_dbg.o
obj-$(CONFIG_MTK_CCU) += ccu_drv.o ccu_n3d_a.o ccu_i2c.o
