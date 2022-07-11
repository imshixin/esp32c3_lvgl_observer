#include <lvgl.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiServer.h>

#define WAINTING_PERIOD 1000
#define MAIN_DATA_OFFSET_X 12
#define MAIN_DATA_OFFSET_Y 18
#define MY_SYMBOL_TEMP "\xEE\x9B\xAE"
#define MY_SYMBOL_LOAD "\xEE\x9C\xBB"
#define TEMP_COLOR lv_color_hex(0xa30cf9)
#define LOAD_COLOR lv_color_hex(0x21a3ef)
// #define MY_SYMBOL_CPU1 "\xEE\x98\x8D"
// #define MY_SYMBOL_CPU2 "\xEE\x98\x9D"
// #define MY_SYMBOL_GPU "\xEE\xA0\xA5"

LV_FONT_DECLARE(iconfont_symbol);
typedef struct align_objs_t
{
  lv_obj_t *arc;
  lv_obj_t *ref;
} align_objs_t;

static const uint16_t screenWidth = 160;
static const uint16_t screenHeight = 80;
const char *ssid = "nova 5 Pro";
const char *passwd = "12345678";
const int port = 34567;

struct Data
{
  uint8_t cpu_load;
  uint8_t cpu_temp;
  uint8_t gpu_load;
  uint8_t gpu_temp;
} data;
struct data_objs_t
{
  lv_obj_t *llarc;
  lv_obj_t *ltarc;
  lv_obj_t *rlarc;
  lv_obj_t *rtarc;
} arcs,labels;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
WiFiServer server(port);
lv_obj_t* ip_show_scr;
lv_obj_t* data_show_scr;
WiFiClient client;
lv_timer_t *server_timer = NULL;
bool showData=false;

void ui_ip_show_setup(void *);
void wifi_server_listen_timer(lv_timer_t *);
void wifi_client_get_data_timer(lv_timer_t *);
void switch_screen(void *data);

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

void change_led_timer(lv_timer_t *timer)
{
  lv_obj_t *led = (lv_obj_t *)(timer->user_data);
  if (WiFi.status() == WL_CONNECTED)
  {
    lv_timer_del(timer);
  }
  else
  {
    lv_led_toggle(led);
  }
}

void check_wifi_conn_timer(lv_timer_t *timer)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("wifi connected");
    lv_obj_t *screen = (lv_obj_t *)(timer->user_data);
    lv_timer_del(timer);
    lv_obj_del(screen);
    lv_async_call(ui_ip_show_setup, NULL);
  }
  else
  {
    Serial.println("waiting for wifi connecting");
    lv_led_toggle((lv_obj_t *)timer->user_data);
  }
}

void lv_arc_set_style(lv_obj_t *arc, lv_coord_t w, uint16_t rotation, uint16_t bg_start, uint16_t bg_end, float zoom, lv_coord_t x, lv_coord_t y, lv_arc_mode_t mode)
{
  lv_obj_set_pos(arc, x, y);
  lv_obj_set_style_arc_width(arc, w, LV_PART_MAIN);
  lv_obj_set_style_arc_width(arc, w, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(arc, LV_OPA_10, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(arc, LV_OPA_10, LV_PART_INDICATOR);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
  lv_obj_center(arc);
  lv_arc_set_rotation(arc, rotation);
  lv_arc_set_bg_angles(arc, bg_start, bg_end);
  lv_obj_set_size(arc, (lv_pct((int16_t)(100.0 * zoom))), (lv_pct((int16_t)(100.0 * zoom))));
  lv_arc_set_mode(arc, mode);
  Serial.printf("style width:%d,height:%d,lv_pct100:%d\n", lv_obj_get_style_width(arc, LV_PART_MAIN), lv_obj_get_style_height(arc, LV_PART_MAIN), lv_pct(100));
}

// void async_lv_arc_align_center(lv_timer_t *objs)
// {
//   align_objs_t *align = (align_objs_t *)objs->user_data;

//   lv_coord_t x = lv_obj_get_width(align->ref) / 2 + lv_obj_get_x(align->ref) - lv_obj_get_width(align->arc)/2;
//   lv_coord_t y = lv_obj_get_height(align->ref) / 2 + lv_obj_get_y(align->ref) - lv_obj_get_height(align->arc)/2;
//   lv_obj_set_pos(align->arc, x, y);
// }

// void inline lv_arc_align_center(lv_obj_t *arc, lv_obj_t *ref)
// {
//   static align_objs_t align;
//   align.arc = arc;
//   align.ref = ref;
//   lv_timer_t *timer = lv_timer_create(async_lv_arc_align_center, 100, &align);
//   lv_timer_set_repeat_count(timer, 1);
// }
// void scale(lv_obj_t *obj, lv_coord_t weight, lv_coord_t height, float zoom)
// {
//   lv_obj_set_size(obj, (int)(weight * zoom), (int)(height * zoom));
// }

void anim_set_value_start(lv_obj_t *arc, int16_t value)
{
  if (arc == NULL)
  {
    Serial.println("arc is NULL");
  }
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, arc);
  lv_anim_set_time(&anim, 300);
  lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_arc_set_value);
  lv_anim_set_values(&anim, lv_arc_get_value(arc), value);
  lv_anim_start(&anim);
}
void update_ui(void *arg)
{
  // data_objs_t *arcs = (data_objs_t *)arg;
  anim_set_value_start(arcs.llarc, data.cpu_load);
  anim_set_value_start(arcs.ltarc, data.cpu_temp);
  anim_set_value_start(arcs.rlarc, 100 - data.gpu_load);
  anim_set_value_start(arcs.rtarc, 100 - data.gpu_temp);
  lv_label_set_text_fmt(labels.llarc,"%d" MY_SYMBOL_LOAD,data.cpu_load);
  lv_label_set_text_fmt(labels.ltarc,"%d" MY_SYMBOL_TEMP,data.cpu_temp);
  lv_label_set_text_fmt(labels.rlarc,"%d" MY_SYMBOL_LOAD,data.gpu_load);
  lv_label_set_text_fmt(labels.rtarc,"%d" MY_SYMBOL_TEMP,data.gpu_temp);
}

