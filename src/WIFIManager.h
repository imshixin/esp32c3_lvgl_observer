#include <WiFi.h>
#include <string>
using namespace std;

class WIFIManager{
public:
  WIFIManager(string ssid,string pwd);
  void connect(bool disconn);
  bool checkConn();
private:
  string ssid,pwd;
};
