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
5. Portenta_STM_template_CM4_DEBUG_NO_BUILD_NO_FLASH

    This was used to be able to debug and already running code, mostly not important.
    
