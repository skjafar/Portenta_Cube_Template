# Arduino Portenta Bare Metal Programming in STM32CubeIDE

## Intro
I was looking for a very powerful controller to use for some work related projects.
The Portenta was released in 2020 with not a lot of documentation at the beggining, but the high density connectors on the back were a great way to integrate this board into any PCB. But I wanted to program it using STM32CubeIDE, and with this I began reverse engineering the PCB and all the initialization code it requires.

### What is this
An STM32CubeIDE project for the Arduino Portenta.
This porject is not optimized in terms of power, the goal here is maximum speed.
If you care for efficiency you will need to work some more on the code. check SystemClock_Config() in main.c in the M7 project.

### Features
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

If you want to return the board to its original state you will have to flash the [ArduinoCore-mbed bootloader](https://github.com/arduino/ArduinoCore-mbed/tree/master/bootloaders/PORTENTA_H7). I tested the [**portentah7_bootloader_mbed_hs_v2.bin**](https://github.com/arduino/ArduinoCore-mbed/blob/master/bootloaders/PORTENTA_H7/portentah7_bootloader_mbed_hs_v2.bin) and it worked fine.

You should be able to directly flash the board without any problems, the SWD pins seem to be defined in the Arduino bootloader. Should you have any problems I recommend putting the controller in DFU mode, there are two ways of acheiving that:
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

## STM32Cube Device Configuration Tool
When you double click the .ioc file the Device Configuration Tool tab will open, this enables you to easily configure your board and the libraries that will be added.

One important thing to note, I have tried to make the code as stable as possible, minimizng the effect of a modificaiton by this tool, but I was not 100% successful.

You will have to check what files have changed and if you expected those files to change, I use the Source Control tool provided by VSCode, it shows all the moidifications that have been applied on the files.

![VSCode_GitDiff](https://user-images.githubusercontent.com/7383226/146281019-febd050b-58e0-41fe-87bd-9bd4a26b275a.PNG)

You will have to do the following to have your program working properly after a modification by this tool:
1. Remove lines 19-46 from CM7/Core/Src/eth.c
   ```C
   /* USER CODE END Header */
   /* Includes ------------------------------------------------------------------*/
   #include "eth.h"

   #if defined ( __ICCARM__ ) /*!< IAR Compiler */

   #pragma location=0x30040000
   ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
   #pragma location=0x300400C0
   ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
   #pragma location=0x300402F0
   uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffers */

   #elif defined ( __CC_ARM )  /* MDK ARM Compiler */

   __attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
   __attribute__((at(0x300400C0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
   __attribute__((at(0x300402F0))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffer */

   #elif defined ( __GNUC__ ) /* GNU Compiler */

   ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
   ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
   uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE] __attribute__((section(".RxArraySection"))); /* Ethernet Receive Buffers */

   #endif

   ETH_TxPacketConfig TxConfig;
   ```
1. Remove lines 53-98 from CM7/Core/Src/eth.c
   ```C
   ETH_HandleTypeDef heth;

   /* ETH init function */
   void MX_ETH_Init(void)
   {

     /* USER CODE BEGIN ETH_Init 0 */

     /* USER CODE END ETH_Init 0 */

      static uint8_t MACAddr[6];

     /* USER CODE BEGIN ETH_Init 1 */

     /* USER CODE END ETH_Init 1 */
     heth.Instance = ETH;
     MACAddr[0] = 0x00;
     MACAddr[1] = 0x80;
     MACAddr[2] = 0xE1;
     MACAddr[3] = 0x00;
     MACAddr[4] = 0x00;
     MACAddr[5] = 0x00;
     heth.Init.MACAddr = &MACAddr[0];
     heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
     heth.Init.TxDesc = DMATxDscrTab;
     heth.Init.RxDesc = DMARxDscrTab;
     heth.Init.RxBuffLen = 1524;

     /* USER CODE BEGIN MACADDRESS */

     /* USER CODE END MACADDRESS */

     if (HAL_ETH_Init(&heth) != HAL_OK)
     {
       Error_Handler();
     }

     memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
     TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
     TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
     TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
     /* USER CODE BEGIN ETH_Init 2 */

     /* USER CODE END ETH_Init 2 */

   }
   ```
3. Remove the first call for **MX_USB_DEVICE_Init()** in CM7/Core/Src/main.c
   ```C
   /* Initialize all configured peripherals */
   MX_GPIO_Init();
   MX_I2C1_Init();
   MX_TIM2_Init();
   MX_USB_DEVICE_Init();
   /* USER CODE BEGIN 2 */
   ```
   It is called again after properly setting up the USB hardware.
4. The Configuration Tool decides to remove the linked files for some reason.
   This is the most frustrating problem in this procedure, I am not able to keep the files that I add to each project stay.
   Of course this is usually not a problem, but since this is a nested project it shares the STM_HAL_Drivers, there is only one copy of those files and it located at the base folder not inside the nested folders. You need to link these files to each project to get it to work. The Configuration Tool decides to remove these links every time it is used.
   
   Here is how I do it, if you know a better way please let me know:
   * Right click on **Drivers** folder in the base project, **Show In -> System Explorer**.

      ![Readding_Links1](https://user-images.githubusercontent.com/7383226/146341539-50b77319-95b8-49ae-9fb7-1022a461321d.png)
   
      This will open the folder in windows explorer.
   * Drag the folder named **STM32H7xx_HAL_Driver** to the **Drivers** folder inside the **portenta_STM_template_CM4** Project.
   
      ![Readding_Links2](https://user-images.githubusercontent.com/7383226/146342134-5b3f0aeb-a677-4246-a359-e202e79fef00.png)
   
      Then select **Link to files and recreate folder structure with virtual folders**, keep the links relative to the **PROJECT_LOC**.
   
      ![Readding_Links3](https://user-images.githubusercontent.com/7383226/146342448-41d8cfd0-95d5-4b92-a4b0-461a9bd74148.png)
   
   * Then delete the **Inc** folder and the **License** file. Make sure they have the small squares next to them, indicating that they are just links not the actual files. This step is optional for a cleaner project.
   
      ![Readding_Links4](https://user-images.githubusercontent.com/7383226/146343104-805b5ba6-1dd2-481f-9a4f-0032cad3f32d.png)

## Configuration Files
The project has some configuration files for setup.

* Common/header/application_config.h
   Contains some helping macros as well as the configuration parameters for the FreeRTOS tasks, such as their stack sizes and priorities.
* CM7/Core/Inc/FreeRTOSConfig.h
   Contains the FreeRTOS configuiation. Consult [FreeRTOS Documentation](https://www.freertos.org/a00110.html) for more information.
* CM7/Application/header/FreeRTOSIPConfig.h
   Contains all the configuration for the FreeRTOS TCP stack. Consult [FreeRTOS-TCP Documentation](https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html) for more information.
