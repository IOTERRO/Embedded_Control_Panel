file(GLOB_RECURSE LIB_H
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    #${EXTERNAL_LIBS}/ftdi-mpsse/*.h
)

file(GLOB_RECURSE LIB_CPP
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    #${EXTERNAL_LIBS}/ftdi-mpsse/*.cpp
)


set(EXTERNAL_LIBS ${CMAKE_CURRENT_LIST_DIR}/../../External)

set(H_FILES ${LIB_H})

set(CPP_FILES ${LIB_CPP})


add_module(factory
      MODULE_TYPE
         dll 
      SOURCE_H_FILES
         ${H_FILES}
      SOURCE_CPP_FILES
         ${CPP_FILES}
      VS_FOLDER
         
	  SAHRED_INCLUDES
		
      DEPENDENCIES
		ioAdapter
	  POSTBUILD_COPY
		
	  IMPORT_SUFFIX
		
	  MODULE_HELP
		FALSE
)