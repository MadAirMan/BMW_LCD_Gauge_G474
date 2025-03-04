/*

  ******************************************************************************
  * @file 			( фаил ):   NV3030B.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	 author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
*/

#include "NV3030B.h"


uint16_t NV3030B_X_Start = NV3030B_XSTART;
uint16_t NV3030B_Y_Start = NV3030B_YSTART;

uint16_t NV3030B_Width = 0;
uint16_t NV3030B_Height = 0;

#if FRAME_BUFFER
// массив буфер кадра
    uint16_t buff_frame[NV3030B_WIDTH*NV3030B_HEIGHT] = { 0x0000, };
#endif

#pragma GCC diagnostic push       // Сохраняем текущие настройки
#pragma GCC diagnostic ignored "-Wunused-parameter" // Отключаем предупреждение
#pragma GCC diagnostic ignored "-Wunused-variable" // Отключаем предупреждение
#pragma GCC diagnostic ignored "-Wunused-function" // Отключаем предупреждение
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // Отключаем предупреждение

static void NV3030B_ExecuteCommandList(const uint8_t *addr);

static void NV3030B_Unselect(void);

static void NV3030B_Select(void);

static void NV3030B_SendCmd(uint8_t Cmd);

static void NV3030B_SendData(uint8_t Data);

static void NV3030B_SendDataMASS(uint8_t *buff, size_t buff_size);

static void NV3030B_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

static void NV3030B_RamWrite(uint16_t *pBuff, uint32_t Len);

static void NV3030B_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd);

static void NV3030B_RowSet(uint16_t RowStart, uint16_t RowEnd);

static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2);

static void NV3030B_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);


//==== данные для инициализации дисплея NV3030B_240X320 ==========

// инициализация для всех дисплеев одна, так как драйвер расчитан на максимальный размер 240x320
//==============================================================================


//==============================================================================
// Процедура инициализации дисплея
//==============================================================================
void NV3030B_Init(void) {

    // Задержка после подачи питания
    // если при старте не всегда запускаеться дисплей увеличиваем время задержки
    HAL_Delay(50);

    NV3030B_Width = NV3030B_WIDTH;
    NV3030B_Height = NV3030B_HEIGHT;

    NV3030B_Select();

    NV3030B_HardReset();

    HAL_Delay(50);

    // Set the read / write scan direction of the frame memory
    //NV3030B_SendCmd(0x36); 			//MX, MY, RGB mode
    //NV3030B_SendData(0x08);			//0x00 0x70 0x08 set RGB

    NV3030B_SendCmd(0xfd);                //private_access
    NV3030B_SendData(0x06);
    NV3030B_SendData(0x08);

    //NV3030B_SendCmd(0x60);
    //NV3030B_SendData(0x01);			//osc_user_adj[3:0] 07
    //NV3030B_SendData(0x01);
    //NV3030B_SendData(0x01);			//01

    NV3030B_SendCmd(0x61);                //add
    NV3030B_SendData(0x07);
    NV3030B_SendData(0x04);

    NV3030B_SendCmd(0x62);                //bias setting
    NV3030B_SendData(0x00);                //01
    NV3030B_SendData(0x44);                //04 44
    NV3030B_SendData(0x45);                //44 65 40
    //NV3030B_SendData(0x01);			//06 vref_adj

    NV3030B_SendCmd(0x63);
    NV3030B_SendData(0x41);
    NV3030B_SendData(0x07);
    NV3030B_SendData(0x12);
    NV3030B_SendData(0x12);

    NV3030B_SendCmd(0x64);
    NV3030B_SendData(0x37);

    //VSP
    NV3030B_SendCmd(0x65);                //Pump1=4.7MHz //PUMP1 VSP
    NV3030B_SendData(0x09);                //D6-5:pump1_clk[1:0] clamp 28 2b
    NV3030B_SendData(0x10);                //6.26
    NV3030B_SendData(0x21);

    //VSN
    NV3030B_SendCmd(0x66);                //pump=2 AVCL
    NV3030B_SendData(0x09);            //clamp 08 0b 09
    NV3030B_SendData(0x10);            //10
    NV3030B_SendData(0x21);

    //add source_neg_time
    NV3030B_SendCmd(0x67);                //pump_sel
    NV3030B_SendData(0x21);                //21 20
    NV3030B_SendData(0x40);

    //gamma vap/van
    NV3030B_SendCmd(0x68);                //gamma vap/van
    NV3030B_SendData(0x90);                //90
    NV3030B_SendData(0x4c);
    NV3030B_SendData(0x50);                //VCOM
    NV3030B_SendData(0x70);                //66

    NV3030B_SendCmd(0xb1);                //frame rate
    NV3030B_SendData(0x0F);                //0x0f fr_h[5:0] 0F
    NV3030B_SendData(0x02);                //0x02 fr_v[4:0] 02
    NV3030B_SendData(0x01);                //0x04 fr_div[2:0] 04

    NV3030B_SendCmd(0xB4);
    NV3030B_SendData(0x01);            //01:1dot 00:column

    //porch
    NV3030B_SendCmd(0xB5);
    NV3030B_SendData(0x02);                //0x02 vfp[6:0]
    NV3030B_SendData(0x02);                //0x02 vbp[6:0]
    NV3030B_SendData(0x0a);                //0x0A hfp[6:0]
    NV3030B_SendData(0x14);                //0x14 hbp[6:0]

    NV3030B_SendCmd(0xB6);
    NV3030B_SendData(0x04);
    NV3030B_SendData(0x01);
    NV3030B_SendData(0x9f);
    NV3030B_SendData(0x00);
    NV3030B_SendData(0x02);

    //gamme sel
    NV3030B_SendCmd(0xdf);
    NV3030B_SendData(0x11);                //gofc_gamma_en_sel=1

    ////gamma_test1 A1#_wangly
    //3030b_gamma_new_
    //GAMMA---------------------------------/////////////

    //GAMMA---------------------------------/////////////
    NV3030B_SendCmd(0xE2);
    NV3030B_SendData(0x03);                //vrp0[5:0]	V0 03
    NV3030B_SendData(0x00);                //vrp1[5:0]	V1
    NV3030B_SendData(0x00);                //vrp2[5:0]	V2
    NV3030B_SendData(0x30);                //vrp3[5:0]	V61
    NV3030B_SendData(0x33);                //vrp4[5:0]	V62
    NV3030B_SendData(0x3f);                //vrp5[5:0]	V63

    NV3030B_SendCmd(0xE5);
    NV3030B_SendData(0x3f);                //vrn0[5:0]	V63
    NV3030B_SendData(0x33);                //vrn1[5:0]	V62
    NV3030B_SendData(0x30);                //vrn2[5:0]	V61
    NV3030B_SendData(0x00);                //vrn3[5:0]	V2
    NV3030B_SendData(0x00);                //vrn4[5:0]	V1
    NV3030B_SendData(0x03);                //vrn5[5:0]  V0 03

    NV3030B_SendCmd(0xE1);
    NV3030B_SendData(0x05);                //prp0[6:0]	V15
    NV3030B_SendData(0x67);                //prp1[6:0]	V51

    NV3030B_SendCmd(0xE4);
    NV3030B_SendData(0x67);                //prn0[6:0]	V51
    NV3030B_SendData(0x06);                //prn1[6:0]  V15

    NV3030B_SendCmd(0xE0);
    NV3030B_SendData(0x05);                //pkp0[4:0]	V3
    NV3030B_SendData(0x06);                //pkp1[4:0]	V7
    NV3030B_SendData(0x0A);                //pkp2[4:0]	V21
    NV3030B_SendData(0x0C);                //pkp3[4:0]	V29
    NV3030B_SendData(0x0B);                //pkp4[4:0]	V37
    NV3030B_SendData(0x0B);                //pkp5[4:0]	V45
    NV3030B_SendData(0x13);                //pkp6[4:0]	V56
    NV3030B_SendData(0x19);                //pkp7[4:0]	V60

    NV3030B_SendCmd(0xE3);
    NV3030B_SendData(0x18);                //pkn0[4:0]	V60
    NV3030B_SendData(0x13);                //pkn1[4:0]	V56
    NV3030B_SendData(0x0D);                //pkn2[4:0]	V45
    NV3030B_SendData(0x09);                //pkn3[4:0]	V37
    NV3030B_SendData(0x0B);                //pkn4[4:0]	V29
    NV3030B_SendData(0x0B);                //pkn5[4:0]	V21
    NV3030B_SendData(0x05);                //pkn6[4:0]	V7
    NV3030B_SendData(0x06);                //pkn7[4:0]	V3
    //GAMMA---------------------------------/////////////

    //source
    NV3030B_SendCmd(0xE6);
    NV3030B_SendData(0x00);
    NV3030B_SendData(0xff);                //SC_EN_START[7:0] f0

    NV3030B_SendCmd(0xE7);
    NV3030B_SendData(0x01);                //CS_START[3:0] 01
    NV3030B_SendData(0x04);                //scdt_inv_sel cs_vp_en
    NV3030B_SendData(0x03);                //CS1_WIDTH[7:0] 12
    NV3030B_SendData(0x03);                //CS2_WIDTH[7:0] 12
    NV3030B_SendData(0x00);                //PREC_START[7:0] 06
    NV3030B_SendData(0x12);                //PREC_WIDTH[7:0] 12

    NV3030B_SendCmd(0xE8);                //source
    NV3030B_SendData(0x00);            //VCMP_OUT_EN 81-
    NV3030B_SendData(0x70);            //chopper_sel[6:4]
    NV3030B_SendData(0x00);            //gchopper_sel[6:4] 60

    ////gate
    NV3030B_SendCmd(0xEc);
    NV3030B_SendData(0x52);                //52

    NV3030B_SendCmd(0xF1);
    NV3030B_SendData(0x01);                //te_pol tem_extend 00 01 03
    NV3030B_SendData(0x01);
    NV3030B_SendData(0x02);


    NV3030B_SendCmd(0xF6);
    NV3030B_SendData(0x01);
    NV3030B_SendData(0x30);
    NV3030B_SendData(0x00);
    NV3030B_SendData(0x00);                //40

    NV3030B_SendCmd(0xfd);
    NV3030B_SendData(0xfa);
    NV3030B_SendData(0xfc);

    NV3030B_SendCmd(0x3a);
    NV3030B_SendData(0x55);                //SH 0x66

    NV3030B_SendCmd(0x35);
    NV3030B_SendData(0x00);

    //NV3030B_SendCmd(0x36);			//bgr_[3]
    //NV3030B_SendData(0x00);			//c0

    NV3030B_SendCmd(0x21);

    NV3030B_SendCmd(0x11); // exit sleep
    HAL_Delay(120);
    NV3030B_SendCmd(0x29); // display on
    HAL_Delay(10);

    NV3030B_Unselect();

#if FRAME_BUFFER
    NV3030B_ClearFrameBuffer();
#endif

}
//==============================================================================


