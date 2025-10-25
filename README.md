# Digital Display Segment Toggle

## Overview

This project demonstrates how to toggle a **single segment** of a common‑anode 7‑segment LED module using an external button on the **STM32F411E-Discovery** development board. The display used is the **ELS‑516SURWA/S530‑A2** common‑anode 7‑segment unit. The goal is to sink current through one segment of the display when the button is pressed, and switch it off on the next press, effectively toggling that segment on and off.

Although a 7‑segment module normally drives all seven segments, this example focuses on controlling **one segment** (wired to **PD13**) to illustrate how to use an external push‑button to toggle a single LED segment.

## Hardware Setup

- **STM32F411E-Discovery board**.
- **7‑segment display ELS‑516SURWA/S530‑A2** (common anode).
- **Button input:** External push‑button connected to **PE15** (with one side tied to ground or VCC depending on the chosen internal pull configuration).
- **Segment connection:** One segment of the 7‑segment module connected to **PD13** through a current‑limiting resistor (220–330 Ω recommended).
- The common anode pin of the display is connected to **3.3 V**.

### Wiring Notes

- **PD13** is configured as an **open‑drain output**. In open‑drain mode, driving the pin low sinks current through the segment (turning it on), while releasing (logic high) leaves the pin floating so the segment is off. This suits a common‑anode display, where the anode is tied to 3.3 V.
- **PE15** is used as a **digital input**. The internal pull resistor (pull‑up or pull‑down) can be chosen in code. In this example it uses an internal **pull‑up**, so the button must connect PE15 to ground when pressed. The commented code shows how to switch to a pull‑down instead if your button wiring is different.
- Use a separate **current‑limiting resistor** between PD13 and the segment cathode to limit the LED current.

## Software Design

The firmware is contained in `Src/main.c` and uses **direct register access** instead of HAL/LL libraries. Below is a high‑level explanation of each part of the code.

1. **Base address and register definitions** – The code defines pointers to the RCC and GPIO registers for GPIOE and GPIOD. This lets the program enable peripheral clocks and configure modes without additional libraries.

2. **Clock enabling** – In `main()`, the AHB1 clock is enabled for **GPIOE** (bit 4) and **GPIOD** (bit 3). Without this, the GPIO modules remain disabled.

3. **Configuring PD13**  
   - **Mode:** PD13 is set as a general‑purpose output.  
   - **Type:** The code uses **open‑drain** for PD13 (`GPIOD_OTYPER |= (1 << LED_PIN)`), which means the pin can only pull low; pulling high leaves the output floating. This is ideal for sinking current through a common‑anode display segment.  
   - **Initial state:** PD13 is cleared using the **bit set/reset register** (BSRR) so the LED is initially off.

4. **Configuring PE15 (button)**  
   - PE15 is configured as an input.  
   - The code enables an **internal pull‑up** (`GPIOE_PUPDR |= (1 << (BUTTON_PIN * 2))`), so the input reads ‘1’ when the button is unpressed and ‘0’ when pressed (if the button connects the pin to ground).  
   - A variable `last_button_state` stores the previous state to detect **rising edges** (button press events).

5. **Main loop and button handling**  
   - The loop reads the current button state (`current_button_state = (GPIOE_IDR & (1 << BUTTON_PIN)) ? 1 : 0`).  
   - It detects a **transition from 0 to 1** (button press) by comparing `current_button_state` with `last_button_state`.  
   - On each detected press, it toggles the `led` variable and uses the BSRR to either set (`LED_SET`) or reset (`LED_RESET`) PD13, turning the segment on or off.  
   - A crude delay loop (`for (volatile uint32_t i = 0; i < DELAY; i++);`) provides **debouncing**, preventing false triggers due to button bounce.

## Modes and Commented Code

Inside the configuration section of `main.c`, there are commented lines showing an **alternative configuration**:

```c
//GPIOD_OTYPER &= ~(1 << LED_PIN);      // 0 = push-pull (NOT open-drain for on-board LED)
//GPIOE_PUPDR |= (2 << (BUTTON_PIN * 2)); // 10 = pull-down (define 0 for unpressed)
```

- **Open-Drain vs Push-Pull:**  
  - The example uses **open-drain** because the segment is connected to a **common-anode display**; it must sink current to light the LED.  
  - If you were driving the **on-board LED** on PD13 instead (which is wired as active-high), you would enable **push-pull** output by clearing the OTYPER bit (as shown in the commented line) so the pin can drive both high and low.  

- **Pull-Up vs Pull-Down:**  
  - With a **pull-up** configuration (as used by default), the button must connect PE15 to ground to register a press (logic low → high transition).  
  - The commented line shows how to configure a **pull-down** resistor instead. In that case, the button should connect PE15 to 3.3 V when pressed. Choose whichever setup matches your wiring.

By switching between the commented and active lines, you can easily adapt the code to use either the on-board LED (push-pull & pull-down) or an external common-anode segment (open-drain & pull-up).

## Conclusion

This project illustrates how to interface a common-anode 7-segment display with the STM32F411 using **direct register manipulation** and how to toggle a single segment with a button press. The commented lines in `main.c` show how to change the configuration between an **open-drain output with pull-up** (suitable for external common-anode displays) and a **push-pull output with pull-down**.
