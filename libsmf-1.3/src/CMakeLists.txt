########### next target ###############

set(smf_LIB_SRCS
    smf.h
    smf_private.h
    smf.c
    smf_decode.c
    smf_load.c
    smf_save.c
    smf_tempo.c
    smf_carray.c
    minIni.c
    smf_carray.h
    minIni.c)

add_library(smf SHARED ${smf_LIB_SRCS})

target_link_libraries(smf -lglib-2.0)

set_target_properties(smf PROPERTIES VERSION 1.0.0 SOVERSION 1)
#install(TARGETS smf ${INSTALL_TARGETS_DEFAULT_ARGS})


  
  include_directories(/home/sakuraharuno/Projects/timidity-utauloid/libsmf-1.3/
		      /usr/include/glib-2.0/
		      /usr/lib/i386-linux-gnu/glib-2.0/include/)
add_definitions(-std=c99 -g)




########### next target ###############

set(smfsh_SRCS smfsh.c)

add_executable(smfsh ${smfsh_SRCS})

target_link_libraries(smfsh ${KDE4_KDECORE_LIBS} smf m)

#install(TARGETS smfsh ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(vsqdump_SRCS vsqdump.c)

add_executable(vsqdump ${vsqdump_SRCS})

target_link_libraries(vsqdump smf m)


########### install files ###############

#install(FILES  smf.h DESTINATION include)



#original Makefile.am contents follow:

#include_HEADERS = smf.h
#
#lib_LTLIBRARIES = libsmf.la
#libsmf_la_SOURCES = smf.h smf_private.h smf.c smf_decode.c smf_load.c smf_save.c smf_tempo.c smf_carray.c minIni.c smf_carray.h minIni.c
#libsmf_la_CFLAGS = $(GLIB_CFLAGS) -DG_LOG_DOMAIN=\"libsmf\"
#libsmf_la_LIBADD = 
#
#bin_PROGRAMS = smfsh
#smfsh_SOURCES = smfsh.c
#smfsh_CFLAGS = $(GLIB_CFLAGS) -DG_LOG_DOMAIN=\"smfsh\"
#smfsh_LDADD = libsmf.la $(GLIB_LIBS) $(READLINE_LIBS) -lm
#
