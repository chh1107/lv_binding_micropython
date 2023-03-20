# Shortcut for current directory
$(BOARD)_DIR := $(LVGL_BINDING_DIR)/driver/stm32/$(BOARD)

# LIB_SRC_C += $(shell find $($(BOARD)_DIR)/*.c)
# Add your module's source files to LIB_SRC_C
LIB_SRC_C += $($(BOARD)_DIR)/modstm32f429disc_disp.c
LIB_SRC_C += $($(BOARD)_DIR)/modgyro.c
LIB_SRC_C += $($(BOARD)_DIR)/stm32f429i_discovery_lcd.c
LIB_SRC_C += $($(BOARD)_DIR)/stm32f429i_discovery_ts.c
LIB_SRC_C += $($(BOARD)_DIR)/stm32f429i_discovery.c
# You must also add helpers and library code that isn't MicroPython-specific.
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_io.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_sdram.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_gyroscope.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/stmpe811/stmpe811.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/ili9341/ili9341.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/l3gd20/l3gd20.c
LIB_SRC_C += $($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/i3g4250d/i3g4250d.c

# Add custom compiler options to CFLAGS
## Enable proper hardware revision
CFLAGS += -DUSE_STM32F429I_DISCOVERY_REVD
## Add includes to search for header files
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/STM32F429I-Discovery
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/stmpe811
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/ili9341
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/l3gd20
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/i3g4250d
CFLAGS += -I$($(BOARD)_DIR)/../../../STM32CubeF4/Drivers/BSP/Components/Common