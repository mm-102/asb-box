list(APPEND SourcesC
    ${CMAKE_CURRENT_LIST_DIR}/main.c
)
add_subdirectory(net)
add_subdirectory(drivers)
add_subdirectory(leds)
add_subdirectory(logic)
add_subdirectory(nvs)

apply_source_list()