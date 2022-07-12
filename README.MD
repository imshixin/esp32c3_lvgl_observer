# ESP32C3+0.96寸tft屏幕电脑状态监控项目

使用VScode+platformIO+lvgl+arduino开发,使用c++编写

上位机使用Python程序和[LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor)编写

通过WIFI传输数据

上位机程序在这个库 [esp32c3_observer](https://github.com/imshixin/esp32c3_observer)

**注意：上位机程序需要使用管理员权限执行，否则获取不到cpu数据**

使用时自行更改`main.cpp`中的`ssid`和`passwd`以及`main.py`的`ip`

# 配置文件
如果你不会自己配置lvgl和tft_eSpi，可以使用上传的配置文件

将`pio.zip`中的`.pio\libdeps\esp32-c3-devkitm-1\lvgl\lv_conf.h`和`.pio\libdeps\esp32-c3-devkitm-1\TFT_eSPI\User_Setup_Select.h` `.pio\libdeps\esp32-c3-devkitm-1\TFT_eSPI\User_Setup.h`复制到自己项目的对应目录下

> 如果你的屏幕不是合宙的9.9包邮0.96寸小屏幕，你可能需要对tft_eSPI做一点配置，具体建议百度
