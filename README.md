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

## 📊 Schematic:
![grafik](https://github.com/user-attachments/assets/20555b76-adcb-4200-9d99-c2b4ca056a9e)

## 🛠️ PCB Layout:
![grafik](https://github.com/user-attachments/assets/004e336d-5b91-43aa-951f-7701e4d78efe)

## 💻 Code:
Using ST-Link via Serial Wire debug mode, the program can be flashed onto the STM32G031J6M6 Microcontroller.

https://github.com/user-attachments/assets/1e935329-2498-4105-b1bf-f483a7316e35

## Assembly:
The PCB can be folded into a ring as shown below:

<img src="https://github.com/user-attachments/assets/227f697d-9f54-4604-9b52-a2f7d8a9d130" alt="Assembly_1" width="500"/>
