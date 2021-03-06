# Check dependencies
include(findGammu.cmake)

if(GAMMU_FOUND)

   # include common plugin definitions
   include("../YadomsPluginsCommon.cmake")

   #########################################
   ## Place here all the plugin source files
   #########################################
   set(SMS_DIALER_SOURCES
      stdafx.h
      stdafx.cpp
      GammuPhone.h
      GammuPhone.cpp
      GammuPhoneConnection.h
      GammuPhoneConnection.cpp
      IPhone.h
      ISmsDialerConfiguration.h
      ISms.h
      PhoneException.hpp
      Sms.h
      Sms.cpp 
      SmsDialer.h
      SmsDialer.cpp 
      SmsDialerConfiguration.h
      SmsDialerConfiguration.cpp 
      SmsDialerFactory.h
      SmsDialerFactory.cpp 
   )

   #########################################
   ## Place here all additional groups (solution folder)
   #########################################
   source_group(sources ./*.*)
   
   #########################################
   ## Place here all additional include directories
   ## (by default yadoms-shared and boost are included)
   #########################################
   set(SMS_DIALER_ADDITIONAL_INC_DIR
      ${GAMMU_INCLUDE_DIRS}
   )

   #########################################
   ## Place here all additional libraries to link with
   ## (by default yadoms-shared and boost are included)
   #########################################
   set(SMS_DIALER_ADDITIONAL_LIB
       ${GAMMU_LIBRARIES}
   )

   #########################################
   ## Always leave this three lines to configure plugin correctly
   ## even if SMS_DIALER_ADDITIONAL_INC_DIR or SMS_DIALER_ADDITIONAL_LIB are empty
   #########################################
   PLUGIN_SOURCES(smsDialer ${SMS_DIALER_SOURCES})
   PLUGIN_INCLDIR(smsDialer ${SMS_DIALER_ADDITIONAL_INC_DIR})
   PLUGIN_LINK(smsDialer ${SMS_DIALER_ADDITIONAL_LIB})

   #########################################
   ## Post-build commands
   ## Copy files to build directory
   ## "package.json" and "icon.png" are needed, but you can add extra-files
   #########################################
   PLUGIN_POST_BUILD_COPY_FILE(smsDialer package.json)
   PLUGIN_POST_BUILD_COPY_FILE(smsDialer changelog.md)
   PLUGIN_POST_BUILD_COPY_FILE(smsDialer icon.png)
   PLUGIN_POST_BUILD_COPY_DIRECTORY(smsDialer locales)
else()
   message(WARNING "smsDialer plugin is not available. Gammu is Missing")
endif()

