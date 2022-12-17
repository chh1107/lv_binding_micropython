
#include "stm32f429i_discovery_lcd.h"

LTDC_HandleTypeDef  hltdc;
//DMA2D_HandleTypeDef hdma2d;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

// /* Default LCD configuration with LCD Layer 1 */
// static uint32_t ActiveLayer = 0;
// static LCD_DrawPropTypeDef DrawProp[MAX_LAYER_NUMBER];
LCD_DrvTypeDef  *LcdDrv;



uint8_t BSP_LCD_Init(void)
{ 
  /* On STM32F429I-DISCO, it is not possible to read ILI9341 ID because */
  /* PIN EXTC is not connected to VDD and then LCD_READ_ID4 is not accessible. */
  /* In this case, ReadID function is bypassed.*/  
  /*if(ili9341_drv.ReadID() == ILI9341_ID)*/

    /* LTDC Configuration ----------------------------------------------------*/
    hltdc.Instance = LTDC;
    
    /* Timing configuration  (Typical configuration from ILI9341 datasheet)
          HSYNC=10 (9+1)
          HBP=20 (29-10+1)
          ActiveW=240 (269-20-10+1)
          HFP=10 (279-240-20-10+1)
    
          VSYNC=2 (1+1)
          VBP=2 (3-2+1)
          ActiveH=320 (323-2-2+1)
          VFP=4 (327-320-2-2+1)
      */
    
    /* Configure horizontal synchronization width */
    hltdc.Init.HorizontalSync = ILI9341_HSYNC;
    /* Configure vertical synchronization height */
    hltdc.Init.VerticalSync = ILI9341_VSYNC;
    /* Configure accumulated horizontal back porch */
    hltdc.Init.AccumulatedHBP = ILI9341_HBP;
    /* Configure accumulated vertical back porch */
    hltdc.Init.AccumulatedVBP = ILI9341_VBP;
    /* Configure accumulated active width */
    hltdc.Init.AccumulatedActiveW = 269;
    /* Configure accumulated active height */
    hltdc.Init.AccumulatedActiveH = 323;
    /* Configure total width */
    hltdc.Init.TotalWidth = 279;
    /* Configure total height */
    hltdc.Init.TotalHeigh = 327;
    
    /* Configure R,G,B component values for LCD background color */
    hltdc.Init.Backcolor.Red= 0;
    hltdc.Init.Backcolor.Blue= 0;
    hltdc.Init.Backcolor.Green= 0;
    
    /* LCD clock configuration */
    /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
    /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
    /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
    /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct); 
    
    /* Polarity */
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    
    BSP_LCD_MspInit();
    HAL_LTDC_Init(&hltdc); 
    
    /* Select the device */
    LcdDrv = &ili9341_drv;

    /* LCD Init */	 
    LcdDrv->Init();
    LcdDrv->DisplayOn();
    return LCD_OK;
}  


/**
  * @brief  Gets the LCD X size.  
  * @retval The used LCD X size
  */
uint32_t BSP_LCD_GetXSize(void)
{
  return LcdDrv->GetLcdPixelWidth();
}

/**
  * @brief  Gets the LCD Y size.  
  * @retval The used LCD Y size
  */
uint32_t BSP_LCD_GetYSize(void)
{
  return LcdDrv->GetLcdPixelHeight();
}

/**
  * @brief  Initializes the LCD layers.
  * @param  LayerIndex: the layer foreground or background. 
  * @param  FB_Address: the layer frame buffer.
  */
void BSP_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{     
  LCD_LayerCfgTypeDef   Layercfg;

 /* Layer Init */
  Layercfg.WindowX0 = 0;
  Layercfg.WindowX1 = BSP_LCD_GetXSize();
  Layercfg.WindowY0 = 0;
  Layercfg.WindowY1 = BSP_LCD_GetYSize(); 
  Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  Layercfg.FBStartAdress = FB_Address;
  Layercfg.Alpha = 255;
  Layercfg.Alpha0 = 0;
  Layercfg.Backcolor.Blue = 0;
  Layercfg.Backcolor.Green = 0;
  Layercfg.Backcolor.Red = 0;
  Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  Layercfg.ImageWidth = BSP_LCD_GetXSize();
  Layercfg.ImageHeight = BSP_LCD_GetYSize();
  
  HAL_LTDC_ConfigLayer(&hltdc, &Layercfg, LayerIndex); 

  /* Dithering activation */
 // HAL_LTDC_EnableDither(&LtdcHandler);
}


__weak void BSP_LCD_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable the LTDC and DMA2D Clock */
  __HAL_RCC_LTDC_CLK_ENABLE();
  __HAL_RCC_DMA2D_CLK_ENABLE(); 
  
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIOs Configuration */
  /*
   +------------------------+-----------------------+----------------------------+
   +                       LCD pins assignment                                   +
   +------------------------+-----------------------+----------------------------+
   |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
   |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
   |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
   |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
   |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
   |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
   -------------------------------------------------------------------------------
            |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
            |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
             -----------------------------------------------------
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  
 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);     

 /* GPIOG configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
 
  /* GPIOB configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}
