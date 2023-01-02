import lvgl as lv
from  lv_utils import event_loop
import discovery_display as d 
import json


class Display:
   
   def read_calibration(self,filename):        
        with open(filename,"r") as f:
            cal =  json.load(f)
            d.ts_calibrate(x1=cal[0],y1=cal[1],x2=cal[2],y2=cal[3])


   def __init__(self,calfile="calibration.json"):
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
            self.read_calibration(calfile)
        except Exception as e:
            print(e)    

        try:
            event_loop()
        except:
            pass 