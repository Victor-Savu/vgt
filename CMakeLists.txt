
#
# Author: Victor-Nicolae Savu
#
#

cmake_minimum_required (VERSION 2.6)

project(VGT C)

#set(CMAKE_BUILD_TYPE RelWithDebugInfo)
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

#cmake_policy(SET CMP0003 NEW)

set(libtype "SHARED")

#set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3 -Wall -Werror -Winit-self -Wuninitialized -Wstrict-aliasing -fstrict-aliasing")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Werror -Winit-self -Wuninitialized -Wstrict-aliasing -fstrict-aliasing")

link_directories("$ENV{CUSTOM_MODULES_PATH}/lib")
include_directories("$ENV{CUSTOM_MODULES_PATH}/include")
include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

set(INSTALL_LIB_DIR "$ENV{CUSTOM_MODULES_PATH}/lib" CACHE PATH
        "Installation directory for libraries")
set(INSTALL_BIN_DIR "$ENV{CUSTOM_MODULES_PATH}/bin" CACHE PATH
        "Installation directory for executables")
set(INSTALL_INCLUDE_DIR "$ENV{CUSTOM_MODULES_PATH}/include" CACHE PATH
        "Installation directory for header files")

set(math_sources
        math/vec.c
        math/mat.c
        math/frame.c
        math/obj.c
        math/math.c
        math/predicates.c
    )

set(math_headers
        math/vec.h
        math/mat.h
        math/spherical.h
        math/spherical_cls.h
        math/frame.h
        math/frame_cls.h
        math/math.h
        math/predicates.h
    )

add_library(math ${libtype} ${math_sources})
target_link_libraries(math m)

set(ads_sources
        ads/red_black_tree.c
        ads/array.c
        ads/red_black_node.c
        ads/list_element.c
        ads/list.c
        ads/vector.c
    )

set(ads_headers
        ads/red_black_tree.h
        ads/list_element.h
        ads/list.h
        ads/red_black_node.h
        ads/array.h
        ads/vector.h
   )

add_library(ads ${libtype} ${ads_sources})
target_link_libraries(ads m math)

set(vgt_sources
        vgt/volumetric_data.c
        vgt/scalar_field.c
        vgt/vector_field.c
        vgt/mesh.c
        vgt/delaunay.c
        vgt/tet.c
        vgt/victor.c
   )

set(vgt_headers
        vgt/vector_field.h
        vgt/volumetric_data.h
        vgt/types.h
        vgt/scalar_field.h
        vgt/topology.h
        vgt/edge_cls.h
        vgt/mesh.h
        vgt/tet_cls.h
        vgt/tet.h
        vgt/delaunay.h
        vgt/victor.h
        vgt/victor_cls.h
        vgt/half_edge_cls.h
   )

add_library(vgt ${libtype} ${vgt_sources})
target_link_libraries(vgt m math ads glut GL GLU)

set(view_sources
        view/renderer.c
        view/graphics.c
        view/camera.c
    )

set(view_headers
        view/renderer.h
        view/graphics.h
        view/graphics_cls.h
        view/camera.h
        view/camera_cls.h
        view/mouse.h
        view/mouse_cls.h
   )

add_library(view ${libtype} ${view_sources})
target_link_libraries(view vgt pthread)

install(FILES ${vgt_headers} DESTINATION "${INSTALL_INCLUDE_DIR}/vgt")
install(FILES ${ads_headers} DESTINATION "${INSTALL_INCLUDE_DIR}/ads")
install(FILES ${math_headers} DESTINATION "${INSTALL_INCLUDE_DIR}/math")
install(FILES ${view_headers} DESTINATION "${INSTALL_INCLUDE_DIR}/view")

install(TARGETS vgt math view ads DESTINATION ${INSTALL_LIB_DIR})

#add_executable(test_volumetric_data test/test_volumetric_data.c)
#add_executable(test_div test/test_div.c)
#add_executable(test_scalar_field test/test_scalar_field.c)
#add_executable(test_vector_field test/test_vector_field.c)
#add_executable(test_lapl test/test_lapl.c)
#add_executable(test_list test/test_list.c)
add_executable(test_view test/test_view.c)
add_executable(test_mesh test/test_mesh.c)
add_executable(test_array test/test_array.c)
add_executable(test_vector test/test_vector.c)
add_executable(test_delaunay test/test_delaunay.c)

#target_link_libraries(test_volumetric_data m vgt math)
#target_link_libraries(test_div m vgt math)
#target_link_libraries(test_scalar_field m vgt math)
#target_link_libraries(test_vector_field m vgt math)
#target_link_libraries(test_lapl m vgt math)
#target_link_libraries(test_list m vgt math)
target_link_libraries(test_view view)
target_link_libraries(test_mesh vgt)
target_link_libraries(test_array m vgt math rt)
target_link_libraries(test_vector ads rt)
target_link_libraries(test_delaunay vgt view)



