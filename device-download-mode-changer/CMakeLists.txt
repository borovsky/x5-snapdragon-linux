cmake_minimum_required (VERSION 2.8)
project (FirmwareUploader)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(GLIB REQUIRED COMPONENTS gio gobject)
find_package(QmiGlib REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Werror")

include_directories(${QMI_GLIB_INCLUDE_DIR} ${GLIB_INCLUDE_DIRS})

add_executable(x5-change-download-mode x5-change-download-mode.c
  dms-set-download-mode.c qmi-message.c)

target_link_libraries(x5-change-download-mode ${QMI_GLIB_LIBRARIES}
  ${GLIB_LIBRARIES} ${GLIB_GIO_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES})
