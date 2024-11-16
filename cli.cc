#include "libs/base/gpio.h"
#include "libs/base/check.h"
#include "libs/base/i2c.h"
#include "libs/base/led.h"
#include "libs/base/tasks.h"
#include "libs/base/console_m7.h"
#include "libs/base/main_freertos_m7.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "FreeRTOS_CLI.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

coralmicro::I2cConfig config;

static char * pcWelcomeMessage =
  "Coral CLI 1.1. with i2c5 active and gpio, use help\r\n";


/* This function implements the behaviour of a command, so must have the correct
   prototype. */
static BaseType_t prvGpioCommand( char *pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char *pcCommandString )
{
const char *pcParameter1, *pcParameter2;
BaseType_t xParameter1StringLength, xParameter2StringLength, xResult;

    /* Obtain the name of the source file, and the length of its name, from
       the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );

    int ledNumber = -1;
    sscanf(pcParameter1, "%d", &ledNumber);
    int onOff = -1;
    if (strstr(pcParameter2, "on"))
        onOff = 1;
    else if (strstr(pcParameter2, "off"))
        onOff = 0;
    else if (strstr(pcParameter2, "get"))
        onOff = 2;

    int value = -1;

    if (ledNumber != -1 && onOff != -1)
    {
        xResult = true;
        if(onOff == 2)
            value = coralmicro::GpioGet((coralmicro::Gpio) ledNumber);    
        else if(onOff == 1)
            coralmicro::GpioSet((coralmicro::Gpio) ledNumber, 1);
        else if(onOff == 0)
            coralmicro::GpioSet((coralmicro::Gpio) ledNumber, 0);
        else
            xResult = false;

    }

    if (xResult == true)
    {
        /* The copy was successful. There is nothing to output. */
        if (onOff == 2)
            snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n>ack gpio value=0x%x\r\n", value);
        else
            snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n>ack gpio %s\r\n", pcParameter2);
    }
    else
    {
        /* The copy was not successful. Inform the users. */
        snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n#Error during gpio\r\n" );
    }

    /* There is only a single line of output produced in all cases. pdFALSE is
       returned because there is no more output to be generated. */
    return pdFALSE;
}


static const CLI_Command_Definition_t xGpioCommand =
{
    "gpio",
    "gpio <number> <on/off/get>: Sets GPIO number on on or off (23-AD26, 24-AD27)",
    prvGpioCommand,
    2
};


/* This function implements the behaviour of a command, so must have the correct
   prototype. */
static BaseType_t prvI2CwriteCommand( char *pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char *pcCommandString )
{
const char *pcParameter1, *pcParameter2, *pcParameter3;
BaseType_t xParameter1StringLength, xParameter2StringLength, xParameter3StringLength, xResult;

    /* Obtain the name of the source file, and the length of its name, from
       the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );
    pcParameter3 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             3,
                                             &xParameter3StringLength );

    uint8_t slave = -1;
    uint temp = -1;
    sscanf(pcParameter1, "%x", &temp);
    slave = (uint8_t)temp;
    temp = -1;
    uint16_t address = -1;
    sscanf(pcParameter2, "%x", &temp);
    address = (uint16_t)temp;
    temp = -1;
    uint8_t data = -1;
    sscanf(pcParameter3, "%x", &temp);
    data = (uint8_t)temp;

    if (slave != -1 && address != -1 && data != -1)
    {
        lpi2c_master_transfer_t transfer;
        transfer.flags = kLPI2C_TransferDefaultFlag;
        transfer.slaveAddress = slave;
        transfer.direction = kLPI2C_Write;
        transfer.subaddress = static_cast<uint16_t>(address);
        transfer.subaddressSize = sizeof(address);
        transfer.data = &data;
        transfer.dataSize = sizeof(data);
        xResult == LPI2C_RTOS_Transfer(I2C5Handle(), &transfer);
        
    }

    if (xResult == kStatus_Success)
    {
        /* The copy was successful. There is nothing to output. */
        snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n>ack 2write %x %x %x\r\n", slave, address, data);
    }
    else
    {
        /* The copy was not successful. Inform the users. */
        snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n#Error during 2write %x %x %x result: %d\r\n",  slave, address, data, xResult);
    }

    /* There is only a single line of output produced in all cases. pdFALSE is
       returned because there is no more output to be generated. */
    return pdFALSE;
}


static const CLI_Command_Definition_t xI2CwriteCommand =
{
    "2write",
    "2write <0x8slave> <0x16address> <0x8value>: writes 1 byte hexa to address at slave\r\n",
    prvI2CwriteCommand,
    3
};


/* This function implements the behaviour of a command, so must have the correct
   prototype. */
