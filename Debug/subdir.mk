################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GraphicsCore.cpp \
../InterfaceCore.cpp \
../Logic_Battle.cpp \
../Logic_StartScreen.cpp \
../main.cpp 

OBJS += \
./GraphicsCore.o \
./InterfaceCore.o \
./Logic_Battle.o \
./Logic_StartScreen.o \
./main.o 

CPP_DEPS += \
./GraphicsCore.d \
./InterfaceCore.d \
./Logic_Battle.d \
./Logic_StartScreen.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


