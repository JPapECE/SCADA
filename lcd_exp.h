#ifndef LCD_EXP_H
#define	LCD_EXP_H

#ifdef	__cplusplus
extern "C" {
#endif


#define F_CPU 16000000UL 
#include<avr/io.h> 
#include<avr/interrupt.h> 
#include<util/delay.h> 
#include <avr/cpufunc.h> 
#include "twi.h"



void write_2_nibbles(uint8_t lcd_data){
    uint8_t r24 = PCA9555_0_read(REG_INPUT_0);    /*read from IO_port*/
    r24 &= 0x0f;       /*keep only 4 lsb's*/
    
    /*PORTD = (lcd_data & 0xf0) | temp;*/
    PCA9555_0_write( REG_OUTPUT_0 , (lcd_data & 0xf0) | r24);
    
    /* set enable pulse*/
    /*PORTD |= (1<<PD3);*/
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write( REG_OUTPUT_0 , temp);
    
    _NOP();             /* NOP equivalent in C*/
    _NOP();
    
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);

    
    lcd_data <<= 4;     /*Shift data left to get low nibble in high 4 bits*/
    
    /*PORTD = (lcd_data & 0xf0) | temp;*/
    //temp = PCA9555_0_read(REG_INPUT_0);    /*read from IO_port*/
    //temp &= 0x0f;       /*keep only 4 lsb's*/
    
    PCA9555_0_write( REG_OUTPUT_0 , (lcd_data & 0xf0) | r24);
    
    
    
    /* set enable pulse*/
    /*PORTD |= (1<<PD3);*/
    temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write( REG_OUTPUT_0 , temp);
    _NOP();             /* NOP equivalent in C*/
    _NOP();
    
    
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);

    
    
    return;
}

void lcd_data(uint8_t data){
	/*PORTD = PORTD | (1<<PD2);*/
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD2);
    PCA9555_0_write( REG_OUTPUT_0 , temp);
	write_2_nibbles(data);
	_delay_ms(1);
	return;

}

void lcd_command(uint8_t cmd){
	/*int temp = ~(1<<PD2);
	PORTD = PORTD & temp;*/
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD2);
    PCA9555_0_write( REG_OUTPUT_0 , temp);
    write_2_nibbles(cmd);
	_delay_ms(1);
	return;
}

void lcd_clear_display(){
	lcd_command(1);
	_delay_ms(5);
	return;
}

void lcd_init(){
	_delay_ms(200);
	 //8bit mode
    /*PORTD= 0x30;*/
    PCA9555_0_write( REG_OUTPUT_0 , 0x30);
    
	//enable pulse
    /*PORTD |= (1<<PD3);*/
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_NOP();
	_NOP();
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_delay_ms(1);
    
     //8bit mode
    /*PORTD= 0x30;*/
    PCA9555_0_write( REG_OUTPUT_0 , 0x30);
    
	//enable pulse
    /*PORTD |= (1<<PD3);*/
    temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_NOP();
	_NOP();
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_delay_ms(1);
    //8bit mode
    /*PORTD= 0x30;*/
    PCA9555_0_write( REG_OUTPUT_0 , 0x30);
    
	//enable pulse
    /*PORTD |= (1<<PD3);*/
    temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_NOP();
	_NOP();
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_delay_ms(1);
    
    //4bit mode
    /*PORTD= 0x20;*/
    PCA9555_0_write( REG_OUTPUT_0 , 0x20);
    
	//enable pulse
    /*PORTD |= (1<<PD3);*/
    temp = PCA9555_0_read(REG_INPUT_0);
    temp |= (1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_NOP();
	_NOP();
    /* Clear enable pulse*/
    /*PORTD &= ~(1<<PD3); */
    temp = PCA9555_0_read(REG_INPUT_0);
    temp &= ~(1<<PD3);
    PCA9555_0_write(REG_OUTPUT_0 , temp);
	_delay_ms(1);
    /*
	PORTD= 0x20; //4bit mode
	PORTD |= (1<<PD3); //enable pulse
	_NOP();
	_NOP();
	PORTD &= ~(1<<PD3);
	_delay_ms(1);
     */
	lcd_command(0x28); //5x8 dots, 2 lines
	lcd_command(0x0c); // display on, cursor off
	lcd_clear_display();
	lcd_command(0x06);
	return;
}

void lcd_exp_init(){
    // setting PORTD output because of display functions
    DDRD = 0xFF;
    
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output

    lcd_init();
    _delay_ms(100);

    lcd_clear_display();
    _delay_ms(1000);
}

void lcd_print_word(char data[]){
    int index=0;
    while(data[index] != '\0' && data[index] != '\n' ){
        lcd_data(data[index++]);
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_EXP_H */