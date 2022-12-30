import lvgl as lv
from  lv_utils import event_loop
import discovery_display as d 


class Display:
   def __init__(self):
        lv.init()
        d.init()

        draw_buf = lv.disp_draw_buf_t()
        buf1_1 = bytearray(240*30*4)
        draw_buf.init(buf1_1, None, len(buf1_1)//4)
        disp_drv = lv.disp_drv_t()
        disp_drv.init()
        disp_drv.draw_buf = draw_buf
        disp_drv.flush_cb = d.flush
        disp_drv.hor_res = 240
        disp_drv.ver_res = 320
        disp_drv.register()

        indev_drv = lv.indev_drv_t()
        indev_drv.init()
        indev_drv.type = lv.INDEV_TYPE.POINTER
        indev_drv.read_cb = d.ts_read;
        indev_drv.register()

        try:
            event_loop()
        except:
            pass 