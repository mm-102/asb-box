list(APPEND SourcesC
    ${CMAKE_CURRENT_LIST_DIR}/main.c
)
add_subdirectory(net)
add_subdirectory(peripheral)

apply_source_list()