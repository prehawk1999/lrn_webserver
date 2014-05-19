################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../HttpRequest.cpp \
../Request.cpp \
../heaptimer.cpp \
../main.cpp \
../utils.cpp 

OBJS += \
./HttpRequest.o \
./Request.o \
./heaptimer.o \
./main.o \
./utils.o 

CPP_DEPS += \
./HttpRequest.d \
./Request.d \
./heaptimer.d \
./main.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


