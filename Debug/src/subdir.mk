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
../src/EcTimer.cpp 

OBJS += \
./src/EcDemoApp.o \
./src/EcDemoMain.o \
./src/EcDemoParms.o \
./src/EcLogging.o \
./src/EcNotification.o \
./src/EcSdoServices.o \
./src/EcSelectLinkLayer.o \
./src/EcSlaveInfo.o \
./src/EcTimer.o 

CPP_DEPS += \
./src/EcDemoApp.d \
./src/EcDemoMain.d \
./src/EcDemoParms.d \
./src/EcLogging.d \
./src/EcNotification.d \
./src/EcSdoServices.d \
./src/EcSelectLinkLayer.d \
./src/EcSlaveInfo.d \
./src/EcTimer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	i686-pc-linux-gnu-g++ -I"C:\Program Files\acontis_technologies\LxWin\SDK\Examples\Inc\Linux" -I"C:\Program Files\acontis_technologies\EC-Master_ECWinRT-Linux\SDK\INC\Linux" -I"C:\Program Files\acontis_technologies\EC-Master_ECWinRT-Linux\Sources\Common" -I"C:\Program Files\acontis_technologies\EC-Master_ECWinRT-Linux\SDK\INC" -I"C:\Program Files\acontis_technologies\LxWin\SDK\Inc" -I"C:\Program Files\acontis_technologies\LxWin\SDK\Inc\Linux" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