//==============================================================================
// Процедура управления SPI
//==============================================================================
static void NV3030B_Select(void) {

#ifdef CS_PORT

    //-- если захотим переделать под HAL ------------------
#ifdef NV3030B_SPI_HAL
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
#endif
    //-----------------------------------------------------

    //-- если захотим переделать под CMSIS  ---------------
#ifdef NV3030B_SPI_CMSIS
    CS_GPIO_Port->BSRR = ( CS_Pin << 16 );
#endif
    //-----------------------------------------------------
#endif

}
//==============================================================================


//==============================================================================
// Процедура управления SPI
//==============================================================================
static void NV3030B_Unselect(void) {

#ifdef CS_PORT

    //-- если захотим переделать под HAL ------------------
#ifdef NV3030B_SPI_HAL
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
#endif
    //-----------------------------------------------------

    //-- если захотим переделать под CMSIS  ---------------
#ifdef NV3030B_SPI_CMSIS
    CS_GPIO_Port->BSRR = CS_Pin;
#endif
    //-----------------------------------------------------

#endif

}
//==============================================================================


//==============================================================================
// Процедура отправки данных для инициализации дисплея
//==============================================================================
static void NV3030B_ExecuteCommandList(const uint8_t *addr) {

    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while (numCommands--) {
        uint8_t cmd = *addr++;
        NV3030B_SendCmd(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & DELAY;
        numArgs &= ~DELAY;
        if (numArgs) {
            NV3030B_SendDataMASS((uint8_t *) addr, numArgs);
            addr += numArgs;
        }

        if (ms) {
            ms = *addr++;
            if (ms == 255) ms = 500;
            HAL_Delay(ms);
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура вывода цветного изображения на дисплей
//==============================================================================
void NV3030B_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {

    if ((x >= NV3030B_Width) || (y >= NV3030B_Height)) {
        return;
    }

    if ((x + w - 1) >= NV3030B_Width) {
        return;
    }

    if ((y + h - 1) >= NV3030B_Height) {
        return;
    }

#if FRAME_BUFFER    // если включен буфер кадра
    for( uint16_t i = 0; i < h; i++ ){
        for( uint16_t j = 0; j < w; j++ ){
            buff_frame[( y + i ) * NV3030B_Width + x + j] = *data;
            data++;
        }
    }
#else	//если попиксельный вывод
    NV3030B_SetWindow(x, y, x + w - 1, y + h - 1);

    NV3030B_Select();

    NV3030B_SendDataMASS((uint8_t *) data, sizeof(uint16_t) * w * h);

    NV3030B_Unselect();
#endif
}
//==============================================================================


//==============================================================================
// Процедура аппаратного сброса дисплея (ножкой RESET)
//==============================================================================
void NV3030B_HardReset(void) {

    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

}
//==============================================================================


//==============================================================================
// Процедура отправки команды в дисплей
//==============================================================================
__inline static void NV3030B_SendCmd(uint8_t Cmd) {

    //-- если захотим переделать под HAL ------------------
#ifdef NV3030B_SPI_HAL

    // pin DC LOW
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET);

    HAL_SPI_Transmit(&NV3030B_SPI_HAL, &Cmd, 1, HAL_MAX_DELAY);
    while (HAL_SPI_GetState(&NV3030B_SPI_HAL) != HAL_SPI_STATE_READY) {};

    // pin DC HIGH
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);

#endif
    //-----------------------------------------------------

    //-- если захотим переделать под CMSIS  ---------------------------------------------
#ifdef NV3030B_SPI_CMSIS

    // pin DC LOW
    DC_GPIO_Port->BSRR = ( DC_Pin << 16 );

    //======  FOR F-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        // Ждем, пока не освободится буфер передатчика
        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};

        // заполняем буфер передатчика 1 байт информации--------------
        *((__IO uint8_t *)&NV3030B_SPI_CMSIS->DR) = Cmd;

        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};

        //Ждем, пока SPI освободится от предыдущей передачи
        //while((NV3030B_SPI_CMSIS->SR&SPI_SR_BSY)){};

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

    //================================================================================

/*		//======  FOR H-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;

        // ждем пока SPI будет свободна------------
        //while (!(NV3030B_SPI_CMSIS->SR & SPI_SR_TXP)){};

        // передаем 1 байт информации--------------
        *((__IO uint8_t *)&NV3030B_SPI_CMSIS->TXDR )  = Cmd;

        // Ждать завершения передачи---------------
        while (!( NV3030B_SPI_CMSIS -> SR & SPI_SR_TXC )){};

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

*/		//================================================================================

    // pin DC HIGH
    DC_GPIO_Port->BSRR = DC_Pin;

#endif
    //-----------------------------------------------------------------------------------

}
//==============================================================================


