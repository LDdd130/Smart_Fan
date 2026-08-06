#include "lcd.h"
#include "fndtimer.h"

// --- [1. I2C(TWI) 제어 함수] ---
void I2C_Init(void) {
    TWSR = 0x00; // 프리스케일러 1
    TWBR = ((F_CPU / 100000L) - 16) / 2; // 100kHz 설정
}

void I2C_Start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))); // 시작 신호 완료 대기
}

void I2C_Stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void I2C_Write(unsigned char data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))); // 데이터 전송 완료 대기
}

static void LCD_SetCursor(uint8_t row, uint8_t col)
{
    LCD_Write_4bit((row == 0 ? 0x80 : 0xC0) + col, 0);
}

// --- [2. I2C LCD 1602 제어 함수] ---
// 주소가 0x27인 경우 0x4E, 0x3F인 경우 0x7E를 사용하세요.
#define LCD_ADDR 0x4E 

void LCD_Write_4bit(unsigned char data, unsigned char rs) {
    unsigned char control = rs ? 0x01 : 0x00; // RS=1(Data), RS=0(Command)
    
    I2C_Start();
    I2C_Write(LCD_ADDR);
    
    // 상위 4비트 전송 (Enable Pulse 포함)
    I2C_Write((data & 0xF0) | control | 0x0C); // En=1, Backlight=1
    _delay_us(2);
    I2C_Write((data & 0xF0) | control | 0x08); // En=0, Backlight=1
    
    // 하위 4비트 전송
    I2C_Write(((data << 4) & 0xF0) | control | 0x0C);
    _delay_us(2);
    I2C_Write(((data << 4) & 0xF0) | control | 0x08);
    
    I2C_Stop();

    if (rs == 0 && (data == 0x01 || data == 0x02)) {
        _delay_us(1600);
    } else {
        _delay_us(50);
    }
}

void LCD_Init(void) {
    _delay_ms(50);
    LCD_Write_4bit(0x33, 0); // 초기화
    LCD_Write_4bit(0x32, 0); // 4비트 모드 설정
    LCD_Write_4bit(0x28, 0); // 2줄, 5x8 폰트
    LCD_Write_4bit(0x0C, 0); // Display ON, Cursor OFF
    LCD_Write_4bit(0x01, 0); // 화면 Clear
    _delay_ms(2);
}

void LCD_WriteString(const char *str)
{
    while(*str) LCD_Write_4bit(*str++, 1);
}

// LCD 1행에는 온습도, 2행에는 팬 현재 파워와 목표 파워를 표시한다.
// Target은 온습도로 계산된 목표 팬 파워이고, Duty는 실제로 팬에 적용 중인 현재 파워다.
void LCD_PrintStatus(uint8_t temperature, uint8_t humidity, uint16_t fan_duty, uint16_t target_duty)
{
    char line[17];
    uint8_t duty_percent = FAN_getDutyPercent(fan_duty);
    uint8_t target_percent = FAN_getDutyPercent(target_duty);

    LCD_SetCursor(0, 0);
    snprintf(line, sizeof(line), "T:%3uC H:%3u%%    ", temperature, humidity);
    LCD_WriteString(line);

    LCD_SetCursor(1, 0);
    snprintf(line, sizeof(line), "F:%3u%% T:%3u%%   ", duty_percent, target_percent);
    LCD_WriteString(line);
}

// DHT 읽기에 실패했을 때 LCD에 오류 상태를 표시한다.
void LCD_PrintDhtError(void)
{
    LCD_SetCursor(0, 0);
    LCD_WriteString("DHT read failed  ");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Check sensor     ");
}