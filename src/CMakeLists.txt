add_subdirectory(sql)

set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Qt4)

set(common_sources db.cpp megatag.cpp)
set(gui_sources main.cpp mega_tray.cpp)
set(cmd_sources megatool.cpp megatool_main.cpp)

set(common_headers db.h megatag.h)
set(cmd_headers megatool.h)
set(moc_headers mega_tray.h)

QT4_WRAP_CPP(gui_headers_moc ${moc_headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.46.0
	REQUIRED
	COMPONENTS graph)

add_executable(main-bin ${gui_sources} ${common_sources} ${gui_headers_moc} ${common_headers})
target_link_libraries(main-bin sqlite3 ${QT_LIBRARIES})
add_executable(megatool-bin ${cmd_sources} ${common_sources} ${cmd_headers} ${common_headers})
target_link_libraries(megatool-bin sqlite3)

set_target_properties(main-bin
	PROPERTIES OUTPUT_NAME megatag)
set_target_properties(megatool-bin
	PROPERTIES OUTPUT_NAME megatool)

install(TARGETS main-bin megatool-bin
	RUNTIME DESTINATION bin)