//==============================================================================
// Процедура отправки данных (параметров) в дисплей 1 BYTE
//==============================================================================
__inline static void NV3030B_SendData(uint8_t Data) {

    //-- если захотим переделать под HAL ------------------
#ifdef NV3030B_SPI_HAL

    HAL_SPI_Transmit(&NV3030B_SPI_HAL, &Data, 1, HAL_MAX_DELAY);
    while (HAL_SPI_GetState(&NV3030B_SPI_HAL) != HAL_SPI_STATE_READY) {};

#endif
    //-----------------------------------------------------


    //-- если захотим переделать под CMSIS  ---------------------------------------------
#ifdef NV3030B_SPI_CMSIS

    //======  FOR F-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        // Ждем, пока не освободится буфер передатчика
        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};

        // передаем 1 байт информации--------------
        *((__IO uint8_t *)&NV3030B_SPI_CMSIS->DR) = Data;

        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};

        // Ждем, пока не освободится буфер передатчика
        //while((NV3030B_SPI_CMSIS->SR&SPI_SR_BSY)){};

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

    //================================================================================

/*		//======  FOR H-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;

        // ждем пока SPI будет свободна------------
        //while (!(NV3030B_SPI_CMSIS->SR & SPI_SR_TXP)){};

        // передаем 1 байт информации--------------
        *((__IO uint8_t *)&NV3030B_SPI_CMSIS->TXDR )  = Data;

        // Ждать завершения передачи---------------
        while (!( NV3030B_SPI_CMSIS -> SR & SPI_SR_TXC )){};

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

*/		//================================================================================

#endif
    //-----------------------------------------------------------------------------------

}
//==============================================================================


//==============================================================================
// Процедура отправки данных (параметров) в дисплей MASS
//==============================================================================
__inline static void NV3030B_SendDataMASS(uint8_t *buff, size_t buff_size) {

    //-- если захотим переделать под HAL ------------------
#ifdef NV3030B_SPI_HAL

    if (buff_size <= 0xFFFF) {
        HAL_SPI_Transmit(&NV3030B_SPI_HAL, buff, buff_size, HAL_MAX_DELAY);
    } else {
        while (buff_size > 0xFFFF) {
            HAL_SPI_Transmit(&NV3030B_SPI_HAL, buff, 0xFFFF, HAL_MAX_DELAY);
            buff_size -= 0xFFFF;
            buff += 0xFFFF;
        }
        HAL_SPI_Transmit(&NV3030B_SPI_HAL, buff, buff_size, HAL_MAX_DELAY);
    }

    while (HAL_SPI_GetState(&NV3030B_SPI_HAL) != HAL_SPI_STATE_READY) {};

#endif
    //-----------------------------------------------------


    //-- если захотим переделать под CMSIS  ---------------------------------------------
#ifdef NV3030B_SPI_CMSIS

    //======  FOR F-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        while( buff_size ){

        // Ждем, пока не освободится буфер передатчика
        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};

            // передаем 1 байт информации--------------
            *((__IO uint8_t *)&NV3030B_SPI_CMSIS->DR) = *buff++;

            buff_size--;
        }

        // TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
        while( (NV3030B_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};

        // Ждем, пока не освободится буфер передатчика
        // while((NV3030B_SPI_CMSIS->SR&SPI_SR_BSY)){};

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

    //================================================================================

/*		//======  FOR H-SERIES ===========================================================

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
        // Enable SPI
        if((NV3030B_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
            // If disabled, I enable it
            SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
        }

        SET_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// NV3030B_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;

        // ждем пока SPI будет свободна------------
        //while (!(NV3030B_SPI_CMSIS->SR & SPI_SR_TXP)){};

        while( buff_size ){

            // передаем 1 байт информации--------------
            *((__IO uint8_t *)&NV3030B_SPI_CMSIS->TXDR )  = *buff++;

            // Ждать завершения передачи---------------
            while (!( NV3030B_SPI_CMSIS -> SR & SPI_SR_TXC )){};

            buff_size--;

        }

        // Disable SPI
        //CLEAR_BIT(NV3030B_SPI_CMSIS->CR1, SPI_CR1_SPE);

*/		//================================================================================

#endif
    //-----------------------------------------------------------------------------------

}
//==============================================================================


//==============================================================================
// Процедура включения режима сна
//==============================================================================
void NV3030B_SleepModeEnter(void) {

    NV3030B_Select();

    NV3030B_SendCmd(NV3030B_SLPIN);

    NV3030B_Unselect();

    HAL_Delay(250);
}
//==============================================================================


//==============================================================================
// Процедура отключения режима сна
//==============================================================================
void NV3030B_SleepModeExit(void) {

    NV3030B_Select();

    NV3030B_SendCmd(NV3030B_SLPOUT);

    NV3030B_Unselect();

    HAL_Delay(250);
}
//==============================================================================


//==============================================================================
// Процедура включения/отключения режима частичного заполнения экрана
//==============================================================================
void NV3030B_InversionMode(uint8_t Mode) {

    NV3030B_Select();

    if (Mode) {
        NV3030B_SendCmd(NV3030B_INVON);
    } else {
        NV3030B_SendCmd(NV3030B_INVOFF);
    }

    NV3030B_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура закрашивает экран цветом color
//==============================================================================
void NV3030B_FillScreen(uint16_t color) {

    NV3030B_FillRect(0, 0, NV3030B_Width, NV3030B_Height, color);
}
//==============================================================================


//==============================================================================
// Процедура очистки экрана - закрашивает экран цветом черный
//==============================================================================
void NV3030B_Clear(void) {

    NV3030B_FillRect(0, 0, NV3030B_Width, NV3030B_Height, 0);
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольника цветом color
//==============================================================================
void NV3030B_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

    if ((x >= NV3030B_Width) || (y >= NV3030B_Height)) {
        return;
    }

    if ((x + w) > NV3030B_Width) {
        w = NV3030B_Width - x;
    }

    if ((y + h) > NV3030B_Height) {
        h = NV3030B_Height - y;
    }

#if FRAME_BUFFER    // если включен буфер кадра
    if( x >=0 && y >=0 ){
        for( uint16_t i = 0; i < h; i++ ){
            for( uint16_t j = 0; j < w; j++ ){
                    buff_frame[( y + i ) * NV3030B_Width + x + j] = ((color & 0xFF)<<8) | (color >> 8 );
            }
        }
    }
#else	//если попиксельный вывод
    NV3030B_SetWindow(x, y, x + w - 1, y + h - 1);

    NV3030B_RamWrite(&color, (h * w));
#endif
}
//==============================================================================


//==============================================================================
// Процедура установка границ экрана для заполнения
//==============================================================================
static void NV3030B_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

    NV3030B_Select();

    NV3030B_ColumnSet(x0, x1);
    NV3030B_RowSet(y0, y1);

    // write to RAM
    NV3030B_SendCmd(NV3030B_RAMWR);

    NV3030B_Unselect();

}
//==============================================================================


