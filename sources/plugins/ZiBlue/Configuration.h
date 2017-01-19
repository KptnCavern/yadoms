#pragma once

#include <shared/DataContainer.h>

//--------------------------------------------------------------
/// \brief	Configuration of the xpl plugin
/// \note   To implement configuration, we have to derive from shared::plugin::configuration::CConfiguration
//--------------------------------------------------------------
class CConfiguration
{
public:
   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CConfiguration();

   //--------------------------------------------------------------
   /// \brief	    The serial port
   //--------------------------------------------------------------
   std::string getSerialPort() const;
   
   //--------------------------------------------------------------
   /// \brief	    Initialize configuration
   //--------------------------------------------------------------
   void initializeWith(shared::CDataContainer container);

private :
   //--------------------------------------------------------------
   /// \brief	    The container
   //--------------------------------------------------------------
   shared::CDataContainer m_container;
};

