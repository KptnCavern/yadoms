# Macros for setting up a script wrapper for yScriptApi
#
# Note that we use SWIG to generate wrappers.
# CMake macro for SWIG generates a target called _yScriptApiWrapper

INCLUDE(${SWIG_USE_FILE})

function(SCRIPT_INTERPRETER_IS_IN_DEV_STATE _targetName)
   string(FIND ${_targetName} "dev-" DEV_SUBSTRING_POSITION)
   if (${DEV_SUBSTRING_POSITION} EQUAL 0)
      set(DEV_SCRIPT_INTERPRETER TRUE PARENT_SCOPE)
   else()
      set(DEV_SCRIPT_INTERPRETER FALSE PARENT_SCOPE)
   endif()
endfunction()

MACRO(SCRIPT_API_WRAPPER_SET_PARENT_INTERPRETER parentInterpreter)
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YADOMS_OUTPUT_DIR}/scriptInterpreters/${parentInterpreter})
   foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
       string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
       set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${YADOMS_OUTPUT_DIR}/${OUTPUTCONFIG}/scriptInterpreters/${parentInterpreter})
   endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)   
ENDMACRO()
   
MACRO(SCRIPT_API_WRAPPER_SOURCES targetLanguage)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SHARED_INCL_DIR})
   SET(SWIG_MODULE_yScriptApiWrapper_EXTRA_DEPS ${SHARED_COMMON_INCL_DIR}/shared/script/yScriptApi/IYScriptApi.h)

   if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
      SWIG_ADD_MODULE(yScriptApiWrapper ${targetLanguage} ${ARGN})
   else()
      SWIG_ADD_LIBRARY(yScriptApiWrapper
         LANGUAGE ${targetLanguage}
         SOURCES ${ARGN})
   endif()

   project(yScriptApiWrapper)   
ENDMACRO()

MACRO(SCRIPT_API_WRAPPER_INCLDIR)
   set_property( TARGET _yScriptApiWrapper PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} ${SHARED_INCL_DIR} ${ARGN})
	if(COTIRE_USE)
      if(COTIRE_USE_UNITY)
         set_property( TARGET _yScriptApiWrapper_unity PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} ${SHARED_INCL_DIR} ${ARGN})
      endif(COTIRE_USE_UNITY)
	endif(COTIRE_USE)
ENDMACRO()

MACRO(SCRIPT_API_WRAPPER_LINK)
   SWIG_LINK_LIBRARIES(yScriptApiWrapper yadoms-shared ${ARGN})
   
	if(COTIRE_USE)
   
		if(COTIRE_USE_UNITY)
			set_target_properties(_yScriptApiWrapper PROPERTIES COTIRE_ADD_UNITY_BUILD TRUE)
		else()
			set_target_properties(_yScriptApiWrapper PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
		endif()
      
      # for this target, precompiled header has no real sense
      set_target_properties(_yScriptApiWrapper PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
      
		cotire(_yScriptApiWrapper)
	endif()	   
ENDMACRO()

MACRO(SCRIPT_API_SOURCE_GROUP parentInterpreter)
   IF(MSVC OR XCODE)
      SET_PROPERTY(TARGET ${SWIG_MODULE_yScriptApiWrapper_REAL_NAME} PROPERTY FOLDER "scriptInterpreters/${parentInterpreter}")
   ENDIF()
ENDMACRO()


# brief Copy a file to the target output dir
# param [in] _resource The resource (absolute path) to copy to the target output dir
# param [in] parentInterpreter The parent interpreter
MACRO(SCRIPT_API_WRAPPER_POST_BUILD_COPY_FILE _resource parentInterpreter)

   get_filename_component(_resourcePath ${_resource}  DIRECTORY)
   get_filename_component(_resourceFile ${_resource}  NAME)

   string(REPLACE "-" "_" ComponentCompatibleName ${parentInterpreter})

   SCRIPT_INTERPRETER_IS_IN_DEV_STATE(${parentInterpreter})
   if (NOT ${DEV_SCRIPT_INTERPRETER})
      install(FILES ${_resource} 
            DESTINATION ${INSTALL_BINDIR}/scriptInterpreters/${parentInterpreter}
            COMPONENT  ${ComponentCompatibleName})
   endif()

   add_custom_command(TARGET _yScriptApiWrapper POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_resource} $<TARGET_FILE_DIR:_yScriptApiWrapper>/${_resourceFile})
   if(COTIRE_USE)
      if(COTIRE_USE_UNITY)
         add_custom_command(TARGET _yScriptApiWrapper_unity POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_resource} $<TARGET_FILE_DIR:_yScriptApiWrapper_unity>/${_resourceFile})
	  endif()	
   endif()	
ENDMACRO()

