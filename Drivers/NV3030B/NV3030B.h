/*
  ******************************************************************************
  * @file 			( фаил ):   NV3030B.h
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	 author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
 */


#ifndef _NV3030B_H
#define _NV3030B_H


/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

// Обязательно нужен #include "main.h" 
// чтоб отдельно не подключать файлы связанные с МК и стандартными библиотеками

#include "main.h"
#include "fonts.h"

#include "stdlib.h"
#include "string.h"
#include "math.h"



//#######  SETUP  ##############################################################################################

//==== выбераем через что будем отправлять через HAL или CMSIS(быстрее) ==================
//-- нужное оставляем другое коментируем ( важно должно быть только один выбран )---------

// указываем порт SPI для CMSIS ( быстро )-------
// так как у разных МК разные регистры то в функциях корректируем под свой МК
// на данный момент есть реализация на серию F1 F4 H7 для выбора серии в функциях
//	void NV3030B_SendCmd(uint8_t Cmd);
//	void NV3030B_SendData(uint8_t Data );
//	void NV3030B_SendDataMASS(uint8_t* buff, size_t buff_size);
// комментируем и раскомментируем то что нам нужно, также там же редактируем под свой МК если не работает
#define 	NV3030B_SPI_CMSIS 	SPI1
//-----------------------------------------------

// указываем порт SPI для HAL ( медлено )--------
//#define    NV3030B_SPI_HAL        hspi1
//-----------------------------------------------

//============================================================================

// выбираем как выводить информацию через буфер кадра или попиксельно ( 1-буфер кадра, 0-попиксельный вывод ) -----
// через буфер быстре если много информации обнавлять за один раз ( требует много оперативки для массива )
// по пиксельно рисует онлайн буз буферра если информация обновляеться немного то выгодно испотзовать данный режим
#define FRAME_BUFFER                0
//-----------------------------------------------------------------------------------------------------------------


//=== указываем порты ( если в кубе назвали их DC RES CS то тогда нечего указывать не нужно )
#if defined (DC_GPIO_Port)
#else
#define DC_GPIO_Port	GPIOC
#define DC_Pin			GPIO_PIN_5
#endif

#if defined (RST_GPIO_Port)
#else
#define RST_GPIO_Port   GPIOB
#define RST_Pin			GPIO_PIN_14
#endif

//--  Cесли используем порт CS для выбора устройства тогда раскомментировать ------------
// если у нас одно устройство лучше пин CS притянуть к земле( или на порту подать GND )

#define CS_PORT

//----------------------------------------------------------------------------------------
#ifdef CS_PORT
#if defined (CS_GPIO_Port)
#else
#define CS_GPIO_Port    GPIOB
#define CS_Pin			GPIO_PIN_12
#endif
#endif

//=============================================================================

//==  выбираем дисплей: =======================================================
//-- нужное оставляем другое коментируем ( важно должно быть только один выбран )---------

//#define	NV3030B_IS_135X240		// 1.14" 135 x 240 NV3030B
//#define	NV3030B_IS_240X240		// 1.3" 240 x 240 NV3030B
//#define	NV3030B_IS_172X320		// 1.47" 172 x 320 NV3030B
#define    NV3030B_IS_240X280        // 1.83" 240 x 280 NV3030B


//=============================================================================


//##############################################################################################################

#ifdef NV3030B_SPI_HAL
extern SPI_HandleTypeDef NV3030B_SPI_HAL;
#endif

extern uint16_t NV3030B_Width, NV3030B_Height;

extern uint16_t NV3030B_X_Start;
extern uint16_t NV3030B_Y_Start;

#define RGB565(r, g, b)         (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define PI    3.14159265

//--- готовые цвета ------------------------------
#define    NV3030B_BLACK            0x0000
#define    NV3030B_BLUE             0x001F
#define    NV3030B_RED              0xF800
#define    NV3030B_GREEN            0x07E0
#define    NV3030B_CYAN             0x07FF
#define    NV3030B_MAGENTA          0xF81F
#define    NV3030B_YELLOW           0xFFE0
#define    NV3030B_WHITE            0xFFFF
#define    NV3030B_AMBER            0xFB60  // Более светлый янтарь
#define    NV3030B_AMBER_DARK       0xDA40  // Темный янтарь
//------------------------------------------------

//-- Битовые маски настройки цветности NV3030B ----
#define NV3030B_ColorMode_65K        0x50
#define NV3030B_ColorMode_262K    0x60
#define NV3030B_ColorMode_12bit    0x03
#define NV3030B_ColorMode_16bit    0x05
#define NV3030B_ColorMode_18bit    0x06
#define NV3030B_ColorMode_16M        0x07
//------------------------------------------------

#define NV3030B_MADCTL_MY                0x80
#define NV3030B_MADCTL_MX                0x40
#define NV3030B_MADCTL_MV                0x20
#define NV3030B_MADCTL_ML                0x10
#define NV3030B_MADCTL_RGB                0x00
#define NV3030B_MADCTL_BGR                0x08
#define NV3030B_MADCTL_MH                0x04
//-------------------------------------------------


