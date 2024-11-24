#include <reg51.h>

// Define LCD pins
sbit rs = P2^0;   // Register Select: 0 = Command, 1 = Data
sbit en = P2^1;   // Enable: Used to latch commands/data
sfr ldata = 0x90; // Data lines connected to Port 1 (address 0x90)

// Define SPI pins
sbit clk = P3^0;  // Clock pin for SPI communication
sbit din = P3^1;  // Data input pin for SPI
sbit dout = P3^2; // Data output pin for SPI
sbit cs = P3^3;   // Chip Select for SPI device

// Function prototypes
void delay(unsigned int i);
void lcd_cmd(unsigned char a);
void lcd_data(unsigned char b);
void lcd_init(void);
void lcd_str(unsigned char *str);
unsigned int spi_adc_value(void);
void hex2ascii(unsigned char value);

void main() {
    unsigned int temp;

    // Initialize the LCD
    lcd_init();

    // Display welcome message
    lcd_str(" WELCOME TO ");
    lcd_cmd(0xC0); // Move to the second row
    lcd_str(" SPI PROJECT ");
    delay(65000);

    // Clear LCD and prepare to display SPI temperature
    lcd_cmd(0x01); // Clear display
    lcd_cmd(0x80); // Move to the first row
    lcd_str(" TEMPERATURE: ");

    while (1) {
        temp = spi_adc_value(); // Read ADC value via SPI
        lcd_cmd(0xC0);          // Move to the second row
				temp = temp / 8.4;      // Convert ADC value to temperature
        hex2ascii(temp);        // Display temperature as ASCII
    }
}

// Function to convert a number to ASCII and display it on the LCD
void hex2ascii(unsigned char value) {
    unsigned char x, d1, d2, d3;

    d3 = value % 10;  // Units place
		value = value / 10;
    d2 = value % 10;      // Tens place
		value = value / 10;
    d1 = value % 10;      // Hundreds place

    // Display each digit as ASCII
    lcd_data(d1 + 0x30); // Convert to ASCII and display
    lcd_data(d2 + 0x30);
    lcd_data(d3 + 0x30);
}

// Function to initialize the LCD
void lcd_init() {
    lcd_cmd(0x38); // 8-bit mode, 2-line display, 5x7 font
    lcd_cmd(0x0C); // Display ON, cursor OFF
    lcd_cmd(0x01); // Clear display
    lcd_cmd(0x80); // Move to the first row, first column
}

// Delay function for timing
void delay(unsigned int i) {
    unsigned int j;
    for (j = 0; j < i; j++);
}

// Function to send a command to the LCD
void lcd_cmd(unsigned char a) {
    rs = 0;        // Command mode
    ldata = a;     // Load command on data lines
    en = 1;        // Enable pulse
    delay(5);
    en = 0;
    delay(5);
}

// Function to send data to the LCD
void lcd_data(unsigned char b) {
    rs = 1;        // Data mode
    ldata = b;     // Load data on data lines
    en = 1;        // Enable pulse
    delay(5);
    en = 0;
    delay(5);
}

// Function to send a string to the LCD
void lcd_str(unsigned char *str) {
    while (*str) {
        lcd_data(*str++); // Send characters one by one
    }
}

// Function to read the ADC value via SPI
unsigned int spi_adc_value(void) {
    unsigned int temp = 0;
    char i;

    cs = 0;         // Activate the ADC chip (Chip Select = 0)

    // Send start bit and mode selection
    clk = 0; din = 1; clk = 1; // Start bit
    clk = 0; din = 1; clk = 1; // Single-ended mode
    clk = 0; din = 0; clk = 1; // D2 (don't care for single-ended)
    clk = 0; din = 0; clk = 1; // D1
    clk = 0; din = 0; clk = 1; // D0

    // Null bit and sampling
    clk = 0; din = 1; clk = 1; // T sample
    clk = 0; din = 1; clk = 1; // Null bit

    // Read 12-bit ADC data
    for (i = 11; i >= 0; i--) {
        clk = 0;
        if (dout == 1) {
            temp |= (1 << i); // Collect each bit
        }
        clk = 1;
    }

    cs = 1; // Deactivate the ADC chip (Chip Select = 1)
    return temp; // Return the ADC value
}
