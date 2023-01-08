import lvgl as lv
from  lv_utils import event_loop
import stm32f429disc_disp as d 
import json


class Display:
   
    def read_calibration(self,filename):        
        with open(filename,"r") as f:
            cal = json.load(f)
            d.ts_calibrate(x1=cal[0],y1=cal[1],x2=cal[2],y2=cal[3])


    def __init__(self,calfile="calibration.json"):
        lv.init()
        d.init()
        w = d.lcd_width()
        h = d.lcd_height()

        draw_buf = lv.disp_draw_buf_t()
        bufsz =  w* 30 * lv.color_t.__SIZE__
        buf1_1 = bytearray(bufsz)
        buf1_2 = bytearray(bufsz)

        draw_buf.init(buf1_1, buf1_2, len(buf1_1) // lv.color_t.__SIZE__)
        disp_drv = lv.disp_drv_t()
        disp_drv.init()
        disp_drv.draw_buf = draw_buf
        disp_drv.flush_cb = d.flush
        disp_drv.hor_res = w
        disp_drv.ver_res = h
        disp_drv.register()

        indev_drv = lv.indev_drv_t()
        indev_drv.init()
        indev_drv.type = lv.INDEV_TYPE.POINTER
        indev_drv.read_cb = d.ts_read
        indev_drv.register()

        try:
            self.read_calibration(calfile)
        except Exception as e:
            print(e)    

        try:
            event_loop()
        except:
            pass 

    def __del__():
        print("Display destructor")
        d.deinit()

