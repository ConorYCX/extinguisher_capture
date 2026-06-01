# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${EXTRA_COMPONENT_DIRS} components/components"
"${EXTRA_COMPONENT_DIRS} components/components/usb/iot_usbh"
"${EXTRA_COMPONENT_DIRS} components/components/usb/iot_usbh_cdc"
"${EXTRA_COMPONENT_DIRS} components/components/usb/iot_usbh_modem"
"${EXTRA_COMPONENT_DIRS} components/components/camera/cam_driver"
"${EXTRA_COMPONENT_DIRS} components/components/audio_camera"
)
# include($ENV{IOT_SOLUTION_PATH}/component.cmake)
add_compile_options(-fdiagnostics-color=always) # 编译时候设置彩色输出
project(audioCamera)