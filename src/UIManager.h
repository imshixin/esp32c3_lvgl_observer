#include <lvgl.h>

enum screen_type
{
  UM_SCR_WAIT_FOR_CONNECT,
  UM_SCR_IP_SHOW,
  UM_SCR_DATA_SHOW
};
struct data_t
{
  uint8_t cpu_load;
  uint8_t cpu_temp;
  uint8_t gpu_load;
  uint8_t gpu_temp;
};
class UIManager
{
public:
//切换界面
  void load_scr(screen_type st);
  UIManager();
  //更新界面
  void update_ui(data_t datas);

private:
  //界面初始化函数
  void init_waiting_for_connect();
  void init_ip_show();
  void init_data_show();
  //动画函数
  void anim_set_value_start(lv_obj_t *arc, int16_t value);
  //圆弧样式设置
  void lv_arc_set_style(lv_obj_t *arc, lv_coord_t w, uint16_t rotation, uint16_t bg_start, uint16_t bg_end, float zoom, lv_coord_t x, lv_coord_t y, lv_arc_mode_t mode);
  lv_obj_t *scr_wait_for_connect;
  lv_obj_t *scr_ip_show;
  lv_obj_t *scr_data_show;
  lv_obj_t* label_ip_show;
  struct data_objs_t
  {
    lv_obj_t *llarc;
    lv_obj_t *ltarc;
    lv_obj_t *rlarc;
    lv_obj_t *rtarc;
  } arcs, labels;
};
