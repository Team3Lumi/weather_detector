/*******************************************************************************
 * ST7735S for ESP-IDF
 *
 * Author: TanPN
 *
 * Last Changed By:  $Author: TanPN $
 * Revision:         $Revision: 2.1$
 * Last Changed:     $Date: 12/06/2024 $
 *
*******************************************************************************/
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "lcd.h"
#include "soc/spi_periph.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
_lcd_dev lcddev;
uint16_t POINT_COLOR = 0x0000;
uint16_t BACK_COLOR = 0xFFFF;
extern const unsigned char bmp1[40960];

spi_device_handle_t spi;
/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/
static void LCDGPIO_Config(void);
static void LCDSPI_Config(void);
static void LCD_RESET(void);
static uint8_t LCDSPI_WriteByte(uint8_t data);
static void LCD_WR_REG(uint8_t data);
static void LCD_WR_DATA(uint16_t data);
static void LCD_WR_DATA8(uint8_t data);
static void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
static void LCD_WriteRAM_Prepare(void);
static void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
static void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd ,uint16_t yEnd);
/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

/**
 * @func	LCD_WR_REG
 * @brief	Write an 8-bit command to the LCD screen
 * @param	Command value to be written
 * @retval  None
*/
static
void LCD_WR_REG(uint8_t data)
{
	LCD_SPI_CS_RESET;					//LCD_CS=0
	LCD_SPI_RS_RESET;
	LCDSPI_WriteByte(data);
	LCD_SPI_CS_SET;						//LCD_CS=1
}

/**
 * @func	LCD_WR_DATA
 * @brief	Write an 16-bit data to the LCD screen
 * @param	data value to be written
 * @retval  None
*/
static
void LCD_WR_DATA(uint16_t data)
{
	LCD_SPI_CS_RESET;					//LCD_CS=0
	LCD_SPI_RS_SET;
	LCDSPI_WriteByte(data>>8);
	LCDSPI_WriteByte(data);
	LCD_SPI_CS_SET;						//LCD_CS=1
}

/**
 * @func	LCD_WR_DATA
 * @brief	Write an 8-bit data to the LCD screen
 * @param	data value to be written
 * @retval  None
*/
static
void LCD_WR_DATA8(uint8_t data)
{
	LCD_SPI_CS_RESET;					//LCD_CS=0
	LCD_SPI_RS_SET;
	LCDSPI_WriteByte(data);
	LCD_SPI_CS_SET;						//LCD_CS=1
}

/**
 * @func   	LCD_WriteReg
 * @brief	Write data into registers
 * @param 	LCD_Reg:Register address
            LCD_RegValue: Data to be written
 * @retval  None
*/
static
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA8(LCD_RegValue);
}

/**
 * @func	LCD_WriteRAM_Prepare
 * @brief	Write GRAM
 * @param	None
 * @retval	None
*/
static
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}

/**
 * @func	LCDGPIO_Config
 * @brief	Initialization LCD screen GPIO
 * @param	None
 * @retval	None
*/
static
void LCDGPIO_Config(void)
{
	gpio_config_t LCDGPIO_InitTypeDef;

    LCDGPIO_InitTypeDef.intr_type    = GPIO_INTR_DISABLE;
    LCDGPIO_InitTypeDef.mode         = GPIO_MODE_OUTPUT;
    LCDGPIO_InitTypeDef.pin_bit_mask = (1ULL << LCD_RST_PIN) | (1ULL << LCD_AO_PIN) | (1ULL << LCD_LED_PIN);
    LCDGPIO_InitTypeDef.pull_down_en = 0;
    LCDGPIO_InitTypeDef.pull_up_en   = 0;

    gpio_config(&LCDGPIO_InitTypeDef);

	/* Turn on Led Background of LCD */
	gpio_set_level(LCD_LED_PIN, 1);
}

/**
 * @func   LCDSPI_Config
 * @brief  Initializes peripheral SPI
 * @param  None
 * @retval None
 */
