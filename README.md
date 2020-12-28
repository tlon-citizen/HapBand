# HapBand
<b>Wearable device to provide multimodal feedback</b>

HapBand is a wearable and wireless device composed of two modules to be worn in the upper arm. The modules are driven by an Espressif ESP32 dual-core microprocessor clocked at 240 Mhz with 520Kb RAM and powered by rechargeable 18650HG2 Li-Ion batteries (3.7V).
The modules-to-PC latency is approximately 30 ms.
Actuators and electrodes are connected to the modules with thin cables that can be comfortably accommodated along the arm.
The tactile module (116mm x 65mm x 15mm) provides tapping and vibration stimuli through a wearable fingertip (64mm x 32mm x 32mm).
Additionally, there is a TES/EMS module (116mm x 52mm x 25mm) providing tendon and muscle electrical stimulation. This module uses an off-the-shelf transcutaneous nerve stimulator (Schwa Medico Medizintechnik Art.-Nr. 104099-V08), the PCB and the firmware are based on the openEMSstim project by Pedro Lopes and Max Pfeiffer[1,2]. 
The TES/EMS module supports two channels (i.e., two muscles or tendons can be stimulated independently by two pairs of electrodes connected from the module to the selected arm muscles). For every channel, experiment operators can set the intensity and the duration of the electrical pulse.
The device case was 3d-printed with resin material and the total weight is 31 grams. 
The fingertip wearable is mechanically connected by electromagnetic solenoids. The top push-pull solenoid provides a tapping sensation on the fingertip and the front solenoid provides soft touches on the finger pulp. The vibration sensations on the fingertip are provided with a piezo actuator measuring 35mm x 4mm x 4mm (Samsung PHAH353832XX, 120Vpp AC, sine wave @230Hz) which is controlled by a haptic driver (Texas Instruments DRV2667 by the I2C protocol). 
The piezo element touches the fingertip whenever the top solenoid is closed.

<p align="center"><img src="Media/device.png" width="50%"></p>
<p align="center"><img src="Media/Modules.png" width="50%"></p>

This repository contains the necessary files to build a new device; model files for 3D printing, schematics for electronics, and embedded/testing code.

<p align="center"><img src="Media/parts.png" width="50%"></p>

A Unity plugin manages all the sensors and actuators connected to the HapBand modules. In addition to UI for controlling/enabling sensations, the plugin also presents real-time information regarding the wireless connection, as well as, debugging information from pressure sensors (up to two) and capacitive pins (up to 8, used for input interaction in VEs). The plugin also manages the wireless connection with the TES/EMS module and safely keeps the parameters under the clinical limits established by the analog nerve stimulator.

<p align="center"><img src="Media/unity.png" width="50%"></p>

[1]https://github.com/PedroLopes/openEMSstim/<br>
[2]https://bitbucket.org/MaxPfeiffer/letyourbodymove/wiki/Home



