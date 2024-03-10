set(EXTERNAL_LIBS ${CMAKE_CURRENT_LIST_DIR}/../../External)


file(GLOB_RECURSE LIB_H
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    #${EXTERNAL_LIBS}/ftdi-mpsse/*.h
)

file(GLOB_RECURSE LIB_CPP
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    #${EXTERNAL_LIBS}/ftdi-mpsse/*.cpp
)



set(H_FILES ${LIB_H})

set(CPP_FILES ${LIB_CPP})

# Set the paths to libusbK libraries
set(libusbk_release ${EXTERNAL_LIBS}/libusbK/bin/dll/Win32/Release)
set(libusbk_debug ${EXTERNAL_LIBS}/libusbK/bin/dll/Win32/Debug)

#Set the paths to FT232 librairy
set(ft232Lib ${EXTERNAL_LIBS}/FT232_Lib)

#Set the paths to mpsse librairy
set(ft232_mpsse_Lib ${EXTERNAL_LIBS}/ftdi-mpsse/official/release/build/Win32)

message("ftd2xx libs: "  ${EXTERNAL_LIBS}/ftdi-mpsse/official/release/libftd2xx)

set(Boost_LIBRARY_DIR $ENV{BOOST_LIBRARYDIR})

add_module(ioAdapter
      MODULE_TYPE
         dll 
      SOURCE_H_FILES
         ${H_FILES}
      SOURCE_CPP_FILES
         ${CPP_FILES}
      VS_FOLDER
         
	  SAHRED_INCLUDES
		${EXTERNAL_LIBS}/ftdi-mpsse/official/release/
		${EXTERNAL_LIBS}/ftdi-mpsse/official/release/include
		#${EXTERNAL_LIBS}/libusbk/includes
		#${EXTERNAL_LIBS}/libusbk/src
		#${ft232Lib}
		#${EXTERNAL_LIBS}/ftdi-mpsse/official/release/
      DEPENDENCIES
		${ft232_mpsse_Lib}/libmpsse.lib
		${ft232_mpsse_Lib}/ftd2xx.lib
		${Boost_LIBRARY_DIR}/libboost_thread-vc142-mt$<$<CONFIG:debug>:-gd>-x32-1_71.lib
		${Boost_LIBRARY_DIR}/libboost_chrono-vc142-mt$<$<CONFIG:debug>:-gd>-x32-1_71.lib
		${Boost_LIBRARY_DIR}/libboost_date_time-vc142-mt$<$<CONFIG:debug>:-gd>-x32-1_71.lib
		${Boost_LIBRARY_DIR}/libboost_regex-vc142-mt$<$<CONFIG:debug>:-gd>-x32-1_71.lib
		#debug ${libusbk_debug}/libusbK.lib
		#${ft232Lib}/i386/ftd2xx.lib
        #optimized ${libusbk_release}/libusbK.lib
	  POSTBUILD_COPY
		${ft232_mpsse_Lib}/libmpsse.dll
		${ft232_mpsse_Lib}/ftd2xx.dll
		#debug ${libusbk_debug}/libusbK.dll
		#${ft232Lib}/i386/ftd2xx.dll
        #optimized ${libusbk_release}/libusbK.dll
	  IMPORT_SUFFIX
		
	  MODULE_HELP
		FALSE
)