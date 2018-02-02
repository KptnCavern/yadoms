#include "stdafx.h"
#include "urlManager.h"
#include <shared/Log.h>
#include "shared/http/SecureSession.h"

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
   boost::shared_ptr<shared::SecureSession> session = boost::make_shared<shared::SecureSession>(deviceUrl);
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
   boost::shared_ptr<shared::SecureSession> session = boost::make_shared<shared::SecureSession>(deviceUrl);
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
   headerParameters.set("X-API-Key", apikey);
   headerParameters.set("Accept", "application/json");

   parameters.set("limit", "5");
   parameters.set("sort", "-creationTs"); // Get the newest command at the first page
   std::string deviceUrl = "https://liveobjects.orange-business.com/api/v0/data/streams/urn:lora:" + devEUI + "!uplink";
   boost::shared_ptr<shared::SecureSession> session = boost::make_shared<shared::SecureSession>(deviceUrl);
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