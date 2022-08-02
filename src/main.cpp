#include <lvgl.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include "WIFIManager.h"
#include "UIManager.h"

#define WAINTING_PERIOD 1000
#define MAIN_DATA_OFFSET_X 12
#define MAIN_DATA_OFFSET_Y 18
#define MY_SYMBOL_TEMP "\xEE\x9B\xAE"
#define MY_SYMBOL_LOAD "\xEE\x9C\xBB"
#define TEMP_COLOR lv_color_hex(0xa30cf9)
#define LOAD_COLOR lv_color_hex(0x21a3ef)
LV_FONT_DECLARE(iconfont_symbol);

static const uint16_t screenWidth = 160;
static const uint16_t screenHeight = 80;
int port = 34567; //端口

data_t data;

//显示缓存
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
WiFiServer server(port);

WiFiClient client;//与上位机通讯的客户端
lv_timer_t *server_timer = NULL;
WIFIManager wm("nova 5 Pro", "81297311");//wifi管理器
UIManager *um;//显示界面管理器

void wifi_server_listen_timer(lv_timer_t *);
void wifi_client_get_data_timer(lv_timer_t *);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void check_wifi_conn_timer(lv_timer_t *timer)
{
  if (wm.checkConn())
  {
    Serial.println("wifi connected");
    server.begin();
    //wifi已连接，切换显示ip
    um->load_scr(UM_SCR_IP_SHOW);
    lv_timer_del(timer);
    lv_timer_create(wifi_server_listen_timer,1000,NULL);
  }
}

void inline display_init()
{
  lv_init();

  tft.begin();        /* TFT init */
  tft.setRotation(1); /* Landscape orientation, flipped */

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

/* 上位机连接监听 */
void wifi_server_listen_timer(lv_timer_t *timer)
{
  Serial.println("等待WIFI上位机连接");
  client = server.available();
  if (!client)
    return;
  Serial.println("WiFiClient established");
  Serial.printf("Client IP :%s\n", client.remoteIP().toString().c_str());
  um->load_scr(UM_SCR_DATA_SHOW);//加载显示数据界面
  client.setTimeout(3);
  //接收处理数据函数的定时器
  //仅执行一次是为了让当前轮函数运行后在执行下一轮函数，以防两轮的函数同时执行
  lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, NULL);
  lv_timer_set_repeat_count(t, 1);
  lv_timer_del(timer);
}

void wifi_client_get_data_timer(lv_timer_t *timer)
{

  if (!client)
  {
    client.stop();
    //当前连接已断开，启动监听上位机连接的函数
    lv_timer_create(wifi_server_listen_timer, 1000, NULL);
    um->load_scr(UM_SCR_IP_SHOW);//加载显示ip的界面
    Serial.println("stop WiFiClient");
    return;
  }
  if (!client.available())
  {
    // delay(WAINTING_PERIOD);
    //开启下一次数据处理函数
    lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, timer->user_data);
    lv_timer_set_repeat_count(t, 1);
    return;
  }
  // delay(WAINTING_PERIOD);
  data.cpu_load = client.parseInt();
  data.cpu_temp = client.parseInt();
  data.gpu_load = client.parseInt();
  data.gpu_temp = client.parseInt();
  um->update_ui(data);//刷新界面
  Serial.printf("get new data::{%d,%d,%d,%d}\n", data.cpu_load, data.cpu_temp, data.gpu_load, data.gpu_temp);
  //开启下一次数据处理函数
  lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, timer->user_data);
  lv_timer_set_repeat_count(t, 1);
  Serial.println("set new async call");
}

void main_init()
{
  //UI管理器
  um = new UIManager();
  Serial.println("show screen wait for connect");
  um->load_scr(UM_SCR_WAIT_FOR_CONNECT);//加载等待wifi连接的界面
  Serial.println("start connect wifi");
  wm.connect(true);//连接wifi
  lv_timer_create(check_wifi_conn_timer, 1000, NULL);//定时检查wifi连接
}
/* 生成随机数据 */
void update_random_data(lv_timer_t *t)
{
  uint8_t dat = rand() % 101;
  data.cpu_load = dat;
  data.cpu_temp = dat;
  data.gpu_load = dat;
  data.gpu_temp = dat;
  um->update_ui(data);
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  display_init();//显示驱动和lvgl初始化
  main_init();
  Serial.println("Setup done");
}
void loop()
{
  //lvgl的定时处理器
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
