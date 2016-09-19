# include common plugin definitions

include("../YadomsPluginsCommon.cmake")

#########################################
## Place here all the plugin source files
#########################################
set(PIFACE2_PLUGIN_SOURCES
   stdafx.h
   stdafx.cpp
   InitializationException.hpp
   IOManager.h
   IOManager.cpp
   IPf2Configuration.h
   Pf2Configuration.h
   Pf2Configuration.cpp
   Piface2.h
   Piface2.cpp
   Piface2Factory.h
   Piface2Factory.cpp
   staticInterrupts.h
   staticInterrupts.cpp
   IIO.h
   IO.h
   IO.cpp
   )

#########################################
## Place here all additional include directories
## (by default yadoms-shared and boost are included)
#########################################

   set(PIFACE2_PLUGIN_INCLDIR  
       ${CMAKE_CURRENT_SOURCE_DIR})

#########################################
## Place here all additional libraries to link with
## (by default yadoms-shared and boost are included)
#########################################
set(PIFACE2_PLUGIN_LINK
   pifacedigital
   mcp23s17
)

#########################################
## Place here all additional compiler flags
## i.e. : for raspberry to link with wiringPi : use add_definition("-lwiringPi")
#########################################

#########################################
## Always leave these lines to configure plugin correctly
## even if FAKE_PLUGIN_INCLDIR or FAKE_PLUGIN_LINK are empty
#########################################

PLUGIN_SOURCES(Piface2 ${PIFACE2_PLUGIN_SOURCES})
PLUGIN_INCLDIR(Piface2 ${PIFACE2_PLUGIN_INCLDIR})
PLUGIN_LINK   (Piface2 ${PIFACE2_PLUGIN_LINK})

#########################################
## Post-build commands
## Copy files to build directory
## "package.json" and "icon.png" are needed, but you can add extra-files
#########################################
PLUGIN_POST_BUILD_COPY_FILE(Piface2 package.json)
PLUGIN_POST_BUILD_COPY_FILE(Piface2 icon.png)
PLUGIN_POST_BUILD_COPY_DIRECTORY(Piface2 locales)