void ui_data_show_setup(void *scr)
{
  static lv_style_t style;
  lv_style_init(&style);
  data_show_scr = lv_obj_create(NULL);
  lv_obj_t *left_con = lv_obj_create(data_show_scr);
  lv_obj_t *right_con = lv_obj_create(data_show_scr);
  lv_obj_set_pos(left_con, 0, 0);
  lv_obj_set_pos(right_con, lv_pct(50), 0);
  lv_obj_set_size(left_con, lv_pct(50), lv_pct(100));
  lv_obj_set_size(right_con, lv_pct(50), lv_pct(100));
  lv_style_set_border_width(&style, 1);
  lv_style_set_radius(&style, 0);
  lv_style_set_pad_all(&style, 0
  );
  lv_obj_set_scrollbar_mode(left_con, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_style(left_con, &style, LV_PART_MAIN);
  lv_obj_add_style(right_con, &style, LV_PART_MAIN);

  lv_obj_t *left_load_arc = lv_arc_create(left_con);
  lv_obj_t *left_temp_arc = lv_arc_create(left_con);
  lv_obj_t *right_load_arc = lv_arc_create(right_con);
  lv_obj_t *right_temp_arc = lv_arc_create(right_con);
  lv_obj_set_style_arc_color(left_load_arc,LOAD_COLOR,LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(right_load_arc,LOAD_COLOR,LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(left_temp_arc,TEMP_COLOR,LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(right_temp_arc,TEMP_COLOR,LV_PART_INDICATOR);
  lv_arc_set_style(left_load_arc, 5, 90, 0, 180, 1, 0, 0, LV_ARC_MODE_NORMAL);
  lv_arc_set_style(left_temp_arc, 8, 90, 0, 180, 0.8, 0, 0, LV_ARC_MODE_NORMAL);
  lv_arc_set_style(right_load_arc, 5, 270, 0, 180, 1, 0, 0, LV_ARC_MODE_REVERSE);
  lv_arc_set_style(right_temp_arc, 8, 270, 0, 180, 0.8, 0, 0, LV_ARC_MODE_REVERSE);
  arcs.llarc = left_load_arc;
  arcs.ltarc = left_temp_arc;
  arcs.rlarc = right_load_arc;
  arcs.rtarc = right_temp_arc;

  lv_obj_t* cpu_label = lv_label_create(left_con);
  lv_obj_t* gpu_label = lv_label_create(right_con);
  lv_label_set_text(cpu_label,"CPU");
  lv_label_set_text(gpu_label,"GPU");
  lv_obj_set_style_text_font(cpu_label,&lv_font_montserrat_16,LV_PART_MAIN);
  lv_obj_set_style_text_font(gpu_label,&lv_font_montserrat_16,LV_PART_MAIN);
  lv_obj_align(cpu_label,LV_ALIGN_RIGHT_MID,0,-14);
  lv_obj_align(gpu_label,LV_ALIGN_LEFT_MID,0,-14);

  lv_obj_t* cpu_load_label = lv_label_create(left_con);
  lv_obj_t* cpu_temp_label = lv_label_create(left_con);
  lv_obj_t* gpu_load_label = lv_label_create(right_con);
  lv_obj_t* gpu_temp_label = lv_label_create(right_con);
  lv_obj_set_style_text_font(cpu_load_label,&iconfont_symbol,LV_PART_MAIN);
  lv_obj_set_style_text_font(cpu_temp_label,&iconfont_symbol,LV_PART_MAIN);
  lv_obj_set_style_text_font(gpu_load_label,&iconfont_symbol,LV_PART_MAIN);
  lv_obj_set_style_text_font(gpu_temp_label,&iconfont_symbol,LV_PART_MAIN);
  lv_obj_set_style_text_color(cpu_load_label,LOAD_COLOR,LV_PART_MAIN);
  lv_obj_set_style_text_color(cpu_temp_label,TEMP_COLOR,LV_PART_MAIN);
  lv_obj_set_style_text_color(gpu_temp_label,TEMP_COLOR,LV_PART_MAIN);
  lv_obj_set_style_text_color(gpu_load_label,LOAD_COLOR,LV_PART_MAIN);
  lv_obj_align(cpu_load_label,LV_ALIGN_BOTTOM_RIGHT,0,0);
  lv_obj_align(cpu_temp_label,LV_ALIGN_BOTTOM_RIGHT,-MAIN_DATA_OFFSET_X,-MAIN_DATA_OFFSET_Y);
  lv_obj_align(gpu_load_label,LV_ALIGN_BOTTOM_LEFT,0,0);
  lv_obj_align(gpu_temp_label,LV_ALIGN_BOTTOM_LEFT,MAIN_DATA_OFFSET_X,-MAIN_DATA_OFFSET_Y);
  labels.llarc = cpu_load_label;
  labels.ltarc = cpu_temp_label;
  labels.rlarc = gpu_load_label;
  labels.rtarc = gpu_temp_label;
}

lv_obj_t *ui_network_setup()
{
  Serial.println("init ui_network");
  lv_obj_t *root = lv_obj_create(lv_scr_act());
  lv_obj_set_size(root, 160, 80);
  lv_obj_set_pos(root, 0, 0);
  lv_obj_t *txt = lv_label_create(root);
  lv_label_set_text(txt, "Connecting WIFI...");
  lv_obj_align(txt, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_t *led = lv_led_create(root);
  lv_obj_align(led, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_timer_create(change_led_timer, 500, led);
  return root;
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

void inline toggle_ip_and_data_screen(){
  if(!showData){
    if(data_show_scr!=NULL){
      lv_scr_load(data_show_scr);
    }
    showData=true;
    // if(ip_show_scr!=NULL){
    //   lv_
    // }
  }else{
    if(ip_show_scr!=NULL){
      lv_scr_load(ip_show_scr);
      showData=false;
    }
  }
}

void ui_ip_show_setup(void *scr)
{
  ip_show_scr = lv_obj_create(NULL);
  lv_obj_t *label = lv_label_create(ip_show_scr);
  lv_label_set_text(label, ("Connect It:\n" + WiFi.localIP().toString() + ":34567").c_str());
  lv_obj_center(label);
  server.begin();
  lv_scr_load(ip_show_scr);
  lv_timer_create(wifi_server_listen_timer, 1000,NULL);
}

void wifi_server_listen_timer(lv_timer_t *timer)
{
  client = server.available();
  if (!client)
  {
    Serial.println("waiting for client connect");
    return;
  }
  Serial.println("WiFiClient established");
  Serial.printf("Client IP :%s\n", client.remoteIP().toString().c_str());
  toggle_ip_and_data_screen();
  client.setTimeout(5);
  lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, NULL);
  lv_timer_set_repeat_count(t, 1);
  lv_timer_del(timer);
}

void wifi_client_get_data_timer(lv_timer_t *timer)
{

  if (!client)
  {
    client.stop();
    lv_timer_create(wifi_server_listen_timer, 1000, NULL);
    // lv_async_call(ui_ip_show_setup, NULL);
    toggle_ip_and_data_screen();
    Serial.println("stop WiFiClient");
    return;
  }
  if (!client.available())
  {
    // delay(WAINTING_PERIOD);
    lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, timer->user_data);
    lv_timer_set_repeat_count(t, 1);
    return;
  }
  // delay(WAINTING_PERIOD);
  data.cpu_load = client.parseInt();
  data.cpu_temp = client.parseInt();
  data.gpu_load = client.parseInt();
  data.gpu_temp = client.parseInt();
  lv_async_call(update_ui,NULL);
  Serial.printf("get new data::{%d,%d,%d,%d}\n", data.cpu_load, data.cpu_temp, data.gpu_load, data.gpu_temp);
  lv_timer_t *t = lv_timer_create(wifi_client_get_data_timer, 1000, timer->user_data);
  lv_timer_set_repeat_count(t, 1);
  Serial.println("set new async call");
}

void main_init()
{
  lv_obj_t *net_screen = ui_network_setup();
  Serial.println("start connect wifi");
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, passwd);
  lv_async_call(ui_data_show_setup, NULL);
  lv_timer_create(check_wifi_conn_timer, 1000, net_screen);
}

void update_random_data(lv_timer_t* t){
  uint8_t dat = rand()%101;
  data.cpu_load =dat;
  data.cpu_temp =dat;
  data.gpu_load =dat;
  data.gpu_temp =dat;
  update_ui(NULL);
}
void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  display_init();
  main_init();
  // ui_data_show_setup(NULL);
  // lv_scr_load(data_show_scr);
  // lv_timer_create(update_random_data,1000,NULL);
  Serial.println("Setup done");
}
void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