static BaseType_t prvI2CreadCommand( char *pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char *pcCommandString )
{
const char *pcParameter1, *pcParameter2;
BaseType_t xParameter1StringLength, xParameter2StringLength,  xResult;

    /* Obtain the name of the source file, and the length of its name, from
       the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );


    uint8_t slave = -1;
    uint temp = -1;
    sscanf(pcParameter1, "%x", &temp);
    slave = (uint8_t)temp;
    temp = -1;
    uint16_t address = -1;
    sscanf(pcParameter2, "%x", &temp);
    address = (uint16_t)temp;
    uint8_t data = -1;

    if (slave != -1 && address != -1)
    {
        lpi2c_master_transfer_t transfer;
        transfer.flags = kLPI2C_TransferDefaultFlag;
        transfer.slaveAddress = slave;
        transfer.direction = kLPI2C_Read;
        transfer.subaddress = address;
        transfer.subaddressSize = sizeof(address);
        transfer.data = &data;
        transfer.dataSize = sizeof(data);
        xResult == LPI2C_RTOS_Transfer(I2C5Handle(), &transfer);
       
    }

    if (xResult == kStatus_Success)
    {
        /* The copy was successful. There is nothing to output. */
        snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n>ack 2read 0x%x 0x%x value=0x%x\r\n", slave, address, data);
    }
    else
    {
        /* The copy was not successful. Inform the users. */
        snprintf( pcWriteBuffer, xWriteBufferLen, "\r\n#Error during 2read 0x%x 0x%x result: %d\r\n",  slave, address, xResult);
    }

    /* There is only a single line of output produced in all cases. pdFALSE is
       returned because there is no more output to be generated. */
    return pdFALSE;
}


static const CLI_Command_Definition_t xI2CreadCommand =
{
    "2read",
    "2read <0x8slave> <0x16address> : reads 1 byte hexa from address at slave\r\n",
    prvI2CreadCommand,
    2
};


[[noreturn]] void vCommandConsoleTask( void *pvParameters )
{
  char cRxedChar, cInputIndex = 0;
  BaseType_t xMoreDataToFollow;
  /* The input and output buffers are declared static to keep them off the stack. */
  static char pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];

    /* This code assumes the peripheral being used as the console has already
       been opened and configured, and is passed into the task as the task
       parameter. Cast the task parameter to the correct type. */

    /* Send a welcome message to the user knows they are connected. */
    coralmicro::ConsoleM7::GetSingleton()->Write(pcWelcomeMessage, strlen( pcWelcomeMessage ));

    //coralmicro::GpioInit();

    FreeRTOS_CLIRegisterCommand( &xGpioCommand );
    FreeRTOS_CLIRegisterCommand( &xI2CwriteCommand );
    FreeRTOS_CLIRegisterCommand( &xI2CreadCommand );

    for( ;; )
    {
          taskYIELD();

        /* This implementation reads a single character at a time. Wait in the
           Blocked state until a character is received. */
        int bytes = coralmicro::ConsoleM7::GetSingleton()->Read(&cRxedChar, 1);
        if (bytes != 1) 
          continue;

        coralmicro::ConsoleM7::GetSingleton()->Write(&cRxedChar, 1);


        if( cRxedChar == 13 )
        {
            /* A newline character was received, so the input command string is
               complete and can be processed. Transmit a line separator, just to
               make the output easier to read. */
            coralmicro::ConsoleM7::GetSingleton()->Write("\r", 1);

            /* The command interpreter is called repeatedly until it returns
               pdFALSE. See the "Implementing a command" documentation for an
               exaplanation of why this is. */
            do
            {
                /* Send the command string to the command interpreter. Any
                   output generated by the command interpreter will be placed in the
                   pcOutputString buffer. */
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                              (
                                  pcInputString,   /* The command string.*/
                                  pcOutputString,  /* The output buffer. */
                                  MAX_OUTPUT_LENGTH/* The size of the output buffer. */
                              );

                /* Write the output generated by the command interpreter to the
                   console. */
                coralmicro::ConsoleM7::GetSingleton()->Write(pcOutputString, strlen( pcOutputString ));
                   

            } while( xMoreDataToFollow != pdFALSE );

            /* All the strings generated by the input command have been sent.
               Processing of the command is complete. Clear the input string ready
               to receive the next command. */
            cInputIndex = 0;
            memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
        }
        else
        {
            /* The if() clause performs the processing after a newline character
               is received. This else clause performs the processing if any other
               character is received. */

            if( cRxedChar == '\r' )
            {
                /* Ignore carriage returns. */
            }
            else if( cRxedChar == '\b' )
            {
                /* Backspace was pressed. Erase the last character in the input
                   buffer - if there are any. */
                if( cInputIndex > 0 )
                {
                    cInputIndex--;
                    pcInputString[ cInputIndex ] = '_';
                }
            }
            else
            {
                /* A character was entered. It was not a new line, backspace
                   or carriage return, so it is accepted as part of the input and
                   placed into the input buffer. When a n is entered the complete
                   string will be passed to the command interpreter. */
                if( cInputIndex < MAX_INPUT_LENGTH )
                {
                    pcInputString[ cInputIndex ] = cRxedChar;
                    cInputIndex++;
                }
            }
        }
    }
}

