#include "stdafx.h"
#include "ConsoleControlHandler.h"
#include <windows.h> 
#include <shared/Log.h>

namespace shared
{
   namespace process
   {
      boost::function<bool()> CConsoleControlHandler::m_onStopRequestedFct;

      void CConsoleControlHandler::setOnStopRequestedHandler(boost::function<bool()> onStopRequestedFct)
      {
         m_onStopRequestedFct = onStopRequestedFct;
         SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(ctrlHandler), TRUE);
      }

      BOOL CConsoleControlHandler::ctrlHandler(DWORD fdwCtrlType)
      {
         switch (fdwCtrlType)
         {
            // Handle the CTRL-C signal. 
         case CTRL_C_EVENT:
            // CTRL-CLOSE : close console event
         case CTRL_CLOSE_EVENT:
            {
               // Signal stop request and wait for application fully stops
               if (!m_onStopRequestedFct())
               {
                  YADOMS_LOG(error) << "CConsoleControlHandler : Fail to wait the app end event";
                  return FALSE;
               }

               return TRUE;
            }
            // Pass other signals to the next handler. 
         case CTRL_BREAK_EVENT:
            return FALSE;

         case CTRL_LOGOFF_EVENT:
            return FALSE;

         case CTRL_SHUTDOWN_EVENT:
            return FALSE;

         default:
            return FALSE;
         }
      }
   }
} // namespace shared::process


