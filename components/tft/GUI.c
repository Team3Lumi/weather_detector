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
#include "string.h"
#include "font.h"
#include "GUI.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
/******************************************************************************/
/*                            PRIVATE FUNCTIONS                              */
/******************************************************************************/
static void _draw_circle_8(int xc, int yc, int x, int y, uint16_t color);
static void _swap(uint16_t *a, uint16_t *b);
static void LCD_WR_REG(uint8_t data);
static void LCD_WR_DATA(uint16_t data);
static void LCD_WR_DATA8(uint8_t data);
static void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
static void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd ,uint16_t yEnd);
static uint8_t LCDSPI_WriteByte(uint8_t data);
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
 * @param	Xpos:	the x coordinate of the pixel
			Ypos: 	the y coordinate of the pixel
 * @retval	None
*/
static
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
}

/**
 * @func	_draw_circle_8
 * @brief	8 symmetry circle drawing algorithm (internal call)
 * @param	xc: 	the x coordinate of the Circular center
			yc: 	the y coordinate of the Circular center
			x:  	the x coordinate relative to the Circular center
			y:  	the y coordinate relative to the Circular center
			color: 	the color value of the circle (WHITE, BLACK, BLUE, MAGENTA,...)
 * @retval	None
*/
static
void _draw_circle_8(
		int xc, int yc,
		int x, int y,
		uint16_t color
) {
	LCD_DrawPoint(xc + x, yc + y, color);
	LCD_DrawPoint(xc - x, yc + y, color);
	LCD_DrawPoint(xc + x, yc - y, color);
	LCD_DrawPoint(xc - x, yc - y, color);
	LCD_DrawPoint(xc + y, yc + x, color);
	LCD_DrawPoint(xc - y, yc + x, color);
	LCD_DrawPoint(xc + y, yc - x, color);
	LCD_DrawPoint(xc - y, yc - x, color);
}

/**
 * @func    _swap
 * @brief   Swaps the values of two uint16_t variables.
 * @param   a: Pointer to the first variable.
 *          b: Pointer to the second variable.
 * @retval  None
 */
static 
void _swap(uint16_t *a, uint16_t *b) {
    uint16_t tmp; // Temporary variable to hold the value during swapping
    tmp = *a; // Store the value of 'a' in 'tmp'
    *a = *b;   // Assign the value of 'b' to 'a'
    *b = tmp; // Assign the original value of 'a' (stored in 'tmp') to 'b'
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
 * @brief	draw a point in LCD screen
 * @param	x:the x coordinate of the point
			y:the y coordinate of the point
			color:the color value of the point (WHITE, BLACK, BLUE, MAGENTA,...)
 * @retval	None
*/
void LCD_DrawPoint(
		uint16_t x, uint16_t y,
		uint16_t color
) {
	LCD_SetCursor(x,y);
	LCD_WR_DATA(color);
}

/**
 * @func    LCD_DrawArea
 * @brief   Draws or fills the specified area on the LCD.
 * @param   x0: The starting x-coordinate of the specified area.
 *          y0: The starting y-coordinate of the specified area.
 *          x1: The ending x-coordinate of the specified area.
 *          y1: The ending y-coordinate of the specified area.
 *          color: The color value to fill or draw the area with. (WHITE, BLACK, BLUE, MAGENTA,...)
 *          fill: Flag indicating whether to fill the area (1) or draw its boundary (0).
 * @retval  None
 */
void LCD_DrawArea(
    uint16_t x0, uint16_t y0,
    uint16_t x1, uint16_t y1,
    uint16_t color, int fill
) {
    uint16_t i, j;
    uint16_t width = x1 - x0 + 1; // Calculate the width of the area
    uint16_t height = y1 - y0 + 1; // Calculate the height of the area

    // Set the drawing window to the specified area
    LCD_SetWindows(x0, y0, x1, y1);

    // Fill the area with the specified color if the fill flag is set
    if (fill) {
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                // Write color data to the LCD
                LCD_WR_DATA(color);
            }
        }
    } else {
        // Draw the boundary of the area
        // Draw horizontal lines at the top and bottom edges
        LCD_DrawLine(x0, y0, x1, y0, color); // Top
        LCD_DrawLine(x0, y1, x1, y1, color); // Bottom
        // Draw vertical lines at the left and right edges
        LCD_DrawLine(x0, y0, x0, y1, color); // Left
        LCD_DrawLine(x1, y0, x1, y1, color); // Right
    }

    // Reset the drawing window to the full screen
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
}

