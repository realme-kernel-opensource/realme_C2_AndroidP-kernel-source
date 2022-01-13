subdir-ccflags-y += -Werror
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
# In case the platform does NOT support this type of sensors
obj-y   +=  mtk_tpd.o
obj-y   +=  tpd_button.o
obj-y   +=  tpd_calibrate.o
obj-y   +=  tpd_debug.o
obj-y   +=  tpd_default.o
obj-y   +=  tpd_misc.o
obj-y   +=  tpd_setting.o
obj-$(CONFIG_TOUCHSCREEN_MTK_SOLOMON)	+=  SOLOMON/
#ifndef  ODM_HQ_EDIT
#zhangyin@ODM_HQ.BSP.TP  2018/11/29 add for TP drivers
#obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH_TS)	+=  focaltech_touch/
#endif #ODM_HQ_EDIT
obj-$(CONFIG_TOUCHSCREEN_MTK_GT1151)	+=  GT1151/
obj-$(CONFIG_TOUCHSCREEN_MTK_GT911)	+=  GT911/
obj-$(CONFIG_TOUCHSCREEN_MTK_GSLX680)	+=  gslX680/
obj-$(CONFIG_TOUCHSCREEN_MTK_NT36xxx)	+=  NT36xxx/
#ifdef  ODM_HQ_EDIT
#Wenchao.Du@ODM_HQ.BSP.TP  2019/01/17 add for TP drivers
obj-$(CONFIG_TOUCHSCREEN_MTK_GT9XX)		+=  gt9xx_mtk/
#zhangyin@ODM_HQ.BSP.TP  2018/11/29 add for TP drivers
obj-$(CONFIG_TOUCHSCREEN_MTK_FOCALTECH_TS)	+=  ft5446/
obj-$(CONFIG_TOUCHSCREEN_HIMAX_CHIPSET)		+=  hx8527/

#endif #ODM_HQ_EDIT
