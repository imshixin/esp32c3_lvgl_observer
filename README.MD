# ESP32C3+0.96寸tft屏幕电脑状态监控项目

使用VScode+platformIO+lvgl+arduino开发,使用c++编写

上位机使用Python程序和[LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor)编写

通过WIFI传输数据

上位机程序在这个库 [esp32c3_observer](https://github.com/imshixin/esp32c3_observer)

**上位机程序需要使用管理员权限执行，否则获取不到cpu数据**

使用时自行更改`main.cpp`中的`ssid`和`passwd`以及`main.py`的`ip`
