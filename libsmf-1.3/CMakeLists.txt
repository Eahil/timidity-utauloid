



#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)
add_subdirectory(man)


########### install files ###############

#install(FILES  smf.pc DESTINATION )



#original Makefile.am contents follow:

#SUBDIRS = src man
#
#EXTRA_DIST = smf.pc.in
#
#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = smf.pc
#
