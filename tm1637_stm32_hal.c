#include "tm1637_stm32_hal.h"

#define CLK_PORT GPIOC
#define DIO_PORT GPIOC
#define CLK_PIN GPIO_PIN_0
#define DIO_PIN GPIO_PIN_1
#define CLK_PORT_CLK_ENABLE __HAL_RCC_GPIOC_CLK_ENABLE
#define DIO_PORT_CLK_ENABLE __HAL_RCC_GPIOC_CLK_ENABLE

static void _tm1637Start(void);
static void _tm1637Stop(void);
static void _tm1637ReadResult(void);
static void _tm1637WriteByte(uint8_t b);
static void _tm1637DelayUsec(uint32_t i);
static void _tm1637ClkHigh(void);
static void _tm1637ClkLow(void);
static void _tm1637DioHigh(void);
static void _tm1637DioLow(void);
void tm1637Init(void);
void tm1637DisplayDecimal(uint32_t v, uint8_t displaySeparator);
void tm1637SetBrightness(uint8_t brightness);

const uint8_t segmentMap[] = {
    0x3f, // 0d
    0x06, // 1d
    0x5b, // 2d
    0x4f, // 3d
    0x66, // 4d 
    0x6d, // 5d
    0x7d, // 6d
    0x07, // 7d
    0x7f, // 8d
    0x6f, // 9d
    0x77, // A
    0x7c, // b
    0x39, // C
    0x5e, // d
    0x79, // E
    0x71, // F
    0x50, // r
    0x00  // EMPTY
};


void tm1637Init(void)
{
    CLK_PORT_CLK_ENABLE();
    DIO_PORT_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pull = GPIO_PULLUP;
    g.Mode = GPIO_MODE_OUTPUT_OD; // OD = open drain
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin = CLK_PIN;
    HAL_GPIO_Init(CLK_PORT, &g);
    g.Pin = DIO_PIN;
    HAL_GPIO_Init(DIO_PORT, &g);

    tm1637SetBrightness(8);
}


void tm1637SetBrightness(uint8_t brightness)
{
    // Brightness command:
    // 1000 0XXX = display off
    // 1000 1BBB = display on, brightness 0-7
    // X = don't care
    // B = brightness
    _tm1637Start();
    _tm1637WriteByte(0x87 + brightness);
    _tm1637ReadResult();
    _tm1637Stop();
}

static void _tm1637Start(void)
{
    _tm1637ClkHigh();
    _tm1637DioHigh();
    _tm1637DelayUsec(2);
    _tm1637DioLow();
}


void tm1637DisplayDecimal(uint32_t v, uint8_t displaySeparator)
{
    uint8_t digitArr[4];
    for (uint8_t i = 0; i < 4; ++i) 
    {
        digitArr[i] = segmentMap[v % 10];
        if (i == 2 && displaySeparator == 1) // if have separator
        {
            digitArr[i] |= 1 << 7;

        }
        v /= 10;
    }

    _tm1637Start();
    _tm1637WriteByte(0x40);
    _tm1637ReadResult();
    _tm1637Stop();

    _tm1637Start();
    _tm1637WriteByte(0xc0);
    _tm1637ReadResult();

    for (uint8_t i = 0; i < 4; ++i) 
    {
        _tm1637WriteByte(digitArr[3 - i]);
        _tm1637ReadResult();
    }

    _tm1637Stop();
}

static void _tm1637Stop(void)
{
    _tm1637ClkLow();
    _tm1637DelayUsec(2);
    _tm1637DioLow();
    _tm1637DelayUsec(2);
    _tm1637ClkHigh();
    _tm1637DelayUsec(2);
    _tm1637DioHigh();
}

static void _tm1637ReadResult(void)
{
    _tm1637ClkLow();
    _tm1637DelayUsec(5);
    // while (dio); // We're cheating here and not actually reading back the response.
    _tm1637ClkHigh();
    _tm1637DelayUsec(2);
    _tm1637ClkLow();
}

static void _tm1637WriteByte(uint8_t b)
{
    /*
    for example b = 0b1001 (9d):
        1 step - 0b00001001 & 0b00000001 = 1 => write 1 and right shift
        2 step - 0b00000100 & 0b00000001 = 0 => write 0 and right shift
        3 step - 0b00000010 & 0b00000001 = 0 => write 0 and right shift
        4 step - 0b00000001 & 0b00000001 = 1 => write 1 and right shift
    */

    for (uint8_t i = 0; i < 8; ++i) // a byte cycle 
    {
        _tm1637ClkLow();

        if (b & 0x01)         // if bit is 1
            _tm1637DioHigh(); // write 1
        else  
            _tm1637DioLow();  // else write 0

        _tm1637DelayUsec(3);

        b >>= 1;              // right bit shift for 1 

        _tm1637ClkHigh();
        _tm1637DelayUsec(3);
    }
}

static void _tm1637DelayUsec(uint32_t i)
{
    for (; i>0; i--) 
    {
        for (int j = 0; j < 10; ++j) 
        {
            __asm  // a syntax for arm compiler 
                   // for gcc - another syntax
                   // should be like this : __asm__ __volatile__("nop\n\t":::"memory");
            {
                nop
            }
        }
    }
}

static void _tm1637ClkHigh(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
}

static void _tm1637ClkLow(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
}

static void _tm1637DioHigh(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_SET);
}

static void _tm1637DioLow(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_RESET);
}
