#include "WIFIManager.h"
#include <WiFi.h>
#include <string>
using namespace std;

WIFIManager::WIFIManager(string ssid, string pwd) : ssid(ssid), pwd(pwd)
{
}
void WIFIManager::connect(bool disconn = true)
{
  if (disconn)
    WiFi.disconnect();
  WiFi.begin(this->ssid.c_str(), this->pwd.c_str());
}
bool WIFIManager::checkConn()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }
  return false;
}
