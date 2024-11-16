
# Add the library
add_library(NanoGraph STATIC
    
    NanoGraph.c
)

target_include_directories(NanoGraph PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../NanoDraw/
)

target_link_libraries(NanoGraph 

)