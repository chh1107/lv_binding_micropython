#include "py/runtime.h"
#include "py/mphal.h"
#include "softtimer.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../../lvgl/lvgl.h"
#include "../../../lv_conf.h"
#include "../../include/common.h"

#include "ports/stm32/i2c.h"

#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

DMA2D_HandleTypeDef *hdma2d = NULL;         // handle to DMA2D, referenced in stm32_it.c
i2c_t *i2c_ts = NULL;                       // I2C handle for touchscreen
lv_disp_drv_t *dma2d_disp_drv = NULL;  // handle to display driver
lv_color_t *fb[2] = {NULL, NULL};           // framebuffer pointers
uint32_t w = LCD_WIDTH;                             // display width
uint32_t h = LCD_HEIGHT;                             // display height
volatile bool dma2d_pend = false;           // flag of DMA2D pending operation

static int16_t  A1, A2, B1, B2;  // Touch Screen Calibration correction factors

static bool config_dma2d(void);


#include "stm32f429i_discovery_lcd.h"


STATIC mp_obj_t framebuffer(mp_obj_t n_obj) {
	int n = mp_obj_get_int(n_obj) -1;

	if (n<0 || n>1){
		return mp_const_none;
	}

	if(fb[n]==NULL){
		// allocation on extRAM with 1KB alignment to speed up LTDC burst access on AHB
		fb[n] = MP_STATE_PORT(f429_disco_fb[n]) = m_malloc(sizeof(lv_color_t) * w * h  + 1024);
		fb[n] = (lv_color_t*)((uint32_t)fb[n] + 1024 - (uint32_t)fb[n] % 1024);
	}
	return mp_obj_new_bytearray_by_ref(sizeof(lv_color_t) * w * h , (void *)fb[n]);
}


STATIC void set_calibration(int32_t x1,int32_t y1,int32_t x2,int32_t y2) {

     A1 = (1000 * ( w))/ ( x2 - x1); 
     B1 =  - A1 * x1; 
      
     A2 = (1000 * (h))/ ( y2 - y1); 
     B2 = - A2 * y1; 
}

STATIC mp_obj_t ts_calibrate(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
   
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x1, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x2, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LCD_WIDTH} },
        { MP_QSTR_y2, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LCD_HEIGHT} }
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    set_calibration(args[0].u_int, args[1].u_int, args[2].u_int, args[3].u_int);

    return mp_const_none;
}    


STATIC mp_obj_t _init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_w, ARG_h };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_w, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LCD_WIDTH} },
        { MP_QSTR_h, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LCD_HEIGHT} }
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    w = args[ARG_w].u_int;
    h = args[ARG_h].u_int;

    framebuffer(mp_obj_new_int(1));

    if (fb[0] == NULL) {
        mp_obj_new_exception_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed allocating frame buffer"));
    }


    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(1,(uint32_t)fb[0]);
    BSP_TS_Init(w,h);
    set_calibration(0,0,w,h);
   

    if (!config_dma2d()) {
         mp_obj_new_exception_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("DMA2D init error"));
    }

    // i2c_ts = I2C3;
    // i2c_init(i2c_ts, MICROPY_HW_I2C3_SCL, MICROPY_HW_I2C3_SDA, 400000, 100);

    // if (BSP_TS_Init(w, h) != TS_OK) {
    //     mp_obj_new_exception_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Touchscreen init error"));
    // }
  
    return mp_const_none;
}


STATIC void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
   
        hdma2d->Init.Mode = DMA2D_M2M;
        hdma2d->Init.OutputOffset = w - lv_area_get_width(area);
        dma2d_disp_drv = disp_drv;
        dma2d_pend = true;
        HAL_DMA2D_Init(hdma2d);
        HAL_DMA2D_Start_IT(hdma2d,
            (uint32_t)color_p,
            (uint32_t)(fb[0] + area->x1 + area->y1 * w),
            lv_area_get_width(area),
            lv_area_get_height(area));
         
}



