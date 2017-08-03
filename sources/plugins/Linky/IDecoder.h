#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/communication/Buffer.hpp>

namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	The Linky protocol interface
//--------------------------------------------------------------
class IDecoder
{
public:
   //--------------------------------------------------------------
   /// \brief	Destructor
   //--------------------------------------------------------------
   virtual ~IDecoder()
   {
   }

   //--------------------------------------------------------------
   /// \brief	                   Decode Linky Reader message
   /// \param [in] api             Plugin execution context (Yadoms API)
   /// \param [in] messages        Received messages
   //--------------------------------------------------------------
   virtual void decodeLinkyMessage(boost::shared_ptr<yApi::IYPluginApi> api,
                                   const boost::shared_ptr<std::map<std::string, std::vector<std::string>>>& messages) = 0;
};