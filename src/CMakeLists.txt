set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB main_src ${src_dir}/*.cpp)
file(GLOB main_hdr ${src_dir}/*.h)

find_package(Qt4)

set(gui_sources db.cpp main.cpp mega_tray.cpp megatag.cpp)
set(moc_headers mega_tray.h)
set(other_headers db.h megatag.h)

QT4_WRAP_CPP(gui_headers_moc ${moc_headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(main-bin ${gui_sources} ${gui_headers_moc} ${other_headers})
target_link_libraries(main-bin sqlite3 ${QT_LIBRARIES})

set_target_properties(main-bin
	PROPERTIES OUTPUT_NAME megatag)

install(TARGETS main-bin
	RUNTIME DESTINATION bin)