void DMA2D_TransferComplete(DMA2D_HandleTypeDef *hdma2d) {
    lv_disp_flush_ready(dma2d_disp_drv);
    dma2d_pend = false;
}

// void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
//     lv_disp_flush_ready(dma2d_disp_drv);
// }


static bool config_dma2d(void) {
    static DMA2D_HandleTypeDef Dma2dHandle = {0};

    #if (LV_COLOR_DEPTH == 32)
    uint32_t OutColorMode = DMA2D_OUTPUT_ARGB8888;
    uint32_t InColorMode = DMA2D_INPUT_ARGB8888;
    #elif (LV_COLOR_DEPTH == 16)
    uint32_t OutColorMode = DMA2D_OUTPUT_RGB565;
    uint32_t InColorMode = DMA2D_INPUT_RGB565;
    #else
    #error "modrk043fn48h: LV_COLOR_DEPTH not supported"
    #endif

    Dma2dHandle.Init.Mode = DMA2D_M2M_BLEND;
    Dma2dHandle.Init.ColorMode = OutColorMode;
    Dma2dHandle.Init.OutputOffset = 0x0;
    Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[1].InputAlpha = 0xFF;
    Dma2dHandle.LayerCfg[1].InputColorMode = InColorMode;
    Dma2dHandle.LayerCfg[1].InputOffset = 0x0;
    Dma2dHandle.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[0].InputAlpha = 0xFF;
    Dma2dHandle.LayerCfg[0].InputColorMode = InColorMode;
    Dma2dHandle.LayerCfg[0].InputOffset = 0x0;
    Dma2dHandle.Instance = DMA2D;
    Dma2dHandle.XferCpltCallback = DMA2D_TransferComplete;

    if (HAL_DMA2D_Init(&Dma2dHandle) != HAL_OK) {
        return false;
    }

    hdma2d = &Dma2dHandle;

    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 0);
    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);

    return true;
}


static inline uint16_t Calibration_GetX(uint16_t x)
{
  return (((A1 * x) + B1)/1000);
}

static inline uint16_t Calibration_GetY(uint16_t y)
{
  return (((A2 * y) + B2)/1000);
}


STATIC bool ts_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static TS_StateTypeDef ts_state;
    static lv_coord_t lastX = 0;
    static lv_coord_t lastY = 0;

    BSP_TS_GetState(&ts_state);
    if (ts_state.TouchDetected) {
        data->point.x = lastX = Calibration_GetX(ts_state.X);
        data->point.y = lastY = Calibration_GetY(ts_state.Y);
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->point.x = lastX;
        data->point.y = lastY;
        data->state = LV_INDEV_STATE_REL;
    }

    return false;
}


STATIC MP_DEFINE_CONST_FUN_OBJ_KW(_init_obj, 0, _init);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(_ts_calibrate_obj, 0, ts_calibrate);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(_framebuffer_obj, framebuffer);
DEFINE_PTR_OBJ(flush_cb);
DEFINE_PTR_OBJ(ts_read);


STATIC const mp_rom_map_elem_t _globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_discovery_display) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_ts_calibrate), MP_ROM_PTR(&_ts_calibrate_obj) },
    // { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_rk043fn48h_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&PTR_OBJ(flush_cb))},
    { MP_ROM_QSTR(MP_QSTR_ts_read), MP_ROM_PTR(&PTR_OBJ(ts_read))},
    { MP_ROM_QSTR(MP_QSTR_framebuffer), MP_ROM_PTR(&PTR_OBJ(_framebuffer))}
};

STATIC MP_DEFINE_CONST_DICT(
    stm32f429_disp_globals,
    _globals_table
    );

const mp_obj_module_t mp_module_discovery_display = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&stm32f429_disp_globals
};


MP_REGISTER_MODULE(MP_QSTR_discovery_display, mp_module_discovery_display);