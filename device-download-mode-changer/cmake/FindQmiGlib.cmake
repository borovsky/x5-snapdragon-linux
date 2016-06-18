find_package(PkgConfig)
pkg_check_modules(PC_QMI_GLIB qmi-glib)

find_path(QMI_GLIB_INCLUDE_DIRS
    NAMES libqmi-glib.h
    HINTS ${QMI_GLIB_INCLUDE_DIR}
          ${PC_QMI_GLIB_INCLUDEDIR}
          ${PC_QMI_GLIB_INCLUDE_DIRS}
    PREFIXES libqmi-glib
)

find_library(QMI_GLIB_LIBRARIES
    NAMES qmi-glib
    HINTS ${PC_QMI_GLIB_LIBRARY_DIRS}
          ${PC_QMI_GLIB_LIBDIR}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QMI_GLIB REQUIRED_VARS QMI_GLIB_INCLUDE_DIRS QMI_GLIB_LIBRARIES
  VERSION_VAR   PC_QMI_GLIB_VERSION)


message(STATUS "LimQMI-glib  was found here:")
message(STATUS "      folder with libraries: ${QMI_GLIB_LIBRARIES}")
message(STATUS "      folder with includes : ${QMI_GLIB_INCLUDE_DIRS}")

mark_as_advanced(
    QMI_GLIB_INCLUDE_DIRS
    QMI_GLIB_LIBRARIES
)