/**
 * @func    LCD_DrawFillBox
 * @brief   Draws a filled box on the LCD.
 * @param   x: The starting x-coordinate of the filled box.
 *          y: The starting y-coordinate of the filled box.
 *          w: The width of the filled box.
 *          h: The height of the filled box.
 *          color: The color value to fill the box with or draw the boundary. (WHITE, BLACK, BLUE, MAGENTA,...)
 *          fill: Flag indicating whether to fill the box (1) or draw its boundary (0).
 * @retval  None
 */
void LCD_DrawFillBox(
    uint16_t x, uint16_t y,
    uint16_t w, uint16_t h,
    uint16_t color, int fill
) {
    // Check if the box should be filled
    if (fill) {
        // Fill the box with the specified color
        LCD_DrawArea(x, y, x + w - 1, y + h - 1, color, 1);
    } else {
        // Draw the boundary of the box with the specified color
        LCD_DrawArea(x, y, x + w - 1, y + h - 1, color, 0);
    }
}

/**
 * @func    LCD_DrawLine
 * @brief   Draws a line between two points on the LCD.
 * @param   x0: The starting x-coordinate of the line.
 *          y0: The starting y-coordinate of the line.
 *          x1: The ending x-coordinate of the line.
 *          y1: The ending y-coordinate of the line.
 *          color: Color of the line.
 * @retval  None
 */
