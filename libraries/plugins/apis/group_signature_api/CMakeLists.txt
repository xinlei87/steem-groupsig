file(GLOB HEADERS "include/steem/plugins/group_signature_api/*.hpp")
add_library( group_signature_api_plugin
             group_signature_api_plugin.cpp
             group_signature_api.cpp
             ${HEADERS}
            )
target_link_libraries( group_signature_api_plugin group_signature_plugin  json_rpc_plugin )
target_include_directories( group_signature_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if( CLANG_TIDY_EXE )
   set_target_properties(
       group_signature_api_plugin PROPERTIES
       CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

install( TARGETS
    group_signature_api_plugin
    
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)