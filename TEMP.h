#ifndef TEMP_H
#define	TEMP_H

#include "display_ext.h"

int one_wire_reset(){
    DDRD |=  (1<<PD4); /*set bit PD4 as output*/
    PORTD &= ~(1<<PD4); /* Clear PD4*/
    
    _delay_us(480);
    
    DDRD &= ~(1<<PD4); /*set PD4 as input*/
    PORTD &= ~(1<<PD4); /* Clear PD4*/
    
    _delay_us(100);
    
    uint8_t input ;
    input = PIND;
    
    _delay_us(380);
    
    uint8_t tmp; 
    tmp = input & (1<<PD4);
    if(tmp == 0x00) return 1;
    else return 0;
}
void one_wire_transmit_bit(uint8_t info){
    DDRD |=  (1<<PD4);
    PORTD &= ~(1<<PD4);             //set pd as output
    
    _delay_us(2);                   //time slot 2 usec
    
    uint8_t tmp;
    tmp = info & (1<<PD0);
    
    tmp <<= 4;
    
    PORTD |= tmp;                   //pd4 = info[0]
    
    _delay_us(58);                  //wait for device
    
    DDRD &= ~(1<<PD4);              //setP PD4 as input
    PORTD &= ~(1<<PD4);             //disable pull-up
    
    _delay_us(1);                   //recovery time 1 usec
    return;
}

uint8_t one_wire_receive_bit(){
    DDRD |= (1<<PD4); /*set bit PD4 as output*/
    PORTD &= ~(1<<PD4); /* Clear PD4*/
    
    _delay_us(2);
    
    DDRD &=  ~(1<<PD4); /*set PD4 as input*/
    PORTD &= ~(1<<PD4); /* Clear PD4*/
   
    _delay_us(10);
    
    uint8_t temp;
    temp = PIND;
    temp &= (1<<PD4); /*read PD4*/
    
    temp >>= 4; /*shift PD4 to LSB*/
    
    
    
    _delay_us(49);
    
    return temp;
    
}

void one_wire_transmit_byte(uint8_t info){
    uint8_t bit ;
    for(int i = 0; i <8; i++){
        bit = info >> i; //i-th bit to lsb
        bit &= 0x01;  //keep LSB
        one_wire_transmit_bit(bit); //call one_wire_transmit_bit
    }
    return;
}

uint8_t one_wire_receive_byte(){
    uint8_t info;
    info = 0x00;
    for(int i = 0 ; i < 8 ; ++i){
        uint8_t temp;
        temp = one_wire_receive_bit(); /*read the i-th bit*/
        
        temp <<= i; /*shift it to the i-th bit*/
        info |= temp ; /*save it to info*/
    }
    
    return info;
}

uint16_t read_temp(){
    int check; /*check if there is a device connected*/
    check = one_wire_reset();
    
    if (check == 0){ /*if there is no device return 0x8000*/
        return 0x8000;
    }
    
    one_wire_transmit_byte(0xCC); /*skip multi-device bus*/
    
    one_wire_transmit_byte(0x44); /*start temperature conversion*/
    
   while(one_wire_receive_bit() == 0x00){
        _delay_ms(750);
    }
    
    
    check = one_wire_reset(); /*new initialization*/
    if (check == 0){ /*if there is no device return 0x8000*/
        return 0x8000;
    }

    one_wire_transmit_byte(0xCC); /*skip multi-device bus*/

    one_wire_transmit_byte(0xBE); /*send command to read the temperature*/ 
    uint16_t data;
    uint8_t lsbyte , msbyte;
    lsbyte = one_wire_receive_byte();

    msbyte = one_wire_receive_byte();

    data = msbyte;
    data = data << 8;
    data += lsbyte;
   
    
    
    return data;  
}
/*a function that converts the 16 bit temprature to a real number
for sensor 18B20*/
float temp2float_18B20(uint16_t data){
    
    
    uint16_t sign; /*checks the sign*/
    sign = data & 0x8000;
    

    if (sign) {
        data = ((data ^ 0xffff) + 1) * -1;
    }

    return data / 16.0;
}


/*a function that converts the 16 bit temprature to a real number
for sensor 1820*/
float temp2float_1820(uint16_t data){
    
    
    uint16_t sign; /*checks the sign*/
    sign = data & 0xFF00;
    

    if (sign) {
        data = ((data ^ 0xffff) + 1) * -1;
    }

    return data / 2.0;
}


/*a procedure that prints the temp*/
void lcd_temp(float temperature){

    temperature *= 10;

    char digits[3];
    int temp = (int)temperature;
    
    
     
    digits[2] = temp / 100; 
    temp %= 100;
    digits[1] = temp / 10;
    digits[0] = temp % 10;


    if(digits[2] != 0)
        lcd_data(digits[2] + '0');
    
    lcd_data(digits[1] + '0');
    
    if(digits[0] != 0){
        lcd_data('.');
        lcd_data(digits[0] + '0');
            
    }

    
    lcd_data(223); /*print degree symbol*/

    lcd_data('C');

    
}

#endif