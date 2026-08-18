/*
Name:   Karim Nasrallah
Date:   7/24/26
Assignment lab 4
Youtube: https://youtube.com/shorts/EOvRSBnXCg0?si=rD6dYO5et_pY3gWV

*/

//Peripheral bases
#define RCC_BASE		0x40021000
#define GPIOA_BASE		0x48000000
#define GPIOC_BASE		0x48000800
#define USART1_BASE		0x40013800


#define RCC_AHBENR		(*(volatile unsigned int*) (RCC_BASE + 0x14))
#define RCC_APB2ENR		(*(volatile unsigned int*) (RCC_BASE + 0x18))


#define GPIOA_MODER		(*(volatile unsigned int*) (GPIOA_BASE + 0x00))
#define GPIOA_AFRH		(*(volatile unsigned int*) (GPIOA_BASE + 0x24))

//Port C mode and input
#define GPIOC_MODER		(*(volatile unsigned int*) (GPIOC_BASE + 0x00))
#define GPIOC_IDR		(*(volatile unsigned int*) (GPIOC_BASE + 0x10))

// USART1 registers
#define USART1_CR1		(*(volatile unsigned int*) (USART1_BASE + 0x00))
#define USART1_BRR		(*(volatile unsigned int*) (USART1_BASE + 0x0C))
#define USART1_ISR		(*(volatile unsigned int*) (USART1_BASE + 0x1C))
#define USART1_TDR		(*(volatile unsigned int*) (USART1_BASE + 0x28))


#define BUTTON		13

#define VOLUME		20
#define LONGPRESS	400
#define BOOT_WAIT	2000
#define DEBOUNCE	20

//Software delay
void delay(volatile unsigned int time){
	while (time--){
		for (volatile int i = 0; i < 1000; i++);
	}
}

// reads the button, returns 1 when pressed
int button_pressed(void){
	if ((GPIOC_IDR & (1 << BUTTON)) == 0){
		return 1;
	}
	return 0;
}


void UART_SendByte(unsigned char data){
	while (!(USART1_ISR & (1 << 7)));
	USART1_TDR = data;
}


void DFPlayer_SendCmd(unsigned char cmd, unsigned char dh, unsigned char dl){
	unsigned int checksum = 0 - (0xFF + 0x06 + cmd + 0x00 + dh + dl);
	unsigned char ckh = (checksum >> 8) & 0xFF;
	unsigned char ckl = checksum & 0xFF;

	UART_SendByte(0x7E);
	UART_SendByte(0xFF);
	UART_SendByte(0x06);
	UART_SendByte(cmd);
	UART_SendByte(0x00);
	UART_SendByte(dh);
	UART_SendByte(dl);
	UART_SendByte(ckh);
	UART_SendByte(ckl);
	UART_SendByte(0xEF);
}

// Initialize
void init(void){

	// Turn on the clocks for GPIO ports A and C
	RCC_AHBENR |= (1 << 17);
	RCC_AHBENR |= (1 << 19);

	// Turn on the clock for USART1
	RCC_APB2ENR |= (1 << 14);

	// PA9 as alternate function for USART1_TX
	GPIOA_MODER &= ~(3 << (9 * 2));
	GPIOA_MODER |=  (2 << (9 * 2));

	// AF1 selects USART1_TX on PA9
	GPIOA_AFRH &= ~(0xF << 4);
	GPIOA_AFRH |=  (0x1 << 4);

	// PC13 as input for the button
	GPIOC_MODER &= ~(3 << (BUTTON * 2));

	// 9600 baud from the 8 MHz clock (8000000 / 9600 = 833)
	USART1_BRR = 833;

	// Enable the transmitter, then the USART
	USART1_CR1 |= (1 << 3);
	USART1_CR1 |= (1 << 0);
}

int main(void){
	init();

	int track = 1;
	int playing = 0;

	
	delay(BOOT_WAIT);
	DFPlayer_SendCmd(0x06, 0x00, VOLUME);

	while(1){

		if (button_pressed()){

			
			delay(DEBOUNCE);

			
			unsigned int held = 0;
			while (button_pressed()){
				delay(1);
				held++;
			}

			
			delay(DEBOUNCE);

			if (held >= LONGPRESS){
				// long press: next track (1 -> 2 -> 3 -> 1)
				track++;
				if (track > 3){
					track = 1;
				}
				DFPlayer_SendCmd(0x03, 0x00, track);
				playing = 1;
			}
			else {
				// short press: toggle play and pause
				if (playing){
					DFPlayer_SendCmd(0x0E, 0x00, 0x00);
					playing = 0;
				}
				else {
					DFPlayer_SendCmd(0x0D, 0x00, 0x00);
					playing = 1;
				}
			}
		}
	}
}