//==============================================================================
// Процедура записи данных в дисплей
//==============================================================================
static void NV3030B_RamWrite(uint16_t *pBuff, uint32_t Len) {

    NV3030B_Select();

    uint8_t buff[2];
    buff[0] = *pBuff >> 8;
    buff[1] = *pBuff & 0xFF;

    while (Len--) {
        NV3030B_SendDataMASS(buff, 2);
    }

    NV3030B_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура установки начального и конечного адресов колонок
//==============================================================================
static void NV3030B_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd) {

    if (ColumnStart > ColumnEnd) {
        return;
    }

    if (ColumnEnd > NV3030B_Width) {
        return;
    }

    ColumnStart += NV3030B_X_Start;
    ColumnEnd += NV3030B_X_Start;

    NV3030B_SendCmd(NV3030B_CASET);
    NV3030B_SendData(ColumnStart >> 8);
    NV3030B_SendData(ColumnStart & 0xFF);
    NV3030B_SendData(ColumnEnd >> 8);
    NV3030B_SendData(ColumnEnd & 0xFF);

}
//==============================================================================


//==============================================================================
// Процедура установки начального и конечного адресов строк
//==============================================================================
static void NV3030B_RowSet(uint16_t RowStart, uint16_t RowEnd) {

    if (RowStart > RowEnd) {
        return;
    }

    if (RowEnd > NV3030B_Height) {
        return;
    }

    RowStart += NV3030B_Y_Start;
    RowEnd += NV3030B_Y_Start;

    NV3030B_SendCmd(NV3030B_RASET);
    NV3030B_SendData(RowStart >> 8);
    NV3030B_SendData(RowStart & 0xFF);
    NV3030B_SendData(RowEnd >> 8);
    NV3030B_SendData(RowEnd & 0xFF);

}
//==============================================================================


//==============================================================================
// Процедура управления подсветкой (ШИМ)
//==============================================================================
//void NV3030B_SetBL(uint8_t Value) {

//  if (Value > 100)
//    Value = 100;

//	tmr2_PWM_set(ST77xx_PWM_TMR2_Chan, Value);

//}
//==============================================================================


//==============================================================================
// Процедура включения/отключения питания дисплея
//==============================================================================
void NV3030B_DisplayPower(uint8_t On) {

    NV3030B_Select();

    if (On) {
        NV3030B_SendCmd(NV3030B_DISPON);
    } else {
        NV3030B_SendCmd(NV3030B_DISPOFF);
    }

    NV3030B_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольника ( пустотелый )
//==============================================================================
void NV3030B_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    NV3030B_DrawLine(x1, y1, x1, y2, color);
    NV3030B_DrawLine(x2, y1, x2, y2, color);
    NV3030B_DrawLine(x1, y1, x2, y1, color);
    NV3030B_DrawLine(x1, y2, x2, y2, color);

}
//==============================================================================


//==============================================================================
// Процедура вспомогательная для --- Процедура рисования прямоугольника ( заполненый )
//==============================================================================
static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2) {

    int16_t TempValue = *pValue1;
    *pValue1 = *pValue2;
    *pValue2 = TempValue;
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольника ( заполненый )
//==============================================================================
void NV3030B_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor) {

    if (x1 > x2) {
        SwapInt16Values(&x1, &x2);
    }

    if (y1 > y2) {
        SwapInt16Values(&y1, &y2);
    }

    NV3030B_FillRect(x1, y1, x2 - x1, y2 - y1, fillcolor);
}
//==============================================================================


