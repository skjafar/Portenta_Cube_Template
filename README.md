# Arduino Portenta Bare Metal Programming in STM32CubeIDE

## Intro
I was looking for a very powerful controller to use for some work related projects.
The Portenta was released in 2020 with not a lot of documentation at the beggining, but the high density connectors on the back were a great way to integrate this board into any PCB. But I do not beleive the Arduino platform is ideal for professional/industrial applications, and with this I began reverse engineering the PCB and all the initialization code it requires.

### What is this
An STM32CubeIDE project for the Arduino Portenta.
This porject is not optimized in terms of power, the goal here is maximum speed.
If you care for efficiency you will need to work some more on the code. check SystemClock_Config() in main.c in the M7 project.

### Feratures
* Running M7 Core at 480MHz, which is the maximum.
* Running M4 Core at 240MHz, which is the maximum.
* FreeRTOS v10.4.3 on the M7 core.
* FreeRTOS-TCP Ethernet stack ruinning on the M7 core, tested at 90Mbps using IPerf.
* Memory sharing example between cores, check shared_memory.c in both projects.
* Virtual Serial port implemented via the USB-C connector, can be run at almost any baudrate, 115200 is recommended.
* Telnet based Command Line Interface on port 2008.
* Tasks running the oboard RGB LED.

## Getting started
1. Download the project to your STM32CubeIDE workspace directory.
2. Go to [File] => Import:

![2021-12-14-184337_1906x1031_scrot1](https://user-images.githubusercontent.com/7383226/146091494-44419878-2078-4ca2-b530-502ce14fed97.png)

3. Select [Exesting Projects into Workspace].

![2021-12-14-184513_519x521_scrot](https://user-images.githubusercontent.com/7383226/146091617-ce6f3fe6-e28d-4745-8ecb-ba4f3a68515d.png)

4. Set the root directory to your Workspace folder and Select all 3 projects:
    * Portenta_Cube_Template.
    * Portenta_STM_Temaplate_CM4
    * Portenta_STM_Temaplate_CM7
    Make sure to select **Search for nested projects
     
![2021-12-14-184615_979x735_scrot](https://user-images.githubusercontent.com/7383226/146092033-cf786f9c-1f3d-4266-b99a-332bce17257c.png)

5. Finish. Now you should have one master project with 2 nested projects.

![2021-12-15-025300_1906x1031_scrot1](https://user-images.githubusercontent.com/7383226/146092267-195c7046-f69e-4175-8df7-6c1f00f82630.png)

## Flashing
The Portenta comes with a custom bootloader that we are going to replace by the procedure that follows.

**VERY IMPORTANT NOTE: AFTER THIS YOUR BOARD WILL NO LONGER WORK WITH THE ARDUINO IDE.**

We need to put the controller in DFU mode in order to flash it, there are two ways of acheiving this:
* Soldering a wire to the BOOT pin on the back of the board

   I don't really recommend this, the pad is absolutely tiny and you might damage the board in the process. But this is the method I had to use initially.
   After a wire was soldered I used a 3V lithium coin battery to set this pin high (in reference to ground of course) and then plugged in the USB cable, this put the controller in DFU mode and enabled me to program it normally.
   
   The yellow wire is soldered to the BOOT pad in the picture below.
   
  ![IMG_20211216_002013](https://user-images.githubusercontent.com/7383226/146274212-ca32e639-b9c9-4231-bfba-ef05b15946c6.jpg)
 
* Settig the BOOT Dip Switch on the Breakout Board

   There is a dual dip switch on the Breakout Board, one of those switches is labeled **BOOT**, when you set this one to ON then reset the board, your board will be in DFU mode and can be programed using your ST-LINK device.
   
   ![InkedIMG_20211216_002428_LI](https://user-images.githubusercontent.com/7383226/146274455-69720184-693a-4910-a472-e5c8985a98b5.jpg)

### JTAG / SWD
You will need a Debugging/Programming probe to program your controller, I have tested both the ST-LINK V3-MINI and the ST-LINK V3, and they both worked fine.

Unfortunately the Portenta board itself does not expose the SWD pins, you will need either the Breakout Board or the Vision Shield.

You can connect the 10 pin connector to the Vision Shield as shown below, it also takes the 20 pin connector.

![IMG_20211216_000918](https://user-images.githubusercontent.com/7383226/146273555-d65ff611-9928-40ba-8130-32899d3fb5fb.jpg)

As for the Breakout Board, this only takes the 20 pin connector as shown below

![IMG_20211216_001006](https://user-images.githubusercontent.com/7383226/146273598-3ecbef03-bdd0-401e-a4c1-40a74d6f0ddc.jpg)



## Building and running the code
The project comes predefined with 5 different run/debug configurations:
1. Portenta_STM_template_CM7 Debug

    This will compile both the CM4 and CM7 projects, then flash them both to the controller.
    As the STM32H747 is a dual core controller it needs two different projects, one for each core, this build configuration takes care of building and flashing both.
    This will then enter Debug mode for the M7 core only.
2. Portenta_STM_template_CM4 Debug

    After running the the Run configuration above you can use this to enter the Debug mode for the M4 core.
    This can be run only after the run configuration above is run.
3. Portenta_STM_template_CM7 Release

    Same as configuration 1 but with optimization level at -O3. No debugging available.
4. Portenta_STM_template_CM4 Release

    Same as configuration 2 but with optimization level at -O3. No debugging available.
5. Portenta_STM_template_CM7_DEBUG_NO_BUILD_NO_FLASH

    This was used to be able to debug an already running code, mostly not important.

## Interfaces
### Virual Com Port
As soon as the board is connected to a PC it populates a virtual com port that can be accessed by any Serial Com Port interface, such as Putty or Minicom.

**Configuration**
* Baudrate: 115200 [This can be changed, it automatically adapts]
* Parity:   1 bit
* Bits:     8 bits
* Flow:     None

This is what the screen will show once connected

![2021-12-14-185310_812x520_scrot](https://user-images.githubusercontent.com/7383226/146254288-5aa96e72-bf8c-4159-ba62-718cebc34230.png)

It is mostly self-explanatory. FreeRTOS Heap and Ethernet buffers are shown, as well as the uptime in seconds.

The **Task Statistics** shows how much CPU time each FreeRTOS task is consuming. This can be used for profiling tasks.

If an Ethernet cable is connected to the board through one of the expansion boards, such as the [Vision Sheild](https://www.arduino.cc/pro/hardware/product/portenta-vision-shield) or the [Breakout Board](https://www.arduino.cc/pro/hardware/product/portenta-breakout), **Phy Link Up** will be displayed along with the IP address the device received by your local DHCP server.

This interface is handled by two FreeRTOS tasks:
* USB Info
   This displays the information and statistics every second.
* USB CLI
   Handles the Command Line Interface over the serial link. Type **help** to see available commands.

### TCP based Command Line Interface
As soon as the device is connected to the local network it will start the task **TCP CLI** which takes care of this interface. You can connect to this using Putty or any other TCP telnet interface such as netcat.

![2021-12-14-185442_943x1023_scrot](https://user-images.githubusercontent.com/7383226/146257847-9616442d-49a8-4c97-a57d-c44291e2bb93.png)

**Configuration**
* IP Address: As shown in the Virtual Com Port Interface.
* TCP Port: 2008

You can type **help** to see available commands.

