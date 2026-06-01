COMPONENT_ADD_INCLUDEDIRS := cam_driver/include conversions/include
COMPONENT_PRIV_INCLUDEDIRS := cam_driver/private_include conversions/private_include sensors/private_include target/private_include
COMPONENT_SRCDIRS := cam_driver conversions sensors target target/esp32
CXXFLAGS += -fno-rtti
