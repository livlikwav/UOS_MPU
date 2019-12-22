#define F_CPU 16000000UL 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MOTOR_CW 0x80
#define MOTOR_CCW 0x40
#define MOTOR_STOP1 0x00
#define MOTOR_STOP2 0x11

unsigned char led[8] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f};
unsigned char digit[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};
unsigned char fnd_sel[4] = {0x08, 0x04, 0x02, 0x01};

volatile int humset=2; //������� 

/*#################�½��� ���start############*/
#define DHT11_ERROR 255
#include "DHT11.h"
volatile unsigned char temperature; // �µ���
volatile unsigned char humidity; // ������
/*#################�½��� ���end #############*/

/* ���� delay�� ���� sec���� delay �Լ�*/
void delay_sec(int sec)
{
	int i;
	for(i=0; i<1000*sec; i++)    
		_delay_ms(1);
}
/*###################################*/



uint8_t dht11_getdata(uint8_t select) // �½��� ��� ���� ����
{	
	
	uint8_t bits[5];
	uint8_t i,j = 0;
	memset(bits, 0, sizeof(bits));
	//reset port
	DHT11_DDR |= (1<<DHT11_INPUTPIN); //output
	DHT11_PORT |= (1<<DHT11_INPUTPIN); //high
	_delay_ms(100);
	//send request
	DHT11_PORT &= ~(1<<DHT11_INPUTPIN); //low
	_delay_ms(18);
	DHT11_PORT |= (1<<DHT11_INPUTPIN); //high
	_delay_us(1);
	DHT11_DDR &= ~(1<<DHT11_INPUTPIN); //input
	_delay_us(39);
	//check start condition 1
	if((DHT11_PIN & (1<<DHT11_INPUTPIN)))
	{
		return DHT11_ERROR;
	}
	_delay_us(80);
	//check start condition 2
	if(!(DHT11_PIN & (1<<DHT11_INPUTPIN)))
	{
		return DHT11_ERROR;
	}
	_delay_us(80);
	//read the data
	for (j=0; j<5; j++) //read 5 byte
	{
		uint8_t result=0;
		for(i=0; i<8; i++) //read every bit
		{
			while(!(DHT11_PIN & (1<<DHT11_INPUTPIN))); //wait for an high input
			_delay_us(30);
			if(DHT11_PIN & (1<<DHT11_INPUTPIN)) //if input is high after 30 us, get result
				result |= (1<<(7-i));
			while(DHT11_PIN & (1<<DHT11_INPUTPIN)); //wait until input get low
		}
		bits[j] = result;
	}
	//reset port
	DHT11_DDR |= (1<<DHT11_INPUTPIN); //output
	DHT11_PORT |= (1<<DHT11_INPUTPIN); //low
	_delay_ms(100);
	//check checksum
	if (bits[0] + bits[1] + bits[2] + bits[3] == bits[4])
	{
		if (select == 0) //return temperature
		{
			
			return(bits[2]);
		}
		else if(select == 1) //return humidity
		{
			
			return(bits[0]);
		}
	}
}
/*
* get temperature (0..50C)
*/
int8_t dht11_gettemperature()
{
	uint8_t ret = dht11_getdata(0);
	if(ret == DHT11_ERROR)
		return -1;
	else
		return ret;
}
/*
* get humidity (20..90%)
*/
int8_t dht11_gethumidity()
{
	uint8_t ret = dht11_getdata(1);
	if(ret == DHT11_ERROR)
		return -1;
	else
		return ret;
}


int main() {
	DDRA = 0xff; // ��Ʈ A�� ��� ������� �Ͽ� LED ����
	DDRB = 0xff; // ��Ʈ B�� ��� ������� �Ͽ� ���� ���� 
	DDRD = 0x00; // ��Ʈ D�� �Է����� ������ �½��������� �����а��� �Է����� �޴´�.
	DDRC = 0xff; // ��Ʈ C�� ��� ������� �Ͽ� FND ����
	
 
	while(1) {
		
		
		humidity = dht11_gethumidity(); //������ �޾ƿ���
		_delay_ms(1000);
		int humt; //���� �ܰ� ǥ�ø� ���� ����
		/* #######���� �ܰ躰 ������ ######*/
		if (humidity<30) humt = 0;
		else if(humidity<38) humt = 1;
		else if(humidity<46) humt = 2;
		else if(humidity<52) humt = 3;
		else if(humidity<60) humt = 4;
		else if (humidity<68) humt = 5;
		else if(humidity<76) humt = 6;
		else humt = 7;
		/* #######���� �ܰ躰 ������ ######*/
		_delay_ms(1000);
		PORTA = led[humt]; //LED�� ������ ǥ��	
		_delay_ms(1000);
		
		if (humt > humset) { //�������� ������ �̻��̸�
			PORTB = MOTOR_CCW; //���� ������
			delay_sec(3); //3�ʵ���
			PORTB = MOTOR_STOP2;
		}
		
		//_delay_ms(1500);
		temperature = dht11_gettemperature(); //�µ��� �޾ƿ���
		uint8_t temten = temperature/10; //fnd�� ǥ���ϱ� ���� 10�� �ڸ� ����
		uint8_t temone = temperature%10; //fnd�� ǥ���ϱ� ���� 1�� �ڸ� ����
		
		for(int i=0; i<100; i++){
			PORTC = digit[temten]; PORTG = fnd_sel[2]; //fnd 3��° �ڸ��� �µ��� 10�� �ڸ��� ���
			_delay_ms(5);
			PORTC = digit[temone]; PORTG = fnd_sel[3]; //fnd 4��° �ڸ��� �µ��� 1�� �ڸ��� ���
			_delay_ms(5);
		 
		}
		PORTC = 0x40; PORTG = fnd_sel[3]; // ���� �ֱ���� fnd 0���� ǥ��
		_delay_ms(1000);
	}
	return 0;
}