#define NV3030B_SWRESET                        0x01
#define NV3030B_SLPIN                        0x10
#define NV3030B_SLPOUT                        0x11
#define NV3030B_NORON                        0x13
#define NV3030B_INVOFF                        0x20
#define NV3030B_INVON                        0x21
#define NV3030B_DISPOFF                        0x28
#define NV3030B_DISPON                        0x29
#define NV3030B_CASET                        0x2A
#define NV3030B_RASET                        0x2B
#define NV3030B_RAMWR                        0x2C
#define NV3030B_COLMOD                        0x3A
#define NV3030B_MADCTL                        0x36
//-----------------------------------------------

#define DELAY                                        0x80


//###  параметры дисплея 1.3" 240 x 240 NV3030B ###################################

// 1.3" 240 x 240 NV3030B  display, default orientation

#ifdef NV3030B_IS_240X240

#define NV3030B_WIDTH  			240
#define NV3030B_HEIGHT 			240
#define NV3030B_XSTART 			0
#define NV3030B_YSTART 			0
#define NV3030B_ROTATION 		(NV3030B_MADCTL_RGB)

#endif

//##############################################################################


//###  параметры дисплея 1.14" 135 x 240 NV3030B ###################################

// 1.14" 135 x 240 NV3030B  display, default orientation

#ifdef NV3030B_IS_135X240

#define NV3030B_WIDTH  			135
#define NV3030B_HEIGHT 			240
#define NV3030B_XSTART 			52
#define NV3030B_YSTART 			40
#define NV3030B_ROTATION 		(NV3030B_MADCTL_RGB)

#endif

//##############################################################################


//##############################################################################


//###  параметры дисплея 1.47" 172 x 320 NV3030B ###################################

// 1.47" 172 x 320 NV3030B display, default orientation

#ifdef NV3030B_IS_172X320

#define NV3030B_WIDTH  			320
#define NV3030B_HEIGHT 			172
#define NV3030B_XSTART 			0
#define NV3030B_YSTART 			34
#define NV3030B_ROTATION 		(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB)

#endif

//##############################################################################


//##############################################################################


//###  параметры дисплея 1.83" 240 x 280 NV3030B ###################################

// 1.83" 240 x 280 NV3030B  display, default orientation

#ifdef NV3030B_IS_240X280

#define NV3030B_WIDTH            240
#define NV3030B_HEIGHT            280
#define NV3030B_XSTART            0
#define NV3030B_YSTART            20
#define NV3030B_ROTATION        (NV3030B_MADCTL_RGB)

#endif

//##############################################################################


//##############################################################################


//###  параметры дисплея 2" 240 x 320 NV3030B ###################################

// 2" 240 x 320 NV3030B  display, default orientation

#ifdef NV3030B_IS_240X320

#define NV3030B_WIDTH  			240
#define NV3030B_HEIGHT 			320
#define NV3030B_XSTART 			0
#define NV3030B_YSTART 			0
#define NV3030B_ROTATION 		(NV3030B_MADCTL_RGB)

#endif

//##############################################################################
//##############################################################################


void NV3030B_Init(void);

void NV3030B_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

void NV3030B_HardReset(void);

void NV3030B_SleepModeEnter(void);

void NV3030B_SleepModeExit(void);

void NV3030B_ColorModeSet(uint8_t ColorMode);

void NV3030B_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror, uint8_t IsBGR);

void NV3030B_InversionMode(uint8_t Mode);

void NV3030B_FillScreen(uint16_t color);

void NV3030B_Clear(void);

void NV3030B_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void NV3030B_SetBL(uint8_t Value);

void NV3030B_DisplayPower(uint8_t On);

void NV3030B_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

void NV3030B_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor);

void NV3030B_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

void NV3030B_DrawLineWithAngle(int16_t x, int16_t y, uint16_t length, double angle_degrees, uint16_t color);

void NV3030B_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

void NV3030B_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                                uint16_t color);

void NV3030B_DrawPixel(int16_t x, int16_t y, uint16_t color);

void NV3030B_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor);

void NV3030B_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);

void NV3030B_DrawEllipse(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, uint16_t color);

void NV3030B_DrawEllipseFilled(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, uint16_t color);

void NV3030B_DrawEllipseFilledWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees,
                                        uint16_t color);

void NV3030B_DrawEllipseWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees,
                                  uint16_t color);

void
NV3030B_DrawChar(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t *Font,
                 uint8_t multiplier, unsigned char ch);

void NV3030B_DrawCharWithAngle(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg,
                               FontDef_t *Font, uint8_t multiplier, double angle_degrees, unsigned char ch);

void NV3030B_print(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t *Font,
                   uint8_t multiplier, char *str);

void NV3030B_printWithAngle(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg,
                            FontDef_t *Font, uint8_t multiplier, double angle_degrees, char *str);

void NV3030B_rotation(uint8_t rotation);

void NV3030B_DrawBitmap(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color);

void
NV3030B_DrawBitmapWithAngle(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color,
                            double angle_degrees);

void NV3030B_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, uint16_t color);

void NV3030B_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);

void
NV3030B_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color);

void NV3030B_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color);

void NV3030B_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color,
                     uint8_t thick);

void NV3030B_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t thick);

void NV3030B_DrawLineThickWithAngle(int16_t x, int16_t y, int16_t length, double angle_degrees, uint16_t color,
                                    uint8_t thick);

#if FRAME_BUFFER
void NV3030B_Update(void);
void NV3030B_ClearFrameBuffer(void);
#endif


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif    /*	_NV3030B_H */

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