void LCD_DrawLine(
    uint16_t x0, uint16_t y0,
    uint16_t x1, uint16_t y1,
    uint16_t color
) {
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x1 - x0;
    delta_y = y1 - y0;
    uRow = x0;
    uCol = y0;

    // Determine the increment direction for x and y
    if (delta_x > 0) incx = 1;
    else if (delta_x == 0) incx = 0;
    else {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0;
    else {
        incy = -1;
        delta_y = -delta_y;
    }

    // Determine the maximum distance between points
    if (delta_x > delta_y) distance = delta_x;
    else distance = delta_y;

    // Draw the line using Bresenham's algorithm
    for (t = 0; t <= distance + 1; t++) {
        // Draw a point at the current position
        LCD_DrawPoint(uRow, uCol, color);

        // Update error values
        xerr += delta_x;
        yerr += delta_y;

        // Update x-coordinate if error exceeds distance
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }

        // Update y-coordinate if error exceeds distance
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/**
 * @func    LCD_Circle
 * @brief   Draws a circle of specified size at a specified location on the LCD.
 * @param   xc: The x-coordinate of the center of the circle.
 *          yc: The y-coordinate of the center of the circle.
 *          color: Color of the circle or circle boundary. (WHITE, BLACK, BLUE, MAGENTA,...)
 *          r: Radius of the circle.
 *          fill: Flag indicating whether to fill the circle (1) or draw its boundary (0).
 * @retval  None
 */
void LCD_Circle(
    int xc, int yc,
    uint16_t color,
    int r, int fill
) {
    int x = 0, y = r, yi, d;

    d = 3 - 2 * r;

    // Draw the circle with or without filling based on the fill flag
    if (fill) {
        // Fill the circle
        while (x <= y) {
            for (yi = x; yi <= y; yi++) {
                _draw_circle_8(xc, yc, x, yi, color); // Draw points in eight octants
            }
            // Update decision parameter
            if (d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    } else {
        // Draw the circle boundary
        while (x <= y) {
            _draw_circle_8(xc, yc, x, y, color); // Draw points in eight octants
            // Update decision parameter
            if (d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    }
}

/**
 * @func    LCD_DrawTriangle
 * @brief   Draws a triangle either filled or not on the LCD.
 * @param   x0: The starting x-coordinate of the first edge of the triangle.
 *          y0: The starting y-coordinate of the first edge of the triangle.
 *          x1: The x-coordinate of the vertex of the triangle.
 *          y1: The y-coordinate of the vertex of the triangle.
 *          x2: The ending x-coordinate of the second edge of the triangle.
 *          y2: The ending y-coordinate of the second edge of the triangle.
 *          color: Color of the triangle or color of the boundary. (WHITE, BLACK, BLUE, MAGENTA,...)
 *          fill: Flag indicating whether to fill the triangle (1) or draw its boundary (0).
 * @retval  None
 */
void LCD_DrawTriangle(
    uint16_t x0, uint16_t y0,
    uint16_t x1, uint16_t y1,
    uint16_t x2, uint16_t y2,
    uint16_t color, int fill
) {
    uint16_t a, b, y, last;
    int dx01, dy01, dx02, dy02, dx12, dy12;
    long sa = 0;
    long sb = 0;

    // If the triangle needs to be filled
    if (fill) {
        // Ensure y0 <= y1 <= y2
        if (y0 > y1) {
            _swap(&y0, &y1);
            _swap(&x0, &x1);
        }
        if (y1 > y2) {
            _swap(&y2, &y1);
            _swap(&x2, &x1);
        }
        if (y0 > y1) {
            _swap(&y0, &y1);
            _swap(&x0, &x1);
        }
        // Handle the case of a flat-bottom triangle (y0 == y2)
        if (y0 == y2) {
            a = b = x0;
            if (x1 < a) a = x1;
            else if (x1 > b) b = x1;
            if (x2 < a) a = x2;
            else if (x2 > b) b = x2;
            // Draw a horizontal line to fill the flat-bottom triangle
            LCD_DrawArea(a, y0, b, y0, color, 1);
            return;
        }
        // Calculate the differences in x and y coordinates
        dx01 = x1 - x0;
        dy01 = y1 - y0;
        dx02 = x2 - x0;
        dy02 = y2 - y0;
        dx12 = x2 - x1;
        dy12 = y2 - y1;
        // Determine the last scanline of the triangle
        if (y1 == y2) last = y1;
        else last = y1 - 1;
        // Scan each scanline of the triangle to fill it
        for (y = y0; y <= last; y++) {
            // Calculate the left and right endpoints of the scanline
            a = x0 + sa / dy01;
            b = x0 + sb / dy02;
            sa += dx01;
            sb += dx02;
            // Ensure a <= b
            if (a > b) _swap(&a, &b);
            // Draw a horizontal line to fill the current scanline
            LCD_DrawArea(a, y, b, y, color, 1);
        }
        // Adjust the step for the next scanline
        sa = dx12 * (y - y1);
        sb = dx02 * (y - y0);
        // Scan each scanline of the triangle to fill the bottom part
        for (; y <= y2; y++) {
            // Calculate the left and right endpoints of the scanline
            a = x1 + sa / dy12;
            b = x0 + sb / dy02;
            sa += dx12;
            sb += dx02;
            // Ensure a <= b
            if (a > b) _swap(&a, &b);
            // Draw a horizontal line to fill the current scanline
            LCD_DrawArea(a, y, b, y, color, 1);
        }
    } else {
        // If the triangle should not be filled, draw its boundary
        LCD_DrawLine(x0, y0, x1, y1, color);
        LCD_DrawLine(x1, y1, x2, y2, color);
        LCD_DrawLine(x2, y2, x0, y0, color);
    }
}


/**
 * @func	LCD_ShowChar
 * @brief	Display a single English character
 * @param	x:			the bebinning x coordinate of the Character display position
			y:			the bebinning y coordinate of the Character display position
			color:		the color value of display character. (WHITE, BLACK, BLUE, MAGENTA,...)
			background:	the background color of display character. (WHITE, BLACK, BLUE, MAGENTA,...)
			num:		the ascii code of display character (0~94)
			size:		the size of display character (>=12)
			mode:		0-no overlying, 1-overlying
 * @retval	None
*/
void LCD_ShowChar(
		uint16_t x,  uint16_t y,
		uint16_t color, uint16_t background,
		uint8_t num, uint8_t size, uint8_t mode
) {
	uint8_t temp;
	uint8_t pos,t;
	uint16_t colortemp=POINT_COLOR;

	num=num-' ';
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);
	if(!mode)
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];
			for(t=0;t<size/2;t++)
		    {
		        if(temp&0x01)LCD_WR_DATA(color);
				else LCD_WR_DATA(background);
				temp>>=1;

		    }

		}
	}else
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];
			for(t=0;t<size/2;t++)
		    {
				POINT_COLOR=color;
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos, color);
		        temp>>=1;
		    }
		}
	}
	POINT_COLOR=colortemp;
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
}

