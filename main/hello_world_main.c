/*
 * First Project in ESP32
 *  Smart FAN project */

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<driver/gpio.h>
#include "driver/ledc.h"
#include <string.h>
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/i2c_master.h"
#include <stdint.h>
// Intiallization of GPIO, ADC, PWM, I2C

#define LED0 GPIO_NUM_4  // Fan Status LED- Green
#define LED1 GPIO_NUM_16 // FAN status LED- Yellow
#define LED2 GPIO_NUM_17 // FAN status LED- Red
#define DC_FAN GPIO_NUM_18 // DC-FAN
#define BUTTON GPIO_NUM_34 //Manual On-OFF Switch
#define I2C_PORT        I2C_NUM_0  // I2C port Assignment
#define I2C_SDA_PIN     GPIO_NUM_21 // LM75 SDA Pin
#define I2C_SCL_PIN     GPIO_NUM_22 // LM75 SCL Pin
#define LM75_ADDRESS    0x48 // LM75- Slave Address
#define TEMP_REGISTER   0x00 // LM75- Temperature register Address
#define UART_PORT UART_NUM_0 // UART port Assignment
// Manual On-OFF
        volatile bool Fan_state = false;
        static void IRAM_ATTR button_isr_handler(void *arg)
        {
                Fan_state = !Fan_state;
        }

void app_main(void)
{  
	// LM35 temperature reading
 	i2c_master_bus_handle_t bus_handle;
	i2c_master_dev_handle_t lm75_handle;
	i2c_master_bus_config_t bus_cfg =
    	{
        	.i2c_port = I2C_PORT,
        	.sda_io_num = I2C_SDA_PIN,
        	.scl_io_num = I2C_SCL_PIN,
        	.clk_source = I2C_CLK_SRC_DEFAULT,
        	.glitch_ignore_cnt = 7,
        	.flags.enable_internal_pullup = true,
    	};
	i2c_new_master_bus(&bus_cfg, &bus_handle);
	i2c_device_config_t dev_cfg =
    	{
        	.dev_addr_length = I2C_ADDR_BIT_LEN_7,
        	.device_address = LM75_ADDRESS,
        	.scl_speed_hz = 100000,
    	};
	i2c_master_bus_add_device(bus_handle,&dev_cfg, &lm75_handle);
	// ADC- Potentiometer Unit
	adc_oneshot_unit_handle_t adc1_handle;
	adc_oneshot_unit_init_cfg_t init_config = {
        	.unit_id = ADC_UNIT_1,
    		};
    	adc_oneshot_new_unit(&init_config, &adc1_handle);
    	adc_oneshot_chan_cfg_t config = {
        	.bitwidth = ADC_BITWIDTH_DEFAULT,
        	.atten = ADC_ATTEN_DB_12,
    		};
    	adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);
	int adc_value;
	 /* PWM timer configuration*/

        ledc_timer_config_t timer = {
        	.speed_mode = LEDC_LOW_SPEED_MODE,
        	.timer_num = LEDC_TIMER_0,
        	.duty_resolution = LEDC_TIMER_10_BIT,
        	.freq_hz = 1000,
        	.clk_cfg = LEDC_AUTO_CLK
        	};
        ledc_timer_config(&timer);
	ledc_channel_config_t fan = {
    		.gpio_num = DC_FAN,
    		.speed_mode = LEDC_LOW_SPEED_MODE,
    		.channel = LEDC_CHANNEL_0,
    		.timer_sel = LEDC_TIMER_0,
    		.duty = 0,
		};
	ledc_channel_config(&fan);
	// FAN status LED block
	gpio_reset_pin(LED0);
        gpio_reset_pin(LED1);
        gpio_reset_pin(LED2);
	gpio_set_direction(LED0,GPIO_MODE_OUTPUT);
        gpio_set_direction(LED1,GPIO_MODE_OUTPUT);
        gpio_set_direction(LED2,GPIO_MODE_OUTPUT);
	/* Interrupt Service Routine*/
	gpio_config_t button_config = {
        	.pin_bit_mask = (1ULL << BUTTON),
        	.mode = GPIO_MODE_INPUT,
        	.pull_up_en = GPIO_PULLUP_ENABLE,
        	.intr_type = GPIO_INTR_NEGEDGE
    		};
    	gpio_config(&button_config);
    	gpio_install_isr_service(0);
    	gpio_isr_handler_add(BUTTON, button_isr_handler, NULL);
	// UART configuration
	uart_config_t uart_config =
	    {
        	.baud_rate = 115200,
        	.data_bits = UART_DATA_8_BITS,
        	.parity = UART_PARITY_DISABLE,
        	.stop_bits = UART_STOP_BITS_1,
        	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        	.source_clk = UART_SCLK_DEFAULT
    		};

    	uart_driver_install(UART_PORT,1024,0,0,NULL,0);
    	uart_param_config(UART_PORT,&uart_config);


 // while loop
    	while (1)
    	{	
		 uint32_t duty;
		//Manual On-Off
		if (Fan_state)
		{
			duty=1023;
		}
		else
		{
    			duty=0;
		}

		// LM35 reading
        	uint8_t reg = TEMP_REGISTER;
        	uint8_t data[2];
		//uint32_t duty;
        	i2c_master_transmit_receive(lm75_handle,&reg,1,data,2,-1);
		int16_t raw =((int16_t)data[0] << 8) |data[1];
        	raw >>= 7;
        	float Sensor_temperature = raw * 0.5;
        	printf("Sensor_Temperature = %.1f C\n",Sensor_temperature);
        	vTaskDelay(pdMS_TO_TICKS(1000));
		//Potentiometer reading
		adc_oneshot_read(adc1_handle,ADC_CHANNEL_0,&adc_value);
		printf("ADC Value = %d\n", adc_value);
		float target_temp =20.0 +((float)adc_value / 4095.0) * 20.0;
		printf("Target Temperature = %.1f C\n",target_temp);
		vTaskDelay(pdMS_TO_TICKS(1000));
		if (Sensor_temperature<=target_temp)
			{ 
				duty=0;// fan off
				gpio_set_level(LED0,1);// Green LED
				gpio_set_level(LED1,0);
				gpio_set_level(LED2,0);
				uart_write_bytes(UART_PORT,
                         " Fan off - Normal Temperature\n",strlen("Fan off - Normal Temperature\n"));
			}
		else if((Sensor_temperature-target_temp)>=5)
			{ 	
				duty=1023;// Full speed
				gpio_set_level(LED2,1);// Red LED
				gpio_set_level(LED0,0);
				gpio_set_level(LED1,0);
				uart_write_bytes(UART_PORT,
                         " Fan full speed - high Temperature\n",strlen("Fan full speed - high Temperature\n"));
			}
		else if((Sensor_temperature-target_temp)>=2)
			{	
				duty=614;// Medium Speed   	
				gpio_set_level(LED1,1);// Yellow LED
				gpio_set_level(LED0,0);
				gpio_set_level(LED2,0);
				uart_write_bytes(UART_PORT,
                         " Fan medium- Moderate Temperature\n",strlen("Fan medium - Moderate Temperature\n"));
			}
		else
			{ 	
				gpio_set_level(LED1,1);// Yellow LED
				duty=307;// low speed
				uart_write_bytes(UART_PORT,
                         " Fan low speed - light high Temperature\n",
			strlen("Fan low speed - light high Temperature\n"));
			}
		ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,duty);
                ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0);
}
}

