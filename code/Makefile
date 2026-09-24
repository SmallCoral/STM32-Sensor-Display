TARGET := stm32_sensor_display
BUILD_DIR := build

PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPU_FLAGS := -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
DEFINES := -DSTM32C562xx
INCLUDES := \
	-ICore/Inc \
	-IApp/Inc \
	-IBSP/Inc \
	-IDrivers/CMSIS/Core/Include \
	-IDrivers/CMSIS/Device/ST/STM32C5xx/Include \
	-IDrivers/STM32C5xx_LL_Driver/Inc

SOURCES := \
	Core/Src/main.c \
	Core/Src/system_clock.c \
	Core/Src/system_stm32c5xx.c \
	Core/Startup/startup_stm32c562xx.c \
	BSP/Src/board_i2c.c \
	BSP/Src/board_user_button.c \
	BSP/Src/ht16k33.c \
	App/Src/display_demo.c

OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
DEPS := $(OBJECTS:.o=.d)
LDSCRIPT := STM32C562CETX_FLASH.ld

CFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) \
	-std=gnu11 -O2 -g3 -Wall -Wextra -Werror \
	-ffunction-sections -fdata-sections -fstack-usage \
	--specs=nano.specs --specs=nosys.specs

LDFLAGS := $(CPU_FLAGS) -T$(LDSCRIPT) \
	-Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
	-Wl,--gc-sections -Wl,--print-memory-usage \
	--specs=nano.specs --specs=nosys.specs -static

.PHONY: all clean size

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
