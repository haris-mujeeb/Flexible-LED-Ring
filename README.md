# 🌈 Flexible-LED-Ring: Ultra-Low-Power (ULP) Control System

A highly energy-efficient controller built around the **STM32L011F4U6TR** microcontroller, designed for long-term battery operation. This project demonstrates advanced **Ultra-Low-Power (ULP)** techniques by driving a variable-frequency LED whose rate is controlled by an analog voltage input (ADC), minimising average current draw.

## ⚡ Core Low-Power Architecture

The system achieves microamp-level consumption by spending the maximum possible time in deep sleep modes, delegating all timing tasks to autonomous, low-frequency peripherals.

| Feature | Low-Power Implementation | Power Rationale |
| :--- | :--- | :--- |
| **System Clock** | **MSI @ 4.2 MHz (Scale 3)** | Runs the core voltage at the lowest possible level ($\approx 1.2\text{ V}$) for maximum efficiency in **Run Mode** and fast processing upon wake-up. |
| **Deep Sleep Mode** | **Stop Mode** (0.54 µA with RAM retention) | CPU is halted while the **LPTIM** continues to run autonomously, managing the blinking schedule. |
| **Variable Timing** | **LPTIM (Low Power Timer)** | Replaces high-consumption `HAL_Delay()` with a hardware timer clocked by the $\mathbf{LSI}$ (Low-Speed Internal oscillator). |
| **LED Duty Cycle** | Fixed **15ms Pulse** | The LED is only briefly turned ON, drastically reducing the single largest current draw source. |
| **I/O Gating** | **ADC\_EN (PA7)** control | External circuitry (e.g., analog front-end) is powered ON **only** during the brief ADC conversion window, minimising external leakage. |

## ✨ Functionality

### LED Control:
* **Variable Blink Rate:** The LED blink frequency is dynamically adjusted based on the **ADC value** (0-4095).
* **Low Duty Cycle:** A fixed $\mathbf{5\text{ ms}}$ pulse ensures energy efficiency across all blink frequencies.
* **Deep Sleep Toggle:** The single external switch (SW1/WKUP1) is used to manage the MCU's power state.
    * **First Press:** Activates the system (blinking begins/resumes).
    * **Second Press:** Deactivates blinking, putting the system into **Standby Mode** ($\approx 0.23\text{ µA}$) until the next press.

### 🔌 Charging Mechanism:
* ⚡ Utilises pogo pins for easy connection and charging.
* 💡 Charging LEDs illuminate when the pogo pins are connected and the battery is charging, turning off once fully charged.

## 🔬 Validation and Tools

The power consumption profile was rigorously measured using the **Nordic Power Profiler Kit II (PPK II)** to validate the efficacy of the **Stop Mode** and **Standby Mode** routines.

https://github.com/user-attachments/assets/664f258f-f0d6-4ddf-a007-7cb5bbc9106d


## 💻 Code:
Using ST-Link via Serial Wire debug mode, the program can be flashed onto the  highly energy-efficient STM32L011F4U6TR Microcontroller.

https://github.com/user-attachments/assets/1e935329-2498-4105-b1bf-f483a7316e35

## Assembly:
The PCB can be folded into a ring as shown below:

<img src="https://github.com/user-attachments/assets/227f697d-9f54-4604-9b52-a2f7d8a9d130" alt="Assembly_1" width="500"/>
**References:**
- For setting up SysTick properly: https://community.st.com/t5/stm32-mcus-embedded-software/i-know-sounds-dumb-but-how-do-you-configure-systick/td-p/377723
- For setting up External Interrupts: https://deepbluembedded.com/stm32-external-interrupt-example-lab/
- For setting up ADC: https://controllerstech.com/stm32-adc1-single-channel-polling-mode/
- For power analysis: https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2
- For low-power core configuration (Scale 3, Stop Mode): See [STM32L011x4 Datasheet](https://www.st.com/resource/en/datasheet/stm32l011f4.pdf) and [RM0377 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0377-ultralowpower-stm32l0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)