/**
 * @func	LCD_ShowString
 * @brief	Display string
 * @param	x:			the bebinning x coordinate of the string
			y:			the bebinning y coordinate of the string
			color: 		the color of the font. (WHITE, BLACK, BLUE, MAGENTA,...)
			background: the color of the background. (WHITE, BLACK, BLUE, MAGENTA,...)
			text:		the start address of the string
			size:		the size of display character (>= 12 to display)
			mode:		0-no overlying, 1-overlying
 * @retval	None
*/
void LCD_ShowString(
    uint16_t x, uint16_t y,
    uint16_t color, uint16_t background,
    uint8_t *text, uint8_t size, uint8_t mode
) {
	uint16_t colortemp=POINT_COLOR;
    while (*text != '\0') {
        if (*text >= ' ' && *text <= '~') {
            // Display printable ASCII characters
            uint8_t num = *text - ' '; // Convert to index suitable for font array
            LCD_SetWindows(x, y, x + size / 2 - 1, y + size - 1);

            if (!mode) {
                for (uint8_t pos = 0; pos < size; pos++) {
                    uint8_t temp = (size == 12) ? asc2_1206[num][pos] : asc2_1608[num][pos];
                    for (uint8_t t = 0; t < size / 2; t++) {
                        LCD_WR_DATA((temp & 0x01) ? color : background);
                        temp >>= 1;
                    }
                }
            } else {
                for (uint8_t pos = 0; pos < size; pos++) {
                    uint8_t temp = (size == 12) ? asc2_1206[num][pos] : asc2_1608[num][pos];
                    for (uint8_t t = 0; t < size / 2; t++) {
                        POINT_COLOR = color;
                        if (temp & 0x01) LCD_DrawPoint(x + t, y + pos, color);
                        temp >>= 1;
                    }
                }
            }
            POINT_COLOR = colortemp;
            LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
            x += size / 2; // Adjust x position for the next character
        }
        text++; // Move to the next character in the string
    }
}

/**
 * @func	LCD_ShowNum
 * @brief	Display number
 * @param	x:		the bebinning x coordinate of the number
			y:		the bebinning y coordinate of the number
			num:	the number(0~4294967295)
			len:	the length of the display number
			size:	the size of display number (>= 12)
 * @retval	None
*/
void LCD_ShowNum(
		uint16_t x, uint16_t y,
		uint32_t num,
		uint8_t len, uint8_t size
) {
	uint8_t t,temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,' ',size,0);
				continue;
			}else enshow=1;

		}
	 	LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,temp+'0',size,0);
	}
}

uint32_t mypow(uint32_t base, uint8_t exp) {
    uint32_t result = 1;
    while (exp > 0) {
        result *= base;
        exp--;
    }
    return result;
}

/**
 * @func	LCD_ShowCentredString
 * @brief	Show string in the middle of the LCD display
 * @param	color:		the color value of strings. (WHITE, BLACK, BLUE, MAGENTA,...)
 *			background:	the background color of strings. (WHITE, BLACK, BLUE, MAGENTA,...)
 *			str:		the start address of the Chinese and English strings
 *			size:		the size of English strings
 *			mode:		0-no overlying,1-overlying
 * @retval	None
*/
void LCD_ShowCentredString(
    uint16_t color, uint16_t background,
    uint8_t *text, uint8_t size, uint8_t mode
) {
    uint16_t len = strlen((const char *)text);
    uint16_t x1 = (lcddev.width - len * size / 2) / 2;
    uint16_t y1 = (lcddev.height - size) / 2; // Calculate starting y-coordinate for vertical centering
    LCD_ShowString(x1, y1, color, background, text, size, mode);
}

/**
 * @func	LCD_DrawBMP16
 * @brief	Display a 16-bit BMP image
 * @param	x:	the bebinning x coordinate of the BMP image
 *			y:	the bebinning y coordinate of the BMP image
 *			p:	the start address of image array
 * @retval	None
*/
void LCD_DrawBMP16(
		uint16_t x, uint16_t y,
		const unsigned char *p
) {
  	int i;
	unsigned char picH,picL;
	LCD_SetWindows(x,y,x+40-1,y+40-1);

    for(i=0;i<40*40;i++) {
	 	picL=*(p+i*2);
		picH=*(p+i*2+1);
		LCD_WR_DATA(picH<<8|picL);
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
}

void LCD_ShowImg(uint8_t width, uint8_t height) {
    uint16_t x, y;

    // 1. Thiết lập vùng hiển thị (Address Window)
    LCD_SetWindows(0, 0, width - 1, height - 1);

    // 2. Gửi toàn bộ dữ liệu ảnh (mảng bmp1) qua SPI
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Tính chỉ số pixel trong mảng bmp1 (mỗi pixel chiếm 2 byte)
            uint32_t index = (y * width + x) + 1;
            
            // Đọc 2 byte dữ liệu và ghép thành giá trị RGB565
            uint16_t color = image_data_160x128x16[index];

            // Gửi pixel qua SPI (hoặc DMA nếu hỗ trợ)
            LCD_WR_DATA(color);
        }
    }
}


/* END FILE */
