if(0)
    set(srcs "src/esp_modem.c"
        "src/esp_modem_dce_command_lib.c"
        "src/esp_modem_dce_common_commands.c"
        "src/esp_modem_dce.c"
        "src/esp_modem_usb_dte.c"
        "src/esp_modem_netif.c"
        "src/esp_modem_recov_helper.c"
        "src/usbh_modem_board.c"
        "src/usbh_modem_wifi.c")

    idf_component_register(SRCS "${srcs}"
                        INCLUDE_DIRS "include"
                        PRIV_INCLUDE_DIRS "private_include"
                        REQUIRES driver esp_netif esp_wifi esp_http_client
                        PRIV_REQUIRES iot_usbh_cdc esp_event )

    include(cmake_utilities)
    cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})

else()
    idf_component_register(SRCS "" "../iot_usbh/iot_usbh.c"
    INCLUDE_DIRS 
    "include" 
    "../iot_usbh_cdc/include" 
    "../iot_usbh/include" 
    "../include/esp_private" 
    "../include/usb" 
    "../private_include"
    PRIV_INCLUDE_DIRS "private_include"
    REQUIRES driver esp_netif esp_wifi esp_http_client
    PRIV_REQUIRES iot_usbh_cdc esp_event)

    set(LIBS libiot_usbh_modem)
    add_library(${LIBS} STATIC IMPORTED)
    set_property(TARGET ${LIBS} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${LIBS}.a)
    target_link_libraries(${COMPONENT_LIB} INTERFACE ${LIBS})
    set_property(TARGET ${LIBS} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${COMPONENT_LIB})
endif()