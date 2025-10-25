#include <stdint.h>

/// --- Base addresses ---
#define PERIPHERAL_BASE       	0x40000000
#define AHB1_PERIPHERAL_BASE    (PERIPHERAL_BASE + 0x20000)
#define RCC_BASE         		(AHB1_PERIPHERAL_BASE + 0x3800)
#define GPIOE_BASE       		(AHB1_PERIPHERAL_BASE + 0x1000)
#define GPIOD_BASE       		(AHB1_PERIPHERAL_BASE + 0x0C00)

// --- Registers ---
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30))  // AHB1 enable (GPIO clocks)
#define GPIOE_MODER   (*(volatile uint32_t *)(GPIOE_BASE + 0x00))  // Mode
#define GPIOE_IDR     (*(volatile uint32_t *)(GPIOE_BASE + 0x10)) // Input data
#define GPIOE_PUPDR   (*(volatile uint32_t *)(GPIOE_BASE + 0x0C))	// Push-up/pull-down
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00))  // Mode
#define GPIOD_OTYPER  (*(volatile uint32_t *)(GPIOD_BASE + 0x04))	// Open drain
#define GPIOD_BSRR    (*(volatile uint32_t *)(GPIOD_BASE + 0x18))  // Bit set/reset (atomic)

// --- LED on PD13 ---
#define LED_PIN       13
#define LED_SET       (1 << LED_PIN)	// BS
#define LED_RESET     (1 << (LED_PIN + 16))	// BR

#define BUTTON_PIN    15	// PE15

// --- Crude delay loops (tune as needed) ---
// Start values assuming ~16 MHz HSI and no compiler optimizations.
#define DELAY   50000


int main(void)
{
	// Enable GPIOE + GPIOD clocks
	RCC_AHB1ENR |= (1 << 4) | (1 << 3);

	// Configure PD13 as general-purpose output, PUSH-PULL
	GPIOD_MODER &= ~(3 << (LED_PIN * 2));	// Clear mode
	GPIOD_MODER |= (1 << (LED_PIN * 2));	// 01 = output
	//GPIOD_OTYPER &= ~(1 << LED_PIN);	// 0 = push-pull (NOT open-drain for on-board LED)
	GPIOD_OTYPER |=  (1 << LED_PIN);   // open-drain (pin sinks current for ON, used for common anode)

	// Configure PE15 as input with internal PULL-DOWN (unpressed=0, pressed=1)
	GPIOE_MODER &= ~(3 << (BUTTON_PIN * 2));	// 00 = input
	GPIOE_PUPDR &= ~(3 << (BUTTON_PIN * 2));	// Clear pull configuration
	//GPIOE_PUPDR |= (2 << (BUTTON_PIN * 2));	// 10 = pull-down (define 0 for unpressed)
	GPIOE_PUPDR |= (1 << (BUTTON_PIN * 2));	// 01 = pull-up

	// Initial states
	GPIOD_BSRR = LED_RESET;	// LED off to start (PD13 = 0)
	uint32_t last_buton_state = (GPIOE_IDR & 1 << BUTTON_PIN) ? 1 : 0;
	uint32_t led = 0;

	while (1)
	{
		// Read button (0 = not pressed, 1 = pressed)
	    uint32_t current_button_state = (GPIOE_IDR & 1 << BUTTON_PIN) ? 1 : 0;

	    // Rising-edge detect: toggle LED on each press
	    if (current_button_state && !last_buton_state)
	    {
	    	if (led)
	    	{
	    		GPIOD_BSRR = LED_RESET;  // PD13 = 0 -> LED off (active-high)
	    		led = 0;
	    	}
	    	else
	    	{
				GPIOD_BSRR = LED_SET;	// PD13 = 1 -> LED on
				led = 1;
	    	}
	    }

	    last_buton_state = current_button_state;

	   // Crude debounce
	   for (volatile uint32_t i = 0; i < DELAY; i++) ;
	}
}
