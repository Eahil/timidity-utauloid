########### next target ###############

set(smf_LIB_SRCS
    smf.h
    smf_private.h
    smf.c
    smf_decode.c
    smf_load.c
    smf_save.c
    smf_tempo.c
    iniparser.c
    iniparser.h
    dictionary.c
    dictionary.h	
		
	
   )

add_library(smf SHARED ${smf_LIB_SRCS})

target_link_libraries(smf -lglib-2.0)

set_target_properties(smf PROPERTIES VERSION 1.0.0 SOVERSION 1)
#install(TARGETS smf ${INSTALL_TARGETS_DEFAULT_ARGS})


  
  include_directories(/usr/include/glib-2.0/
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

########### next target ###############

set(ustdump_SRCS ustdump.c)

add_executable(ustdump ${ustdump_SRCS})

target_link_libraries(ustdump smf m)


########### install files ###############

#install(FILES  smf.h DESTINATION include)




