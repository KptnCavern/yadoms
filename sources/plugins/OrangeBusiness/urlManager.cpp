#include "stdafx.h"
#include "urlManager.h"
#include <shared/Log.h>
//#include "shared/http/SecureSession.h"
#include "shared/http/StandardSession.h"

urlManager::urlManager():
   m_url("https://liveobjects.orange-business.com/api/v0"), //liveobjects
   m_baseUrl("liveobjects.orange-business.com")
{}

shared::CDataContainer urlManager::getRegisteredEquipments(const std::string &apikey,
                                                           const int page,
                                                           const bool activated,
                                                           const boost::posix_time::time_duration& timeout)
{
   shared::CDataContainer response;
   shared::CDataContainer headerParameters;
   shared::CDataContainer parameters;

   // define some header parameters
   headerParameters.set("Host", m_baseUrl.str());
   headerParameters.set("X-API-Key", apikey);
   headerParameters.set("Accept", "application/json");

   parameters.set("page", boost::lexical_cast<std::string>(page));

   if (activated)
      parameters.set("status", "ACTIVATED");

   std::string deviceUrl = m_url.str() + "/vendors/lora/devices";
   auto session = boost::make_shared<shared::StandardSession>(deviceUrl);
   shared::CHttpMethods::SendGetRequest(session,
                                        headerParameters,
                                        parameters,
                                        [&](shared::CDataContainer& data)
                                        {
                                           response = data;
                                        },
                                        timeout);

   return response;
}

shared::CDataContainer urlManager::getDeviceInformation(const std::string &apikey, 
                                                        const std::string &devEUI,
                                                        const boost::posix_time::time_duration& timeout)
{
   shared::CDataContainer response;
   shared::CDataContainer headerParameters;
   shared::CDataContainer noParameters;

   // define some header parameters
   headerParameters.set("Host", m_baseUrl.str());
   headerParameters.set("X-API-Key", apikey);
   headerParameters.set("Accept", "application/json");

   std::string deviceUrl = "https://liveobjects.orange-business.com/api/v0/vendors/lora/devices/" + devEUI;
   boost::shared_ptr<shared::StandardSession> session = boost::make_shared<shared::StandardSession>(deviceUrl);
   shared::CHttpMethods::SendGetRequest(session,
                                        headerParameters,
                                        noParameters,
                                        [&](shared::CDataContainer& data)
                                        {
                                           response = data;
                                        },
                                        timeout);

   return response;
}

shared::CDataContainer urlManager::listDeviceCommands(const std::string &apikey,
                                                      const std::string &devEUI,
                                                      const int page,
                                                      const boost::posix_time::time_duration& timeout)
{
   shared::CDataContainer response;
   shared::CDataContainer headerParameters;
   shared::CDataContainer parameters;

   // define some header parameters
   headerParameters.set("Host", m_baseUrl.str());
   headerParameters.set("X-API-Key", apikey);
   headerParameters.set("Accept", "application/json");

   parameters.set("page", boost::lexical_cast<std::string>(page));

   std::string deviceUrl = "https://liveobjects.orange-business.com/api/v0/vendors/lora/devices/" + devEUI + "/commands";
   boost::shared_ptr<shared::StandardSession> session = boost::make_shared<shared::StandardSession>(deviceUrl);
   shared::CHttpMethods::SendGetRequest(session,
                                        headerParameters,
                                        parameters,
                                        [&](shared::CDataContainer& data)
                                        {
                                           response = data;
                                        },
                                        timeout);

   return response;
}