static
void LCDSPI_Config(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = LCD_SPI_GPIO_MOSI,
        .sclk_io_num = LCD_SPI_GPIO_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8
    };
    spi_device_interface_config_t devcfg = {

        .clock_speed_hz = 10 * 1000 * 1000,     //Clock out at 10 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = LCD_CS_PIN,             //CS pin
        .queue_size = 7,                        //We want to be able to queue 7 transactions at a time
        // .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(LCD_SPI, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_SPI, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}


/**
 * @func	LCD_RESET
 * @brief	Reset LCD screen
 * @param	None
 * @retval	None
*/
static
void LCD_RESET(void)
{
	LCD_SPI_RST_RESET;
	vTaskDelay(pdMS_TO_TICKS(100));
	LCD_SPI_RST_SET;
	vTaskDelay(pdMS_TO_TICKS(50));
}

/**
 * @func	LCDSPI_WriteByte
 * @brief	Write a byte of data using STM32's Software SPI
 * @param	data:Data to be written
 * @retval	None
*/
static
uint8_t LCDSPI_WriteByte(
		uint8_t data
) {
    spi_transaction_t transaction = {
        .length = 8,               // Number of bits to send
        .tx_buffer = &data,        // Pointer to the data to send
        .rx_buffer = NULL          // No need to receive for this write
    };

    // Transmit data and wait for the transaction to complete
    esp_err_t ret = spi_device_transmit(spi, &transaction);
    if (ret != ESP_OK) {
        printf("SPI transmit failed: %s\n", esp_err_to_name(ret));
        return 0; // Return 0 if an error occurs
    }

    // If you need to receive a response, you can use the `rx_buffer`
    return data; // Return the transmitted byte
}

/**
 * @func	LCD_SetCursor
 * @brief	Set coordinate value
 * @param	Xpos:the  x coordinate of the pixel
			Ypos:the  y coordinate of the pixel
 * @retval	None
*/
static
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
}

/**
 * @func	LCD_SetWindows
 * @brief	Setting LCD display window
 * @param	xStar:the bebinning x coordinate of the LCD display window
			yStar:the bebinning y coordinate of the LCD display window
			xEnd:the endning x coordinate of the LCD display window
			yEnd:the endning y coordinate of the LCD display window
 * @retval	None
*/
static
void LCD_SetWindows(
		uint16_t xStar, uint16_t yStar,
		uint16_t xEnd ,uint16_t yEnd
) {
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(xStar);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(xEnd);

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(yStar);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(yEnd);

	LCD_WriteRAM_Prepare();
}

/**
 * @func	LCD_DrawPoint
 * @brief	Write a pixel data at a specified location
 * @param	x:the x coordinate of the pixel
            y:the y coordinate of the pixel
            color: color of the point
 * @retval	None
*/

/**
 * @func	LCD_Init
 * @brief	Initialization LCD screen
 * @param	None
 * @retval	None
*/
void LCD_Init(void)
{
	LCDSPI_Config();
	LCDGPIO_Config();
	LCD_RESET();

	lcddev.width=128;
	lcddev.height=160;
	lcddev.wramcmd=0X2C;
	lcddev.setxcmd=0X2A;
	lcddev.setycmd=0X2B;

	LCD_WR_REG(0x11); //Sleep out
	vTaskDelay(pdMS_TO_TICKS(120)); //Delay 120ms
	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	LCD_WR_REG(0xB1);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_REG(0xB3);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x05);
	LCD_WR_DATA8(0x3C);
	LCD_WR_DATA8(0x3C);
	//------------------------------------End ST7735S Frame Rate-----------------------------------------//
	LCD_WR_REG(0xB4); //Dot inversion
	LCD_WR_DATA8(0x03);
	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x04);
	LCD_WR_REG(0xC1);
	LCD_WR_DATA8(0XC0);
	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x00);
	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0x2A);
	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x8D);
	LCD_WR_DATA8(0xEE);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	LCD_WR_REG(0xC5); //VCOM
	LCD_WR_DATA8(0x1A);
	LCD_WR_REG(0x36); //MX, MY, RGB mode
	LCD_WR_DATA8(0xC0);
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x22);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x0A);
	LCD_WR_DATA8(0x2E);
	LCD_WR_DATA8(0x30);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x2A);
	LCD_WR_DATA8(0x28);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x2E);
	LCD_WR_DATA8(0x3A);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x03);
	LCD_WR_DATA8(0x13);
	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x06);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x2D);
	LCD_WR_DATA8(0x26);
	LCD_WR_DATA8(0x23);
	LCD_WR_DATA8(0x27);
	LCD_WR_DATA8(0x27);
	LCD_WR_DATA8(0x25);
	LCD_WR_DATA8(0x2D);
	LCD_WR_DATA8(0x3B);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x13);
	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
	LCD_WR_REG(0x3A); //65k mode
	LCD_WR_DATA8(0x05);
	LCD_WR_REG(0x29); //Display on

	LCD_Clear(WHITE);
}

