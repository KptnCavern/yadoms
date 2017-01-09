#pragma once

namespace shared
{
   namespace script
   {
      namespace yInterpreterApi
      {
         //-----------------------------------------------------
         ///\brief Interface for interpreter to start script
         //-----------------------------------------------------
         class IStartScript
         {
         public:

            //-----------------------------------------------------
            ///\brief Destructor
            //-----------------------------------------------------
            virtual ~IStartScript()
            {
            }

            //-----------------------------------------------------
            ///\brief Get the script instance ID
            //-----------------------------------------------------
            virtual int getScriptInstanceId() = 0;

            //-----------------------------------------------------
            ///\brief Get the script path
            //-----------------------------------------------------
            virtual const std::string& getScriptPath() = 0;

            //-----------------------------------------------------
            ///\brief Get the script API instance ID
            //-----------------------------------------------------
            virtual const std::string& getScriptApiId() = 0;
         };
      }
   }
} // namespace shared::script::yInterpreterApi	