//==============================================================================
// Процедура вспомогательная для --- Процедура рисования линии
//==============================================================================
static void NV3030B_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    const int16_t deltaX = abs(x2 - x1);
    const int16_t deltaY = abs(y2 - y1);
    const int16_t signX = x1 < x2 ? 1 : -1;
    const int16_t signY = y1 < y2 ? 1 : -1;

    int16_t error = deltaX - deltaY;

    NV3030B_DrawPixel(x2, y2, color);

    while (x1 != x2 || y1 != y2) {

        NV3030B_DrawPixel(x1, y1, color);
        const int16_t error2 = error * 2;

        if (error2 > -deltaY) {

            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {

            error += deltaX;
            y1 += signY;
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования линии
//==============================================================================
void NV3030B_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    if (x1 == x2) {

        if (y1 > y2) {
            NV3030B_FillRect(x1, y2, 1, y1 - y2 + 1, color);
        } else {
            NV3030B_FillRect(x1, y1, 1, y2 - y1 + 1, color);
        }

        return;
    }

    if (y1 == y2) {

        if (x1 > x2) {
            NV3030B_FillRect(x2, y1, x1 - x2 + 1, 1, color);
        } else {
            NV3030B_FillRect(x1, y1, x2 - x1 + 1, 1, color);
        }

        return;
    }

    NV3030B_DrawLine_Slow(x1, y1, x2, y2, color);
}
//==============================================================================


//==============================================================================
// Процедура рисования линии с указаным углом и длиной
//==============================================================================
void NV3030B_DrawLineWithAngle(int16_t x, int16_t y, uint16_t length, double angle_degrees, uint16_t color) {
    // Преобразование угла в радианы
    double angle_radians = (360.0 - angle_degrees) * PI / 180.0;

    // Вычисление конечных координат
    int16_t x2 = x + length * cos(angle_radians) + 0.5;
    int16_t y2 = y + length * sin(angle_radians) + 0.5;

    // Используем существующую функцию для рисования линии
    NV3030B_DrawLine(x, y, x2, y2, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования треугольника ( пустотелый )
//==============================================================================
void
NV3030B_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
    /* Draw lines */
    NV3030B_DrawLine(x1, y1, x2, y2, color);
    NV3030B_DrawLine(x2, y2, x3, y3, color);
    NV3030B_DrawLine(x3, y3, x1, y1, color);
}
//==============================================================================


//==============================================================================
// Процедура рисования треугольника ( заполненый )
//==============================================================================
void NV3030B_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                                uint16_t color) {

    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
            yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
            curpixel = 0;

    deltax = abs(x2 - x1);
    deltay = abs(y2 - y1);
    x = x1;
    y = y1;

    if (x2 >= x1) {
        xinc1 = 1;
        xinc2 = 1;
    } else {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1) {
        yinc1 = 1;
        yinc2 = 1;
    } else {
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay) {
        xinc1 = 0;
        yinc2 = 0;
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax;
    } else {
        xinc2 = 0;
        yinc1 = 0;
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay;
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++) {
        NV3030B_DrawLine(x, y, x3, y3, color);

        num += numadd;
        if (num >= den) {
            num -= den;
            x += xinc1;
            y += yinc1;
        }
        x += xinc2;
        y += yinc2;
    }
}
//==============================================================================


//==============================================================================
// Процедура окрашивает 1 пиксель дисплея
//==============================================================================
void NV3030B_DrawPixel(int16_t x, int16_t y, uint16_t color) {

    if ((x < 0) || (x >= NV3030B_Width) || (y < 0) || (y >= NV3030B_Height)) {
        return;
    }

#if FRAME_BUFFER    // если включен буфер кадра
    buff_frame[y * NV3030B_Width + x] = ((color & 0xFF)<<8) | (color >> 8 );
#else	//если попиксельный вывод
    NV3030B_SetWindow(x, y, x, y);
    NV3030B_RamWrite(&color, 1);
#endif
}
//==============================================================================


//==============================================================================
// Процедура рисования круг ( заполненый )
//==============================================================================
void NV3030B_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor) {

    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;

    while (y >= 0) {

        NV3030B_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
        NV3030B_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
        error = 2 * (delta + y) - 1;

        if (delta < 0 && error <= 0) {

            ++x;
            delta += 2 * x + 1;
            continue;
        }

        error = 2 * (delta - x) - 1;

        if (delta > 0 && error > 0) {

            --y;
            delta += 1 - 2 * y;
            continue;
        }

        ++x;
        delta += 2 * (x - y);
        --y;
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования круг ( пустотелый )
//==============================================================================
void NV3030B_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color) {

    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;

    while (y >= 0) {

        NV3030B_DrawPixel(x0 + x, y0 + y, color);
        NV3030B_DrawPixel(x0 + x, y0 - y, color);
        NV3030B_DrawPixel(x0 - x, y0 + y, color);
        NV3030B_DrawPixel(x0 - x, y0 - y, color);
        error = 2 * (delta + y) - 1;

        if (delta < 0 && error <= 0) {

            ++x;
            delta += 2 * x + 1;
            continue;
        }

        error = 2 * (delta - x) - 1;

        if (delta > 0 && error > 0) {

            --y;
            delta += 1 - 2 * y;
            continue;
        }

        ++x;
        delta += 2 * (x - y);
        --y;
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс
//==============================================================================
void NV3030B_DrawEllipse(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, uint16_t color) {
    int x, y;
    for (float angle = 0; angle <= 360; angle += 0.1) {
        x = x0 + radiusX * cos(angle * PI / 180);
        y = y0 + radiusY * sin(angle * PI / 180);
        NV3030B_DrawPixel(x, y, color);
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс под указаным углом наклона
//==============================================================================
void NV3030B_DrawEllipseWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees,
                                  uint16_t color) {
    float cosAngle = cos((360.0 - angle_degrees) * PI / 180);
    float sinAngle = sin((360.0 - angle_degrees) * PI / 180);

    for (int16_t t = 0; t <= 360; t++) {
        float radians = t * PI / 180.0;
        int16_t x = radiusX * cos(radians);
        int16_t y = radiusY * sin(radians);

        int16_t xTransformed = x0 + cosAngle * x - sinAngle * y;
        int16_t yTransformed = y0 + sinAngle * x + cosAngle * y;

        NV3030B_DrawPixel(xTransformed, yTransformed, color);
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс закрашенный
//==============================================================================
void NV3030B_DrawEllipseFilled(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, uint16_t color) {
    int x, y;

    for (y = -radiusY; y <= radiusY; y++) {
        for (x = -radiusX; x <= radiusX; x++) {
            if ((x * x * radiusY * radiusY + y * y * radiusX * radiusX) <= (radiusX * radiusX * radiusY * radiusY)) {
                NV3030B_DrawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}
//==============================================================================


//==============================================================================
// рисуем элипс закрашенный под указаным углом наклона
//==============================================================================
void NV3030B_DrawEllipseFilledWithAngle(int16_t x0, int16_t y0, int16_t radiusX, int16_t radiusY, float angle_degrees,
                                        uint16_t color) {
    float cosAngle = cos((360.0 - angle_degrees) * PI / 180.0);
    float sinAngle = sin((360.0 - angle_degrees) * PI / 180.0);

    for (int16_t y = -radiusY; y <= radiusY; y++) {
        for (int16_t x = -radiusX; x <= radiusX; x++) {
            float xTransformed = cosAngle * x - sinAngle * y;
            float yTransformed = sinAngle * x + cosAngle * y;

            if ((x * x * radiusY * radiusY + y * y * radiusX * radiusX) <= (radiusX * radiusX * radiusY * radiusY)) {
                NV3030B_DrawPixel(x0 + xTransformed, y0 + yTransformed, color);
            }
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования символа ( 1 буква или знак )
//==============================================================================
void
NV3030B_DrawChar(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t *Font,
                 uint8_t multiplier, unsigned char ch) {

    uint32_t i, b, j;

    uint32_t X = x, Y = y;

    uint8_t xx, yy;

    if (multiplier < 1) {
        multiplier = 1;
    }

    /* Check available space in LCD */
    if (NV3030B_Width >= (x + Font->FontWidth) || NV3030B_Height >= (y + Font->FontHeight)) {


        /* Go through font */
        for (i = 0; i < Font->FontHeight; i++) {

            if (ch < 127) {
                b = Font->data[(ch - 32) * Font->FontHeight + i];
            } else if ((uint8_t) ch > 191) {
                // +96 это так как латинские символы и знаки в шрифтах занимают 96 позиций
                // и если в шрифте который содержит сперва латиницу и спец символы и потом
                // только кирилицу то нужно добавлять 95 если шрифт
                // содержит только кирилицу то +96 не нужно
                b = Font->data[((ch - 192) + 96) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 168) {    // 168 символ по ASCII - Ё
                // 160 эллемент ( символ Ё )
                b = Font->data[(160) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 184) {    // 184 символ по ASCII - ё
                // 161 эллемент  ( символ ё )
                b = Font->data[(161) * Font->FontHeight + i];
            }
                //-------------------------------------------------------------------

                //----  Украинская раскладка ----------------------------------------------------
            else if ((uint8_t) ch == 170) {    // 168 символ по ASCII - Є
                // 162 эллемент ( символ Є )
                b = Font->data[(162) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 175) {    // 184 символ по ASCII - Ї
                // 163 эллемент  ( символ Ї )
                b = Font->data[(163) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 178) {    // 168 символ по ASCII - І
                // 164 эллемент ( символ І )
                b = Font->data[(164) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 179) {    // 184 символ по ASCII - і
                // 165 эллемент  ( символ і )
                b = Font->data[(165) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 186) {    // 184 символ по ASCII - є
                // 166 эллемент  ( символ є )
                b = Font->data[(166) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 191) {    // 168 символ по ASCII - ї
                // 167 эллемент ( символ ї )
                b = Font->data[(167) * Font->FontHeight + i];
            }
            //-----------------------------------------------------------------------------

            for (j = 0; j < Font->FontWidth; j++) {

                if ((b << j) & 0x8000) {

                    for (yy = 0; yy < multiplier; yy++) {
                        for (xx = 0; xx < multiplier; xx++) {
                            NV3030B_DrawPixel(X + xx, Y + yy, TextColor);
                        }
                    }

                } else if (TransparentBg) {

                    for (yy = 0; yy < multiplier; yy++) {
                        for (xx = 0; xx < multiplier; xx++) {
                            NV3030B_DrawPixel(X + xx, Y + yy, BgColor);
                        }
                    }

                }
                X = X + multiplier;
            }
            X = x;
            Y = Y + multiplier;
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования строки
//==============================================================================
void NV3030B_print(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t *Font,
                   uint8_t multiplier, char *str) {

    if (multiplier < 1) {
        multiplier = 1;
    }

    unsigned char buff_char;

    uint16_t len = strlen(str);

    while (len--) {

        //---------------------------------------------------------------------
        // проверка на кириллицу UTF-8, если латиница то пропускаем if
        // Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
        // проверяем первый байт из двух ( так как UTF-8 ето два байта )
        // если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
        if ((uint8_t) * str >= 0xC0) {    // код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251

            // проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
            switch ((uint8_t) * str) {
                case 0xD0: {
                    // увеличиваем массив так как нам нужен второй байт
                    str++;
                    // проверяем второй байт там сам символ
                    if ((uint8_t) * str >= 0x90 && (uint8_t) * str <= 0xBF) {
                        buff_char = (*str) + 0x30;
                    }    // байт символов А...Я а...п  делаем здвиг на +48
                    else if ((uint8_t) * str == 0x81) {
                        buff_char = 0xA8;
                        break;
                    }        // байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x84) {
                        buff_char = 0xAA;
                        break;
                    }        // байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x86) {
                        buff_char = 0xB2;
                        break;
                    }        // байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x87) {
                        buff_char = 0xAF;
                        break;
                    }        // байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    break;
                }
                case 0xD1: {
                    // увеличиваем массив так как нам нужен второй байт
                    str++;
                    // проверяем второй байт там сам символ
                    if ((uint8_t) * str >= 0x80 && (uint8_t) * str <= 0x8F) {
                        buff_char = (*str) + 0x70;
                    }    // байт символов п...я	елаем здвиг на +112
                    else if ((uint8_t) * str == 0x91) {
                        buff_char = 0xB8;
                        break;
                    }        // байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x94) {
                        buff_char = 0xBA;
                        break;
                    }        // байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x96) {
                        buff_char = 0xB3;
                        break;
                    }        // байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x97) {
                        buff_char = 0xBF;
                        break;
                    }        // байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    break;
                }
            }
            //------------------------------------------------------------------------------------------------
            // уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
            len--;

            NV3030B_DrawChar(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, buff_char);
        }
            //---------------------------------------------------------------------
        else {
            NV3030B_DrawChar(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, *str);
        }

        x = x + (Font->FontWidth * multiplier);
        /* Increase string pointer */
        str++;
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования символа с указаным углом ( 1 буква или знак )
//==============================================================================
void NV3030B_DrawCharWithAngle(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg,
                               FontDef_t *Font, uint8_t multiplier, double angle_degrees, unsigned char ch) {

    uint32_t i, b, j;

    uint32_t X = x, Y = y;

    uint8_t xx, yy;

    // Преобразуем угол в радианы
    double radians = (360.0 - angle_degrees) * PI / 180.0;

    // Вычисляем матрицу поворота
    double cosTheta = cos(radians);
    double sinTheta = sin(radians);

    // Переменные для преобразованных координат
    double newX, newY;

    if (multiplier < 1) {
        multiplier = 1;
    }

    /* Check available space in LCD */
    if (NV3030B_Width >= (x + Font->FontWidth) || NV3030B_Height >= (y + Font->FontHeight)) {

        /* Go through font */
        for (i = 0; i < Font->FontHeight; i++) {

            if (ch < 127) {
                b = Font->data[(ch - 32) * Font->FontHeight + i];
            } else if ((uint8_t) ch > 191) {
                // +96 это так как латинские символы и знаки в шрифтах занимают 96 позиций
                // и если в шрифте который содержит сперва латиницу и спец символы и потом
                // только кирилицу то нужно добавлять 95 если шрифт
                // содержит только кирилицу то +96 не нужно
                b = Font->data[((ch - 192) + 96) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 168) {    // 168 символ по ASCII - Ё
                // 160 эллемент ( символ Ё )
                b = Font->data[(160) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 184) {    // 184 символ по ASCII - ё
                // 161 эллемент  ( символ ё )
                b = Font->data[(161) * Font->FontHeight + i];
            }
                //-------------------------------------------------------------------

                //----  Украинская раскладка ----------------------------------------------------
            else if ((uint8_t) ch == 170) {    // 168 символ по ASCII - Є
                // 162 эллемент ( символ Є )
                b = Font->data[(162) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 175) {    // 184 символ по ASCII - Ї
                // 163 эллемент  ( символ Ї )
                b = Font->data[(163) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 178) {    // 168 символ по ASCII - І
                // 164 эллемент ( символ І )
                b = Font->data[(164) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 179) {    // 184 символ по ASCII - і
                // 165 эллемент  ( символ і )
                b = Font->data[(165) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 186) {    // 184 символ по ASCII - є
                // 166 эллемент  ( символ є )
                b = Font->data[(166) * Font->FontHeight + i];
            } else if ((uint8_t) ch == 191) {    // 168 символ по ASCII - ї
                // 167 эллемент ( символ ї )
                b = Font->data[(167) * Font->FontHeight + i];
            }
            //-----------------------------------------------------------------------------

            for (j = 0; j < Font->FontWidth; j++) {
                if ((b << j) & 0x8000) {
                    // Применяем поворот к координатам
                    newX = cosTheta * (X - x) - sinTheta * (Y - y) + x;
                    newY = sinTheta * (X - x) + cosTheta * (Y - y) + y;

                    for (yy = 0; yy < multiplier; yy++) {
                        for (xx = 0; xx < multiplier; xx++) {
                            NV3030B_DrawPixel(newX + xx, newY + yy, TextColor);
                        }
                    }
                } else if (TransparentBg) {
                    // Аналогично для фона
                    newX = cosTheta * (X - x) - sinTheta * (Y - y) + x + 0.5;
                    newY = sinTheta * (X - x) + cosTheta * (Y - y) + y + 0.5;

                    for (yy = 0; yy < multiplier; yy++) {
                        for (xx = 0; xx < multiplier; xx++) {
                            NV3030B_DrawPixel(newX + xx, newY + yy, BgColor);
                        }
                    }
                }
                X = X + multiplier;
            }
            X = x;
            Y = Y + multiplier;
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования строки с указаным углом
//==============================================================================
void NV3030B_printWithAngle(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg,
                            FontDef_t *Font, uint8_t multiplier, double angle_degrees, char *str) {

    if (multiplier < 1) {
        multiplier = 1;
    }

    unsigned char buff_char;

    uint16_t len = strlen(str);

    while (len--) {

        //---------------------------------------------------------------------
        // проверка на кириллицу UTF-8, если латиница то пропускаем if
        // Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
        // проверяем первый байт из двух ( так как UTF-8 ето два байта )
        // если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
        if ((uint8_t) * str >= 0xC0) {    // код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251

            // проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
            switch ((uint8_t) * str) {
                case 0xD0: {
                    // увеличиваем массив так как нам нужен второй байт
                    str++;
                    // проверяем второй байт там сам символ
                    if ((uint8_t) * str >= 0x90 && (uint8_t) * str <= 0xBF) {
                        buff_char = (*str) + 0x30;
                    }    // байт символов А...Я а...п  делаем здвиг на +48
                    else if ((uint8_t) * str == 0x81) {
                        buff_char = 0xA8;
                        break;
                    }        // байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x84) {
                        buff_char = 0xAA;
                        break;
                    }        // байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x86) {
                        buff_char = 0xB2;
                        break;
                    }        // байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x87) {
                        buff_char = 0xAF;
                        break;
                    }        // байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    break;
                }
                case 0xD1: {
                    // увеличиваем массив так как нам нужен второй байт
                    str++;
                    // проверяем второй байт там сам символ
                    if ((uint8_t) * str >= 0x80 && (uint8_t) * str <= 0x8F) {
                        buff_char = (*str) + 0x70;
                    }    // байт символов п...я	елаем здвиг на +112
                    else if ((uint8_t) * str == 0x91) {
                        buff_char = 0xB8;
                        break;
                    }        // байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x94) {
                        buff_char = 0xBA;
                        break;
                    }        // байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x96) {
                        buff_char = 0xB3;
                        break;
                    }        // байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    else if ((uint8_t) * str == 0x97) {
                        buff_char = 0xBF;
                        break;
                    }        // байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
                    break;
                }
            }
            //------------------------------------------------------------------------------------------------
            // уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
            len--;

            NV3030B_DrawCharWithAngle(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, angle_degrees,
                                      buff_char);
        }
            //---------------------------------------------------------------------
        else {
            NV3030B_DrawCharWithAngle(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, angle_degrees, *str);
        }
        // Смещаем начальные координаты с каждым символом с учетом угла
        x += (Font->FontWidth * multiplier * cos((360.0 - angle_degrees) * PI / 180.0) + 0.5);
        y += (Font->FontWidth * multiplier * sin((360.0 - angle_degrees) * PI / 180.0) + 0.5);

        /* Increase string pointer */
        str++;
    }
}
//==============================================================================


//==============================================================================
// Процедура ротации ( положение ) дисплея
//==============================================================================
// па умолчанию 1 режим ( всего 1, 2, 3, 4 )
void NV3030B_rotation(uint8_t rotation) {

    NV3030B_Select();

    NV3030B_SendCmd(NV3030B_MADCTL);

    // длайвер расчитан на экран 320 х 240 (  максимальный размер )
    // для подгона под любой другой нужно отнимать разницу пикселей

    switch (rotation) {

        case 1:
            //== 1.13" 135 x 240 NV3030B =================================================
#ifdef NV3030B_IS_135X240
            NV3030B_SendData(NV3030B_MADCTL_RGB);
            NV3030B_Width = 135;
            NV3030B_Height = 240;
            NV3030B_X_Start = 52;
            NV3030B_Y_Start = 40;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.3" 240 x 240 NV3030B =================================================
#ifdef NV3030B_IS_240X240
            NV3030B_SendData(NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 240;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.47" 172 x 320 NV3030B =================================================
#ifdef NV3030B_IS_172X320
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 320;
            NV3030B_Height = 172;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 34;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.69" 240 x 280 NV3030B =================================================
#ifdef NV3030B_IS_240X280
            NV3030B_SendData(NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 280;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 20;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 2" 240 x 320 NV3030B =================================================
#ifdef NV3030B_IS_240X320
            NV3030B_SendData(NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 320;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================
            break;

        case 2:
            //== 1.13" 135 x 240 NV3030B =================================================
#ifdef NV3030B_IS_135X240
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 135;
            NV3030B_X_Start = 40;
            NV3030B_Y_Start = 53;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.3" 240 x 240 NV3030B =================================================
#ifdef NV3030B_IS_240X240
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 240;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.47" 172 x 320 NV3030B =================================================
#ifdef NV3030B_IS_172X320
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MY | NV3030B_MADCTL_RGB);
            NV3030B_Width = 172;
            NV3030B_Height = 320;
            NV3030B_X_Start = 34;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.69" 240 x 280 NV3030B =================================================
#ifdef NV3030B_IS_240X280
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 280;
            NV3030B_Height = 240;
            NV3030B_X_Start = 20;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 2" 240 x 320 NV3030B =================================================
#ifdef NV3030B_IS_240X320
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 320;
            NV3030B_Height = 240;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================
            break;

        case 3:
            //== 1.13" 135 x 240 NV3030B =================================================
#ifdef NV3030B_IS_135X240
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MY | NV3030B_MADCTL_RGB);
            NV3030B_Width = 135;
            NV3030B_Height = 240;
            NV3030B_X_Start = 53;
            NV3030B_Y_Start = 40;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.3" 240 x 240 NV3030B =================================================
#ifdef NV3030B_IS_240X240
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MY | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 240;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 80;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.47" 172 x 320 NV3030B =================================================
#ifdef NV3030B_IS_172X320
            NV3030B_SendData(NV3030B_MADCTL_MY | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 320;
            NV3030B_Height = 172;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 34;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.69" 240 x 280 NV3030B =================================================
#ifdef NV3030B_IS_240X280
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MY | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 280;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 20;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 2" 240 x 320 NV3030B =================================================
#ifdef NV3030B_IS_240X320
            NV3030B_SendData(NV3030B_MADCTL_MX | NV3030B_MADCTL_MY | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 320;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            break;

        case 4:
            //== 1.13" 135 x 240 NV3030B =================================================
#ifdef NV3030B_IS_135X240
            NV3030B_SendData(NV3030B_MADCTL_MY | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 135;
            NV3030B_X_Start = 40;
            NV3030B_Y_Start = 52;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.3" 240 x 240 NV3030B =================================================
#ifdef NV3030B_IS_240X240
            NV3030B_SendData(NV3030B_MADCTL_MY | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 240;
            NV3030B_Height = 240;
            NV3030B_X_Start = 80;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.47" 172 x 320 NV3030B =================================================
#ifdef NV3030B_IS_172X320
            NV3030B_SendData(NV3030B_MADCTL_RGB);
            NV3030B_Width = 172;
            NV3030B_Height = 320;
            NV3030B_X_Start = 34;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 1.69" 240 x 280 NV3030B =================================================
#ifdef NV3030B_IS_240X280
            NV3030B_SendData(NV3030B_MADCTL_MY | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 280;
            NV3030B_Height = 240;
            NV3030B_X_Start = 20;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================

            //== 2" 240 x 320 NV3030B =================================================
#ifdef NV3030B_IS_240X320
            NV3030B_SendData(NV3030B_MADCTL_MY | NV3030B_MADCTL_MV | NV3030B_MADCTL_RGB);
            NV3030B_Width = 320;
            NV3030B_Height = 240;
            NV3030B_X_Start = 0;
            NV3030B_Y_Start = 0;
            NV3030B_FillScreen(0);
#endif
            //==========================================================================
            break;

        default:
            break;
    }

    NV3030B_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура рисования иконки монохромной
//==============================================================================
void NV3030B_DrawBitmap(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color) {

    int16_t byteWidth = (w + 7) / 8;    // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for (int16_t j = 0; j < h; j++, y++) {

        for (int16_t i = 0; i < w; i++) {

            if (i & 7) {
                byte <<= 1;
            } else {
                byte = (*(const unsigned char *) (&bitmap[j * byteWidth + i / 8]));
            }

            if (byte & 0x80) {
                NV3030B_DrawPixel(x + i, y, color);
            }
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования иконки монохромной с указаным углом
//==============================================================================
void
NV3030B_DrawBitmapWithAngle(int16_t x, int16_t y, const unsigned char *bitmap, int16_t w, int16_t h, uint16_t color,
                            double angle_degrees) {
    // Преобразование угла в радианы
    double angle_radians = (360.0 - angle_degrees) * PI / 180.0;

    // Вычисление матрицы поворота
    double cosTheta = cos(angle_radians);
    double sinTheta = sin(angle_radians);

    // Ширина и высота повернутого изображения
    int16_t rotatedW = round(fabs(w * cosTheta) + fabs(h * sinTheta));
    int16_t rotatedH = round(fabs(h * cosTheta) + fabs(w * sinTheta));

    // Вычисление центральных координат повернутого изображения
    int16_t centerX = x + w / 2;
    int16_t centerY = y + h / 2;

    // Проходим по каждому пикселю изображения и рисуем его повернутым
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Вычисление смещения от центра
            int16_t offsetX = i - w / 2;
            int16_t offsetY = j - h / 2;

            // Применение матрицы поворота
            int16_t rotatedX = round(centerX + offsetX * cosTheta - offsetY * sinTheta);
            int16_t rotatedY = round(centerY + offsetX * sinTheta + offsetY * cosTheta);

            // Проверка находится ли пиксель в пределах экрана
            if (rotatedX >= 0 && rotatedX < NV3030B_Width && rotatedY >= 0 && rotatedY < NV3030B_Height) {
                // Получение цвета пикселя из исходного изображения
                uint8_t byteWidth = (w + 7) / 8;
                uint8_t byte = (*(const unsigned char *) (&bitmap[j * byteWidth + i / 8]));
                if (byte & (0x80 >> (i & 7))) {
                    // Рисование пикселя на экране
                    NV3030B_DrawPixel(rotatedX, rotatedY, color);
                }
            }
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( заполненый )
//==============================================================================
void
NV3030B_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color) {

    int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
    if (cornerRadius > max_radius) {
        cornerRadius = max_radius;
    }

    NV3030B_DrawRectangleFilled(x + cornerRadius, y, x + cornerRadius + width - 2 * cornerRadius, y + height, color);
    // draw four corners
    NV3030B_DrawFillCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 1,
                                 height - 2 * cornerRadius - 1, color);
    NV3030B_DrawFillCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 2, height - 2 * cornerRadius - 1,
                                 color);
}
//==============================================================================

//==============================================================================
// Процедура рисования половины окружности ( правая или левая ) ( заполненый )
//==============================================================================
void NV3030B_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++; // Avoid some +1's in the loop

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (x < (y + 1)) {
            if (corners & 1) {
                NV3030B_DrawLine(x0 + x, y0 - y, x0 + x, y0 - y - 1 + 2 * y + delta, color);
            }
            if (corners & 2) {
                NV3030B_DrawLine(x0 - x, y0 - y, x0 - x, y0 - y - 1 + 2 * y + delta, color);
            }
        }
        if (y != py) {
            if (corners & 1) {
                NV3030B_DrawLine(x0 + py, y0 - px, x0 + py, y0 - px - 1 + 2 * px + delta, color);
            }
            if (corners & 2) {
                NV3030B_DrawLine(x0 - py, y0 - px, x0 - py, y0 - px - 1 + 2 * px + delta, color);
            }
            py = y;
        }
        px = x;
    }
}
//==============================================================================																		

//==============================================================================
// Процедура рисования четверти окружности (закругление, дуга) ( ширина 1 пиксель)
//==============================================================================
void NV3030B_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, uint16_t color) {
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    while (x <= y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        if (quadrantMask & 0x4) {
            NV3030B_DrawPixel(x0 + x, y0 + y, color);
            NV3030B_DrawPixel(x0 + y, y0 + x, color);;
        }
        if (quadrantMask & 0x2) {
            NV3030B_DrawPixel(x0 + x, y0 - y, color);
            NV3030B_DrawPixel(x0 + y, y0 - x, color);
        }
        if (quadrantMask & 0x8) {
            NV3030B_DrawPixel(x0 - y, y0 + x, color);
            NV3030B_DrawPixel(x0 - x, y0 + y, color);
        }
        if (quadrantMask & 0x1) {
            NV3030B_DrawPixel(x0 - y, y0 - x, color);
            NV3030B_DrawPixel(x0 - x, y0 - y, color);
        }
    }
}
//==============================================================================		

//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( пустотелый )
//==============================================================================
void
NV3030B_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color) {

    int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
    if (cornerRadius > max_radius) {
        cornerRadius = max_radius;
    }

    NV3030B_DrawLine(x + cornerRadius, y, x + cornerRadius + width - 1 - 2 * cornerRadius, y, color);         // Top
    NV3030B_DrawLine(x + cornerRadius, y + height - 1, x + cornerRadius + width - 1 - 2 * cornerRadius, y + height - 1,
                     color); // Bottom
    NV3030B_DrawLine(x, y + cornerRadius, x, y + cornerRadius + height - 1 - 2 * cornerRadius, color);         // Left
    NV3030B_DrawLine(x + width - 1, y + cornerRadius, x + width - 1, y + cornerRadius + height - 1 - 2 * cornerRadius,
                     color); // Right

    // draw four corners
    NV3030B_DrawCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 1, color);
    NV3030B_DrawCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 2, color);
    NV3030B_DrawCircleHelper(x + width - cornerRadius - 1, y + height - cornerRadius - 1, cornerRadius, 4, color);
    NV3030B_DrawCircleHelper(x + cornerRadius, y + height - cornerRadius - 1, cornerRadius, 8, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования линия толстая ( последний параметр толщина )
//==============================================================================
void NV3030B_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t thick) {
    const int16_t deltaX = abs(x2 - x1);
    const int16_t deltaY = abs(y2 - y1);
    const int16_t signX = x1 < x2 ? 1 : -1;
    const int16_t signY = y1 < y2 ? 1 : -1;

    int16_t error = deltaX - deltaY;

    if (thick > 1) {
        NV3030B_DrawCircleFilled(x2, y2, thick >> 1, color);
    } else {
        NV3030B_DrawPixel(x2, y2, color);
    }

    while (x1 != x2 || y1 != y2) {
        if (thick > 1) {
            NV3030B_DrawCircleFilled(x1, y1, thick >> 1, color);
        } else {
            NV3030B_DrawPixel(x1, y1, color);
        }

        const int16_t error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}
//==============================================================================		


//==============================================================================
// линия толстая нужной длины и указаным углом поворота (0-360) ( последний параметр толшина )
//==============================================================================
void NV3030B_DrawLineThickWithAngle(int16_t x, int16_t y, int16_t length, double angle_degrees, uint16_t color,
                                    uint8_t thick) {
    double angleRad = (360.0 - angle_degrees) * PI / 180.0;
    int16_t x2 = x + (int16_t)(cos(angleRad) * length) + 0.5;
    int16_t y2 = y + (int16_t)(sin(angleRad) * length) + 0.5;

    NV3030B_DrawLineThick(x, y, x2, y2, color, thick);
}
//==============================================================================


//==============================================================================
// Процедура рисования дуга толстая ( часть круга )
//==============================================================================
void NV3030B_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color,
                     uint8_t thick) {

    int16_t xLast = -1, yLast = -1;

    if (startAngle > endAngle) {
        // Рисование первой части дуги от startAngle до 360 градусов
        for (int16_t angle = startAngle; angle <= 360; angle += 2) {
            float angleRad = (float) (360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    NV3030B_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    NV3030B_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }

        // Рисование второй части дуги от 0 до endAngle
        for (int16_t angle = 0; angle <= endAngle; angle += 2) {
            float angleRad = (float) (360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    NV3030B_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    NV3030B_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }
    } else {
        // Рисование дуги от startAngle до endAngle
        for (int16_t angle = startAngle; angle <= endAngle; angle += 2) {
            float angleRad = (float) (360 - angle) * PI / 180;
            int x = cos(angleRad) * radius + x0;
            int y = sin(angleRad) * radius + y0;

            if (xLast != -1 && yLast != -1) {
                if (thick > 1) {
                    NV3030B_DrawLineThick(xLast, yLast, x, y, color, thick);
                } else {
                    NV3030B_DrawLine(xLast, yLast, x, y, color);
                }
            }

            xLast = x;
            yLast = y;
        }
    }
}
//==============================================================================

#pragma GCC diagnostic pop        // Восстанавливаем настройки

#if FRAME_BUFFER
//==============================================================================
// Процедура вывода буффера кадра на дисплей
//==============================================================================
void NV3030B_Update(void){

        NV3030B_SetWindow(0, 0, NV3030B_Width-1, NV3030B_Height-1);

        NV3030B_Select();

        NV3030B_SendDataMASS((uint8_t*)buff_frame, sizeof(uint16_t)*NV3030B_Width*NV3030B_Height);

        NV3030B_Unselect();
}
//==============================================================================

//==============================================================================
// Процедура очистка только буфера кадра  ( при етом сам экран не очищаеться )
//==============================================================================
void NV3030B_ClearFrameBuffer(void){
    memset((uint8_t*)buff_frame, 0x00, NV3030B_Width*NV3030B_Height*sizeof(uint16_t) );
}
//==============================================================================
#endif




//#########################################################################################################################
//#########################################################################################################################




/************************ (C) COPYRIGHT GKP *****END OF FILE****/
