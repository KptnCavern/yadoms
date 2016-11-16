#include "stdafx.h"
#include "urlManager.h"
#include "http/HttpMethods.h"

shared::CDataContainer urlManager::sendCommand(/*Poco::Net::IPAddress IPAddress,*/ Poco::Net::SocketAddress socket, shared::CDataContainer parameters)
{
   std::stringstream url;

   // create the URL
   url << "http://" << /*IPAddress.toString() << ":" <<*/ socket.toString() << "/api/xdevices.json";

   return http::CHttpMethods::SendGetRequest(url.str(), parameters);
}