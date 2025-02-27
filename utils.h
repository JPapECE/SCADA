#ifndef UTILS_H
#define	UTILS_H
#include <avr/io.h>
#include <string.h>

extern volatile uint8_t cmd;

typedef struct {
    int buttonPD6;
    int buttonPD7;
    int PWM;
    char LCD[17];
} DeviceState;

void ADC_init(void) {
    ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void ADMUX_select(uint8_t pot) {
    ADMUX &= 0;
    ADMUX |= pot;
}

void EXP_LCD_init(void) {
    //Set EXT_PORT0 as output
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);
    
    // Display init
    lcd_init();
    _delay_ms(50);
    lcd_clear_display();
}

void keypad_init(void) {
    // IO1[3:0] OUTPUTS
    // IO1[7:4] INPUTS
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    
    // Setting IO1[3:0] HIGH
    PCA9555_0_write(REG_OUTPUT_1, 0xFF);
}

void PWM_init(void) {
    // configure TMR1
    TCCR1B = (1 << CS12) | (1 << WGM12);
    TCCR1A = (1 << WGM10) | (1 << COM1A1);
}

uint16_t ADC_read(void) {
    uint16_t ADC_value;
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));   //when conversion is finished then ADSC becomes 0
    ADC_value = ADC;
    
    return ADC_value;
}

void parse_string(const char *input, DeviceState *state) {
    state->buttonPD6 = input[8]-'0';
    state->buttonPD7 = input[14]-'0';
    int i = 20;
    int j = 0;
    char x[4];
    while(input[i] != ',') x[j++] = input[i++];
    x[i] = '\0';
    state->PWM = atoi(x);
    i += 5;
    j=0;
    while(input[i] != '\0') state->LCD[j++] = input[i++];
    state->LCD[j] = '\0';
}

void lcd_print_str(char name[]) {
    int i = 0; 
    while(name[i] != '\0') {
        lcd_data(name[i]);
        ++i;
    }
    while(i<16) {
        lcd_data(' ');
        ++i;
    }

}

int check_measurements(uint16_t pot1, uint16_t pot2, float temp) {
    cmd = 0b11111111;
    
    // Critical problem with 1 or more measurements => immediate shutdown (of the machine) and notify the PC
    if(pot1 < 100 || pot1 > 900) cmd &= 0b11101111;
    if(pot2 < 100 || pot2 > 900) cmd &= 0b11011111;
    if(temp < 15 || temp > 35) cmd &= 0b10111111;
    
//    if(pot1 < 90 || pot1 > 1000) cmd &= 0b11111101;
//    if(pot2 < 40 || pot2 > 1010) cmd &= 0b11111011;
//    if(temp < 5 || temp > 600) cmd &= 0b11110111;
    
    // Problem with 1 or more measurements => send the measurements and notify the PC
    if(pot1 < 10 || pot1 > 1010) cmd &= 0b11111101;
    if(pot2 < 10 || pot2 > 1010) cmd &= 0b11111011;
    if(temp < 5 || temp > 40) cmd &= 0b11110111;
    
    
    return cmd;
}

#endif	/* UTILS_H */
