# 🌈 Flexible-LED-Ring

A portable battery-powered LED Ring based on STM32.


## ✨ Features
### LED Control:
    🔘 First Button Press: Activates main LEDs to blink.
    🔘 Second Button Press: Deactivates the blinking function.
    ⚠️ Low Battery Indicator: When battery level is low, the blinking frequency reduces to signal low power.

### 🔌 Charging Mechanism:
    ⚡ Utilizes pogo pins for easy connection and charging.
    💡 Charging LEDs illuminate when the pogo pins are connected and the battery is charging, turning off once fully charged.


https://github.com/user-attachments/assets/664f258f-f0d6-4ddf-a007-7cb5bbc9106d


## 📊 Schematic:
![grafik](https://github.com/user-attachments/assets/20555b76-adcb-4200-9d99-c2b4ca056a9e)

## 🛠️ PCB Layout:
![grafik](https://github.com/user-attachments/assets/004e336d-5b91-43aa-951f-7701e4d78efe)

## 💻 Code:
Using ST-Link via Serial Wire debug mode, the program can be flashed onto the STM32G031J6M6 Microcontroller.

https://github.com/user-attachments/assets/1e935329-2498-4105-b1bf-f483a7316e35

**References:**
- For setting up SysTick properly: https://community.st.com/t5/stm32-mcus-embedded-software/i-know-sounds-dumb-but-how-do-you-configure-systick/td-p/377723
- For setting up External Interrupts: https://deepbluembedded.com/stm32-external-interrupt-example-lab/
- For setting up ADC: https://controllerstech.com/stm32-adc1-single-channel-polling-mode/

## Assembly:
The PCB can be folded into a ring as shown below:

<img src="https://github.com/user-attachments/assets/227f697d-9f54-4604-9b52-a2f7d8a9d130" alt="Assembly_1" width="500"/>
