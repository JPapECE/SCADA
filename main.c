#define F_CPU 16000000UL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "display_ext.h"
#include "twi.h"
#include "uart.h"
#include "utils.h"
#include "TEMP.h"

#define MAX_PAYLOAD_SIZE 100
#define pot1 (0 << MUX3)|(0 << MUX2)|(0 << MUX1)|(0 << MUX0)
#define pot2 (0 << MUX3)|(0 << MUX2)|(0 << MUX1)|(1 << MUX0)
#define pot3 (0 << MUX3)|(0 << MUX2)|(1 << MUX1)|(0 << MUX0)
#define pot4 (0 << MUX3)|(0 << MUX2)|(1 << MUX1)|(1 << MUX0)

char send_buf[MAX_PAYLOAD_SIZE] = {0};
char rec_buf[MAX_PAYLOAD_SIZE] = {0};
char sys_on[] = "SYSTEM: ONLINE";
char sys_off[] = "SYSTEM: OFFLINE";

DeviceState state = {0};

volatile uint8_t cmd = 0b11111111;

ISR (INT0_vect)              
{
    /* ============ Button Debouncing ============ */
    while((EIFR & 0b00000001) == 0b00000001) {
        EIFR = (1 << INTF0);
        _delay_ms(1);
    }
    /* =========================================== */
    cmd &= 0b11111110;
    send_string("POT1:0,POT2:0,TEMP:0.0,BUTTONS:00,STATUS:254\n");
    lcd_command(0x80);
    lcd_print_str(sys_off);
           
    // turn the interrupts on and wait here until the re-start button is pressed  
    sei(); 

    while((cmd & 0b00000001) == 0) {
         
    } 
}

ISR (INT1_vect)              
{
    /* ============ Button Debouncing ============ */
    while((EIFR & 0b00000010) == 0b00000010) {
        EIFR = (1 << INTF1);
        _delay_ms(5);
    }
    /* =========================================== */
    
    // If the button is pressed and the system is already online do nothing 
    if((cmd & 0b00000001) == 0b00000001) return; 
        
    // else we need to turn it back on 
    cmd = 0b11111111;
      
    lcd_command(0x80);
    lcd_print_str(sys_on);
}

int main() {
    DDRB = 0x07;            // PORTB
    DDRC = 0x00;            // PORTC as input
    DDRD = 0b11000000;      // PORTD as input - output 
    
    ADC_init();
    // Init UART
    usart_init(103);
        
    // Init timers for the PWM 
    TCCR1A = (1<<WGM10)|(1<<COM1A1);
    TCCR1B = (1<<WGM12)|(1<<CS10);
    
    // Init INT0
    EICRA= (1<<ISC01) | (1<<ISC00) | (1<<ISC11) | (1<<ISC10); 
    EIMSK= (1<<INT0) | (1<<INT1); 
    sei();
    
    // Init LCD 
    twi_init();
    EXP_LCD_init();
    lcd_print_str(sys_on);
    
    int duty = 0;
    
    
    uint16_t pot1_val = 0;
    uint16_t pot2_val = 0;
    uint8_t status = 0;
    uint8_t buttons;
    
    while(1) {
        memset(send_buf, 0, MAX_PAYLOAD_SIZE);
        memset(rec_buf, 0, MAX_PAYLOAD_SIZE);
        
        while(1) {
            memset(rec_buf, 0, MAX_PAYLOAD_SIZE);
            receive_string(rec_buf);
//            sprintf(rec_buf,"msg:PD6:1,PD7:0,PWM:50,LCD:hello" );

            if (strncmp(rec_buf, "msg:", 4) == 0) break;
        }

        
        parse_string(rec_buf, &state);
        
        // READ THE POTENTIOMETERS
        ADMUX_select(pot1);
        pot1_val = ADC_read();
        
        ADMUX_select(pot3);
        pot2_val = ADC_read();
        
        
        //  TEMPERATURE
        uint16_t temporary = read_temp();
        float temperature;
        temperature = (temporary == 0x8000) ?  0 : temp2float_18B20(temporary);
        
        // OPEN - CLOSE PD6 PD7
        PORTD = (state.buttonPD6 << PD6) | (state.buttonPD7 << PD7);
        
        // LCD MESSAGE
        lcd_command(0xC0);
        lcd_print_str(state.LCD);
        
        // LED BRIGHTNESS
        duty = state.PWM;
        OCR1A = 255 * duty / 100;
        
        // DETERMINE STATUS
        status = check_measurements(pot1_val, pot2_val, temperature);
        
        // READ PB4 AND PB5
        buttons = PINB;
        buttons = ~buttons;
        buttons &= 0b00011000;
        buttons = buttons >> 3;
        
        int INT = (int)(temperature*10);
        //FORM PAYLOAD
        sprintf(send_buf,"POT1:%d,POT2:%d,TEMP:%d.%d,BUTTONS:%d,STATUS:%d\n", pot1_val, pot2_val, INT/10, INT%10, buttons, status);
        send_string(send_buf);
    } 
}
