#pragma once
#include <shared/DataContainer.h>
#include <Poco/Net/SocketAddress.h>

//--------------------------------------------------------------
/// \brief	Interface of plugin configuration
//--------------------------------------------------------------
class ImasterDeviceConfiguration
{
public:
   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~ImasterDeviceConfiguration()
   {}

   //--------------------------------------------------------------
   /// \brief      Load configuration data
   /// \param [in] data The data container
   //--------------------------------------------------------------
   virtual void initializeWith(const shared::CDataContainer& data) = 0;

   //--------------------------------------------------------------
   /// \brief      retrieve the IP address with the port from the configuration
   /// \return     the IP address withe the port
   //--------------------------------------------------------------
   virtual Poco::Net::SocketAddress getIPAddressWithSocket() const = 0;

   //--------------------------------------------------------------
   /// \brief      retrieve the password used to connect the WES
   /// \return     the password
   //--------------------------------------------------------------
   virtual std::string getPassword() const = 0;

   //--------------------------------------------------------------
   /// \brief      return if the instant current for the selected clamp should be registered
   /// \return     true if the instant current element is selected
   //--------------------------------------------------------------
   virtual bool isInstantCurrentClampRegistered(const int selectedClamp) const = 0;

   //--------------------------------------------------------------
   /// \brief      retrieve the password used to connect the WES
   /// \return     the password
   //--------------------------------------------------------------
   virtual std::string getUser() const = 0;
};