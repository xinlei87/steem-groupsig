file(GLOB HEADERS "include/steem/plugins/group_signature/*.hpp")

add_library( group_signature_plugin
            group_signature.cpp
            #  block_producer.cpp
             ${HEADERS}
           )
#需添加编译参数-lpbc -lgmp
target_link_libraries( group_signature_plugin p2p_plugin chain_plugin rc_plugin appbase steem_chain steem_utilities steem_utilities -lpbc -lgmp )

target_include_directories( group_signature_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )


if( CLANG_TIDY_EXE )
   set_target_properties(
      group_signature_plugin PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

install( TARGETS
   group_signature_plugin

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
