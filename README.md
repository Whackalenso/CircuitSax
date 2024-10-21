# Circuit Sax

The Circuit Sax is a saxophone MIDI controller. It lets you play any software instrument you’d like using saxophone fingerings, allowing you to practice without disturbing others, create music in any DAW, and even perform with it. It’s designed to feel as close to a regular saxophone as possible so that it won’t feel unfamiliar to the player.

### Features

- Breath input to trigger/release notes and control the **Mod Wheel** to map to any synth parameter
- Mechanical key switches with custom keycaps meant to mimic the feel of a saxophone
- MIDI output via USB to a computer or phone

### Components

- **MPXV5010GP** - Pressure sensor for breath input
- **Haimu Heartbeat Silent Linear Switches** - Mechanical switches for the keys
- **Arduino Micro** - Microcontroller board for sending MIDI output
- **Custom 3D Prints** - For the mouthpiece, thumb rests, and keys
- **Custom PCB** - Circuit board used as the body

## The Whole Process

- [Overall Design](#overall-design)
- [Choosing Components](#choosing-components)
- [Designing the models](#designing-the-models)
- [Electrical Design](#designing-the-pcb)
- [Assembly](#assembly)
- [Programming](#programming)

### Overall Design

After originally making a prototype out of breadboards, I wanted to make a new version using a PCB as the body, mechanical switches for buttons, and 3D prints for the mouthpiece, thumb rest, and keys. However, I wasn't sure what I would do about the side keys. After finding [this project](https://github.com/cardonabits/haxo-hw) by Javier Cardona, I decided I would do something similar and attach additional smaller PCBs to the sides with angle brackets and electrical header pins.

In contrast to the [Haxophone](https://github.com/cardonabits/haxo-hw), I wanted to position the keys to be more accurate. In CAD, I positioned all the keys and angled the side PCBs based on measurements from my soprano sax. In order to make sure everything would fit properly, I spent months modeling the entire thing, including mechanical switches, 3D prints, brackets, nuts, bolts, and electrical components.

### Choosing Components

- For the breath sensor, I decided to use a [MPXV5010GP pressure sensor](https://www.digikey.com/en/products/detail/nxp-usa-inc/MPXV5010GP/951847?s=N4IgTCBcDaILYAcAeA3ArABgIwYOYJAF0BfIA) since that was what many other similar projects use, so I assumed it would do what I needed.
- For the microcontroller, I decided to use an [Arduino Micro](https://store.arduino.cc/products/arduino-micro) because I already had one and its atmega32u4 chip would work for sending USB output. I would attach it to the PCB with header pins.
- For the mechanical switches, I decided to use [Haimu Heartbeat Silent Linear Switches](https://cannonkeys.com/products/haimu-heartbeat-silent-linear-switch?srsltid=AfmBOoo-FxAV9BJslixNNQ3mZ6u7veaNx8jfjfWYgJqzr-3P1V7ygZ23) because silent switches wouldn't disturb others, and the heavier spring weight would be more similar to a real saxophone.
- For the electrical connections to other PCBs (for the side keys), I got various header pins and sockets that I would have to bend in specific angles in order to fit properly. I figured that if they broke, I would just use wires as a plan B.

### Designing the Models

Since I didn't have access to a quality 3D printer, I decided to use [JLCPCB's 3D printing service](https://jlc3dp.com/?source=JLCPCB-top-productbar) and get my prints online. This meant I would have to be very thorough in making sure each part would work before ordering them.
I modeled the mouthpiece, thumb rest, and all the keycaps in the same Fusion 360 project as the entire thing so that everything would align properly and I would get a visual sense of what it would look like.

#### Mouthpiece / Left Thumb Rest

For the mouthpiece, I had to come up with a design that would allow enough pressure to reach the sensor while also making sure that it wouldn't get exposed to moisture. In other EWI designs I researched, both homemade and professional, it seemed as though the airflow split at the mouth into two equal channels, one for the breath sensor and one for moisture drainage and airflow. I was concerned that moisture would reach the sensor, however after reading somewhere that most of the moisture comes from condensation and condensation only happens in the tube that doesn't have air flow through it, I decided to use this design. _(Turns out I was wrong, because some moisture does build up in the sensor tube. I will want to use a different design in the future.)_

Since having two separate parts for the neck and left thumb rest would create unnecessary distance between them, I designed the mouthpiece, neck, and left thumb rest to all be the same part. Both air channels would flow through the thumb rest. The sensor channel would be aligned with the pressure sensor so that it can be connected directly with a short vinyl tube.

#### Right Thumb Rest

I designed the right thumb rest to also act as an extension for the drainage tube. A channel goes through the thumb rest and extends into a tube which ends at the very bottom of the board. A vinyl tube would connect this channel with the drainage channel of the left thumb rest.

I used form modeling in Fusion 360 to make this thumb rest as realistically smooth as possible.

In addition, notches on the bottom of both thumb rests allow space for electrical header sockets and the bottom the mechanical switches on the other side of the board.

#### Keycaps

In order for the keycaps to properly fit the stems of the switches, I needed to order a few tests prints with varied size tolerances to see what would fit best.

I designed each key to have as similar of a shape as possible to a real sax. Some details include the concavity on the primary keys, the lip on the top side key, and the different angles of the palm keys. Every detail was carefully measured or eyeballed on my real sax.

Some keys, such as the G sharp key and bis key have tabs so that when another key is pressed next to it, it will also get pressed. This has no function, but makes it feel more realistic.

On some keys, the stem (where it connects to the switch) is offset so that the key can be in the ideal position.

### Electrical Design

The electrical design is very simple since I'm using an Arduino. The Arduino Micro is plugged into 2 rows of header sockets on the board. Each switch is connected to a digital pin on the Arduino and ground. Switches on separate board are connected to main board with header pins, and then connected to the Arduino. The MPXV5010GP pressure sensor is connected to 5 volts on the Arduino with 2 decoupling capacitors, an analog pin on the arduino with 1 decoupling capacitor for the data output, and ground.

I designed the circuit board in [KiCad](https://www.kicad.org/), properly positioning every component and sizing the board according to my CAD model.

#### Circuit Board Aesthetics

Since the circuit board was going to be the body of the device, I wanted it to look good. I neatly aligned the traces, rounded the corners, made a logo, and ordered it in yellow.

<!-- ### Assembly

After the PCB and electrical components had arrived, it was time to get building. Assembly consisted of:

* Soldering all electrical components on, such as the breath sensor, small SMD capacitors, headers, and mechanical switches.
* Bending brackets  -->
