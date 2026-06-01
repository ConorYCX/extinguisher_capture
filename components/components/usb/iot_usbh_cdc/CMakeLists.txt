if(0)
    idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "include"
    REQUIRES iot_usbh
    PRIV_REQUIRES esp_ringbuf)

    include(cmake_utilities)
    cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})

else()
    idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "include"
    REQUIRES iot_usbh
    PRIV_REQUIRES esp_ringbuf)

    set(LIBS libiot_usbh_cdc)
    add_library(${LIBS} STATIC IMPORTED)
    set_property(TARGET ${LIBS} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${LIBS}.a)
    target_link_libraries(${COMPONENT_LIB} INTERFACE ${LIBS})
    set_property(TARGET ${LIBS} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${COMPONENT_LIB})
endif()