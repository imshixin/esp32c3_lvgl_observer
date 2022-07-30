#include "UIManager.h"
#include "WIFIManager.h"
#include <lvgl.h>
#include <Arduino.h>

#define MAIN_DATA_OFFSET_X 12
#define MAIN_DATA_OFFSET_Y 18
#define MY_SYMBOL_TEMP "\xEE\x9B\xAE"
#define MY_SYMBOL_LOAD "\xEE\x9C\xBB"
#define TEMP_COLOR lv_color_hex(0xa30cf9)
#define LOAD_COLOR lv_color_hex(0x21a3ef)
LV_FONT_DECLARE(iconfont_symbol);

extern WIFIManager wm;
extern int port;
// extern data_t datas;

UIManager::UIManager(){
  Serial.println("start init screen");
  init_data_show();
  init_ip_show();
  init_waiting_for_connect();
  Serial.println("screen init success");
}

void UIManager::load_scr(screen_type st){
  switch (st)
  {
  case UM_SCR_IP_SHOW:
    // if(!wm.checkConn()){
    //   load_scr(UM_SCR_WAIT_FOR_CONNECT);
    //   break;
    // }
    lv_label_set_text(label_ip_show,("Connect It:\n" + WiFi.localIP().toString() + ":"+String(port)).c_str());
    lv_scr_load(scr_ip_show);
    break;
  case UM_SCR_DATA_SHOW:
    lv_scr_load(scr_data_show);
    break;
  case UM_SCR_WAIT_FOR_CONNECT:
  default:
    lv_scr_load(scr_wait_for_connect);
    break;
  }
}

void UIManager::init_ip_show()
{
  scr_ip_show = lv_obj_create(NULL);
  label_ip_show = lv_label_create(scr_ip_show);
  // lv_label_set_text(label_ip_show, ("Connect It:\n" + WiFi.localIP().toString() + ":34567").c_str());
  lv_obj_center(label_ip_show);
}

void UIManager::init_data_show(){
  static lv_style_t style;
  lv_style_init(&style);
  scr_data_show = lv_obj_create(NULL);
  lv_obj_t *left_con = lv_obj_create(scr_data_show);
  lv_obj_t *right_con = lv_obj_create(scr_data_show);
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

// void set_text(lv_timer_t* timer){
//   lv_obj_t* text=(lv_obj_t*)timer->user_data;
//   lv_label_set_text(text, "Connecting WIFI...");
// }

void inline UIManager::init_waiting_for_connect(){
  scr_wait_for_connect = lv_obj_create(NULL);
  lv_obj_set_size(scr_wait_for_connect, 160, 80);
  lv_obj_set_pos(scr_wait_for_connect, 0, 0);
  lv_obj_t *txt = lv_label_create(scr_wait_for_connect);
  lv_label_set_text(txt, "Connecting WIFI..");
  lv_obj_align(txt, LV_ALIGN_TOP_MID, 0, 0);
  // txt->user_data=0;
  // lv_obj_t *led = lv_led_create(scr_wait_for_connect);
  // lv_obj_align(led, LV_ALIGN_BOTTOM_MID, 0, 0);
  // lv_timer_create(set_text, 500, txt);
}

void UIManager::update_ui(data_t datas)
{
  // data_objs_t *arcs = (data_objs_t *)arg;
  anim_set_value_start(arcs.llarc, datas.cpu_load);
  anim_set_value_start(arcs.ltarc, datas.cpu_temp);
  anim_set_value_start(arcs.rlarc, 100 - datas.gpu_load);
  anim_set_value_start(arcs.rtarc, 100 - datas.gpu_temp);
  lv_label_set_text_fmt(labels.llarc,"%d" MY_SYMBOL_LOAD,datas.cpu_load);
  lv_label_set_text_fmt(labels.ltarc,"%d" MY_SYMBOL_TEMP,datas.cpu_temp);
  lv_label_set_text_fmt(labels.rlarc,"%d" MY_SYMBOL_LOAD,datas.gpu_load);
  lv_label_set_text_fmt(labels.rtarc,"%d" MY_SYMBOL_TEMP,datas.gpu_temp);
}

void UIManager::anim_set_value_start(lv_obj_t *arc, int16_t value)
{
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, arc);
  lv_anim_set_time(&anim, 300);
  lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_arc_set_value);
  lv_anim_set_values(&anim, lv_arc_get_value(arc), value);
  lv_anim_start(&anim);
}


void UIManager::lv_arc_set_style(lv_obj_t *arc, lv_coord_t w, uint16_t rotation, uint16_t bg_start, uint16_t bg_end, float zoom, lv_coord_t x, lv_coord_t y, lv_arc_mode_t mode)
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
}
