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
#ifndef _GUI_H_
#define _GUI_H_
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
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
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/
uint32_t mypow(uint32_t base, uint8_t exp);
/**
 * @func	LED_DrawPoint
 * @brief	draw a point in LCD screen
 * @param	x:the x coordinate of the point
			y:the y coordinate of the point
			color:the color value of the point (WHITE, BLACK, BLUE, MAGENTA,...)
 * @retval	None
*/
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

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
		uint16_t ax, uint16_t ay,
		uint16_t bx, uint16_t by,
		uint16_t color, int fill
);

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
);

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
		uint16_t x1, uint16_t y1,
		uint16_t x2, uint16_t y2,
		uint16_t color
);

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
void LCD_Circle(int xc, int yc,
				uint16_t c,
				int r, int fill);

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
);

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
void LCD_ShowChar(uint16_t x, uint16_t y,
				  uint16_t color, uint16_t background,
				  uint8_t num, uint8_t size, uint8_t mode);

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
);

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
void LCD_ShowNum(uint16_t x, uint16_t y,
				 uint32_t num, uint8_t len, uint8_t size);

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
void LCD_ShowCentredString(uint16_t color, uint16_t background,
	    uint8_t *text, uint8_t size, uint8_t mode);

/**
 * @func	LCD_DrawBMP16
 * @brief	Display a 16-bit BMP image
 * @param	x:	the bebinning x coordinate of the BMP image
 *			y:	the bebinning y coordinate of the BMP image
 *			p:	the start address of image array
 * @retval	None
*/
void LCD_DrawBMP16(uint16_t x, uint16_t y,
				   const unsigned char *p);

/**
 * @func	LCD_ShowImg
 * @brief	Show image with DMA
 * @param	None
 * @retval	None
*/
void LCD_ShowImg(uint8_t width, uint8_t height);

#endif

/* END FILE */
