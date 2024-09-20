/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Jan 19, 2024
  * @brief   ECE 362 Lab 3 Student template
  ******************************************************************************
*/

/**
******************************************************************************/

// Fill out your username, otherwise your completion code will have the 
// wrong username!
const char* username = "aafrose";

/******************************************************************************
*/ 

#include "stm32f0xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Global data structure
char disp[9]         = "Hello...";
uint8_t col          = 0;
uint8_t mode         = 'A';
uint8_t thrust       = 0;
int16_t fuel         = 800;
int16_t alt          = 4500;
int16_t velo         = 0;

// Keymap is in `font.S` to match up what autotester expected
extern char keymap;
extern char disp[9];
extern uint8_t col;
extern uint8_t mode;
extern uint8_t thrust;
extern int16_t fuel;
extern int16_t alt;
extern int16_t velo;

// Make it easier to access keymap
char* keymap_arr = &keymap;

// Font array in assembly file
// as I am too lazy to convert it into C array
extern uint8_t font[];

// The functions we should implement
void enable_ports();
void show_char(int n, char c);
void drive_column(int c);
int read_rows();
char rows_to_key(int rows);
void handle_key(char key);
void setup_tim7();
void write_display();
void update_variables();
void setup_tim14();

// Auotest functions
void internal_clock();
extern void check_wiring();
extern void autotest();
extern void fill_alpha();

int main(void) {
    internal_clock();

    // Uncomment when you are ready to test wiring.
    // check_wiring();
    
    // Uncomment when you are ready to test everything.
    // autotest();
    
    enable_ports();
    // Comment out once you are checked off for fill_alpha
    fill_alpha();

    setup_tim7();
    setup_tim14();

    for(;;) {
        // enter low power sleep mode 
        // and Wait For Interrupt (WFI)
        asm("wfi");
    }
}

/**
 * @brief Enable the ports and configure pins as described
 *        in lab handout
 * 
 */
void enable_ports() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;//Enabling clock for port B
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;//Enabling clock for port C
    GPIOB_MODER &= ~0x001FFFFF;//setting 0-10 as defaults for output
    GPIOB_MODER |=  0x00055555;//pins 0-10 outputs for port B
    GPIOC_MODER &= ~0x0000FF00;//setting pins 4-7 as defaults for output
    GPIOC_MODER |=  0x00005500;//pins 4-7 outputs for port C
    GPIOC_PUPDR &= ~0x000000FF;//Setting pins 0-3 as defaults for pull down
    GPIOC_PUPDR |=  0x000000AA;//pull down = pull low, so PUPDR. Setting Pins 0-3 for port C to pull down
}

/**
 * @brief Show a character `c` on column `n`
 *        of the segment LED display
 * 
 * @param n 
 * @param c 
 */
void show_char(int n, char c) {
    bool validRange = false;//using this variable as a way to return value
    uint16_t font = font[int(c)];//Converting char into ASCII values and using this variable to store the c variable data 
    for (int i = 0; i <= 7; i++) {//looping for 0 to 7
      if (n == i) {//making sure that i values are being read as n values
        validRange = true;//return true
        break;//get out of this loop once this is true
      }
    }
    if (validRange) {//if it is not true, then 
      return;
    }
    GPIOB_ODR &= ~0x00000700;//setting pins 8,9,10 for port B as defaults for ODR
    GPIOB_ODR |= (n << 8);//using bit masking, I made it so 
    GPIOB_ODR &= ~0x000000FF;//setting pins 0-7 for port B as defaults for ODR
    GPIOB_ODR |= font;//output this as the font 
}

/**
 * @brief Drive the column pins of the keypad
 *        First clear the keypad column output
 *        Then drive the column represented by `c`
 * 
 * @param c 
 */
void drive_column(int c) {
    GPIOC->BSRR = 0x000F0000;//setting pins 4-7 for port C as defaults for BSRR
    GPIOC->BSRR = 0x00000010 << c;//using bit masking, shift the bits c and set those bits to correspond to column c
}

/**
 * @brief Read the rows value of the keypad
 * 
 * @return int 
 */
int read_rows() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;//Enabling clock for port C
    int returnRow = GPIO->IDR & 0x0000000F;//read the state of the port and store that state in the returnRow variable
    return returnRow;//returns the value
}

/**
 * @brief Convert the pressed key to character
 *        Use the rows value and the current `col`
 *        being scanning to compute an offset into
 *        the character map array
 * 
 * @param rows 
 * @return char 
 */
char rows_to_key(int rows) {
    for (int i = 0; i < 4; i++) {//4 bit value
      if (rows & (1 << i)) {//if there is a row inputted from the user corresponding with the loop (from 0 to 3), then:
        rows = i;//consider that row pressed
      }
    }
    int offset = 4 * col + rows;//applying the necessary offset
    return keymap_arr[offset];//returning keymap array indexed by the offset
}

/**
 * @brief Handle key pressed in the game
 * 
 * @param key 
 */
void handle_key(char key) {
    if (key == 'A' | key == 'B' | key == 'D') {//if the key is A, B, or D, then:
      mode = key;//make the mode the key given by the user
    else if (key >= '0' && key <= '9') {//if key is a number, then:
      thrust = key - '0';//the -0 at the end will turn the character into an integer. This has to do with ASCII, where the key will use the ASCII value and subtract it by the ASCII value of 0. 
    }
    }
}

//-------------------------------
// Timer 7 ISR goes here
//-------------------------------
// TODO


/**
 * @brief Setup timer 7 as described in lab handout
 * 
 */
void setup_tim7() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;//Enabling TIM7 clock
    TIM7->PSC = 48;//PSC value
    TIM7->ARR = 999; //will go from 0 and count for 1000 ticks (so basically will stop at 999)
    TIM7->DIER |= TIM_DIER_UIE;//Enabling the UIE flag
    NVIC_EnableIRQ(TIM7_IRQn);//Enabling interrupt for timer 7
    TIM7->CR1 |= TIM_CR1_CEN;//Setting the CEN bit in the Timer 7 Control Register 1
}


/**
 * @brief Write the display based on game's mode
 * 
 */
void write_display() {
    if (mode == 'C') {
        snprintf(disp, sizeof(disp), "Crashed");//updated display to show crashed
    }
    else if (mode == 'L') {
        snprintf(disp, sizeof(disp), "Landed");//updated display to show landed
    }
    else if (mode == 'A') {
        snprintf(disp, sizeof(disp), "ALt%5d", alt);//updated display to show ALT
    }
    else if (mode == 'B') {
        snprintf(disp, sizeof(disp), "FUEL %3d", fuel);//updated display to show fuel
    }
    else if (mode == 'D') {
        snprintf(disp, sizeof(disp), "Spd %4d", velo);//updated display to show spd (velo)
    }
}

/**
 * @brief Game logic
 * 
 */
void update_variables() {
    fuel -= thrust;
    if (fuel <= 0) {
      thrust = 0;
      fuel = 0;
    }
    if (alt <= 0) {
      if (velo < 10) {
        mode = 'L';
      else {
        mode = 'C';
      }
      return;
      }
      velo += thrust - 5;
    }
}


//-------------------------------
// Timer 14 ISR goes here
//-------------------------------
// TODO


/**
 * @brief Setup timer 14 as described in lab
 *        handout
 * 
 */
void setup_tim14() {
  update_variables();
  write_display();
}
