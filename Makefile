#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := audioCamera

include $(IDF_PATH)/make/project.mk

EXTRA_COMPONENT_DIRS += $(PROJCT_PATH)/components/components
EXTRA_COMPONENT_DIRS += $(PROJCT_PATH)/components/components/usb/iot_usbh
EXTRA_COMPONENT_DIRS += $(PROJCT_PATH)/components/components/usb/iot_usbh_cdc
EXTRA_COMPONENT_DIRS += $(PROJCT_PATH)/components/components/usb/iot_usbh_modem