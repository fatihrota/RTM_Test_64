################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/EcDemoApp.cpp \
../src/EcDemoMain.cpp \
../src/EcDemoParms.cpp \
../src/EcLogging.cpp \
../src/EcNotification.cpp \
../src/EcSdoServices.cpp \
../src/EcSelectLinkLayer.cpp \
../src/EcSlaveInfo.cpp \
../src/EcTimer.cpp \
../src/RTM_CompactController.cpp \
../src/RTM_MainApp.cpp \
../src/RTM_TestRun.cpp \
../src/RtosComm.cpp \
../src/RtosMessage.cpp 

OBJS += \
./src/EcDemoApp.o \
./src/EcDemoMain.o \
./src/EcDemoParms.o \
./src/EcLogging.o \
./src/EcNotification.o \
./src/EcSdoServices.o \
./src/EcSelectLinkLayer.o \
./src/EcSlaveInfo.o \
./src/EcTimer.o \
./src/RTM_CompactController.o \
./src/RTM_MainApp.o \
./src/RTM_TestRun.o \
./src/RtosComm.o \
./src/RtosMessage.o 

CPP_DEPS += \
./src/EcDemoApp.d \
./src/EcDemoMain.d \
./src/EcDemoParms.d \
./src/EcLogging.d \
./src/EcNotification.d \
./src/EcSdoServices.d \
./src/EcSelectLinkLayer.d \
./src/EcSlaveInfo.d \
./src/EcTimer.d \
./src/RTM_CompactController.d \
./src/RTM_MainApp.d \
./src/RTM_TestRun.d \
./src/RtosComm.d \
./src/RtosMessage.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	x86_64-pc-linux-gnu-g++ -I"C:\Users\fatih\OneDrive\Masa�st�\EC-Master-V3.1-EC-WinRT-Linux-x86_64Bit-Eval\SDK\INC\Linux" -I"C:\Users\fatih\OneDrive\Masa�st�\EC-Master-V3.1-EC-WinRT-Linux-x86_64Bit-Eval\SDK\Examples\Linux\EcMasterDemo" -I"C:\Users\fatih\OneDrive\Masa�st�\EC-Master-V3.1-EC-WinRT-Linux-x86_64Bit-Eval\SDK\INC" -I"C:\Users\fatih\OneDrive\Masa�st�\EC-Master-V3.1-EC-WinRT-Linux-x86_64Bit-Eval\SDK\INC\Linux" -I"C:\Program Files\acontis_technologies\LxWin\SDK\Examples\Inc\Linux" -I"C:\Program Files\acontis_technologies\LxWin\SDK\Inc" -I"C:\Program Files\acontis_technologies\LxWin\SDK\Inc\Linux" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


