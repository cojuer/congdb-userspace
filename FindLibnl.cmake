# - Find libnl
#
# This module defines
#  LIBNL_FOUND - whether the libnl library was found
#  LIBNL_LIBRARIES - the libnl library
#  LIBNL_INCLUDE_DIRS - the include path of the libnl library

find_library(LIBNL_LIBRARY nl-3)
find_library(LIBNL_GENL_LIBRARY nl-genl-3)

set(LIBNL_LIBRARIES
  ${LIBNL_LIBRARY}
  ${LIBNL_GENL_LIBRARY}
)

find_path(LIBNL_INCLUDE_DIRS
  NAMES libnl3/netlink/netlink.h
)

set(LIBNL_INCLUDE_DIRS
  ${LIBNL_INCLUDE_DIRS}
  ${LIBNL_INCLUDE_DIRS}/libnl3
)
