#include <WiFi.h>
#include <string>
using namespace std;

class WIFIManager{
public:
  WIFIManager(string ssid,string pwd);
  /* 连接WIFI
    disconn：是否要先断开再连接
   */
  void connect(bool disconn);
  /* 检查wifi连接 */
  bool checkConn();
private:
  string ssid,pwd;
};
