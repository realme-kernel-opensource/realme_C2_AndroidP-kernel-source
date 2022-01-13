#
# Copyright (C) 2016 MediaTek Inc.
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

subdir-ccflags-$(CONFIG_MTK_CCU) += -DMTK_VPU_KERNEL
subdir-ccflags-$(CONFIG_MTK_CCU) += -DCCU_AF_ENABLE

platform = $(subst ",,$(CONFIG_MTK_PLATFORM))

obj-$(CONFIG_MTK_CCU) += $(subst ",,$(platform))/

$(warning CCU_MAKE_FILE_CALLED)
ifneq (,$(filter $(platform),mt6757 mt6799))
    obj-$(CONFIG_MTK_CCU) += 1.0/
else ifneq (,$(filter $(platform),mt6758 mt6763 mt6765))
    obj-y += 1.1/
else ifneq (,$(filter $(platform),mt6771 mt6775))
    obj-y += 2.0/
else ifneq (,$(filter $(platform),mt6779))
    obj-y += 2.1/
    $(warning CCU_MAKE_FILE_CALLED_2_1)

endif