/**
 * @func	LCD_Clear
 * @brief	Full screen filled LCD screen
 * @param	color: Color of the screen (WHITE, BLACK, BLUE, MAGENTA, SKY,...)
 * @retval	None
*/
void LCD_Clear(uint16_t color)
{
	unsigned int i,m;

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
	LCD_SPI_CS_RESET;
	LCD_SPI_RS_SET;

	for(i=0;i<lcddev.height;i++) {
		for(m=0;m<lcddev.width;m++) {
			LCD_WR_DATA(color);
		}
	}

	LCD_SPI_CS_SET;
}

/**
 * @func	LCD_Clear_DMA
 * @brief	Full screen filled LCD screen mode DMA
 * @param	color: Color of the screen (WHITE, BLACK, BLUE, MAGENTA, SKY,...)
 * @retval	None
*/
// LCD_Clear_DMA function in ESP-IDF
void LCD_Clear_DMA(uint16_t color) {
    uint32_t total_pixels = lcddev.width * lcddev.height;
    uint16_t *buffer = heap_caps_malloc(total_pixels * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!buffer) {
        printf("Failed to allocate buffer for DMA\n");
        return;
    }

    // Fill buffer with the color
    for (uint32_t i = 0; i < total_pixels; i++) {
        buffer[i] = color;
    }

    // Prepare the SPI transaction
    spi_transaction_t transaction = {
        .length = total_pixels * 16, // Total transfer size in bits
        .tx_buffer = buffer,         // Pointer to the buffer
        .rx_buffer = NULL,           // No data to receive
    };

    LCD_SPI_CS_RESET;  // Select the LCD
    LCD_SPI_RS_SET; // Set RS for data

    // Transmit the data
    esp_err_t ret = spi_device_transmit(spi, &transaction);
    if (ret != ESP_OK) {
        printf("SPI transaction failed: %s\n", esp_err_to_name(ret));
    }

    LCD_SPI_CS_SET; // Deselect the LCD

    // Free the buffer
    heap_caps_free(buffer);
}

/**
 * @func	LCD_Direction
 * @brief	Setting the display direction of LCD screen
 * @param	direction: (0: 0 degree
                       	1: 90 degree
					   	2: 180 degree
					   	3: 270 degree
 * @retval	None
*/
void LCD_Direction(uint8_t direction)
{
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	lcddev.wramcmd=0x2C;

	switch(direction){
		case 0:
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;
			LCD_WriteReg(0x36,(0<<3)|(1<<6)|(1<<7));//BGR==1,MY==0,MX==0,MV==0
			break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x36,(0<<3)|(1<<7)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
			break;
		case 2:
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;
			LCD_WriteReg(0x36,(0<<3)|(0<<6)|(0<<7));//BGR==1,MY==0,MX==0,MV==0
			break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x36,(0<<3)|(0<<7)|(1<<6)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
			break;

		default:
			break;
	}
}


/* END FILE */
