#include <Arduino.h>
#include "UartCommandParser.h"

// Uncomment define below to enable debug logging in this file.
// #define LOGGER Serial
#include "Logger.h"

//////////////////////////////////////////
// Command descriptions
//////////////////////////////////////////
namespace UartCommand {
    const Parser::CommandDescription Parser::nullCommand = {0, ID::none, Parser::ParamType::string, 0};

    const Parser::CommandDescription Parser::bluefruitCommands[] = {
        // Color input
        {'C', ID::color, ParamType::color, 4},

        // Button event
        {'B', ID::buttonEvent, ParamType::buttonEvent, 3},
    };   

    const int Parser::bluefruitCommandsCount = sizeof(bluefruitCommands) / sizeof(bluefruitCommands[0]);
}

//////////////////////////////////////////
// Parser class
//////////////////////////////////////////
namespace UartCommand {
    void Parser::reset() {
        rxMethod = &Parser::rxWaitForPrefix;
        
        rxPos = 0;
        memset(paramBuffer, 0, paramBufferSize);

        currentCommand = nullCommand;
    }

    bool Parser::isIdle() const {
        return rxMethod == &Parser::rxWaitForPrefix;
    }

    //////////////////////////////////////////
    // RX
    //////////////////////////////////////////
    void Parser::rxWaitForPrefix(char rxByte) {
        switch (rxByte) {
            case char(CmdType::bluefruit):
                currentCommandType = CmdType(rxByte);
                rxMethod = &Parser::rxWaitForCode;
                break;

            case char(CmdType::text):
                currentCommandType = CmdType(rxByte);
                rxMethod = &Parser::rxReadText;
                break;

            default:
                executeError("Unrecognized UART command type");
                break;
        };
    }
    
    void Parser::rxWaitForCode(char rxByte) {
        switch (currentCommandType) {
            case CmdType::bluefruit: {
                int matchingIndex = -1;
                
                for (int i = 0; i < bluefruitCommandsCount; i++) {
                    if (bluefruitCommands[i].code == rxByte) {
                        matchingIndex = i;
                        break;
                    }
                }

                if (matchingIndex != -1) {
                    currentCommand = bluefruitCommands[matchingIndex];
                    rxMethod = &Parser::rxReadBluefruit;
                }
                else {
                    // We don't recognize this command
                    executeError("Unsupported Bluefruit command type");
                }
            }
            break;
                
            default:
                executeError("Unrecognized Bluefruit command type");
                break;
        }
    }
    
    void Parser::rxReadBluefruit(char rxByte) {
        // LOGFMT("rxPos: %d, rxByte: %d\n", rxPos, rxByte);

        if ((rxPos + 1) >= currentCommand.paramLength) {
            LOGFMT("Received checksum: %d\n", rxByte);

            // rxByte should contain the checksum
            if (isBluefruitChecksumValid(rxByte)) {
                executeBluefruitCommand();
            } 
            else {
                executeError("Invalid Bluefruit checksum");
            }

            return;
        } 
        else {
            paramBuffer[rxPos] = rxByte;
            rxPos++;
        }
    }

    void Parser::rxReadText(char rxByte) {
        if (rxByte == '\n') {
            paramBuffer[rxPos] = 0;
            executeTextCommand();
            return;
        }

        paramBuffer[rxPos] = rxByte;
        rxPos++;

        if (rxPos >= paramBufferSize) {
            LOGLN("Param buffer full, truncating text.");

            // cap the buffer with a null character and just send what we received
            paramBuffer[paramBufferSize - 1] = 0;
            executeTextCommand();
        }
    }

    void Parser::executeBluefruitCommand() {
        switch (currentCommand.identifier) {
            case ID::color: 
                if (colorCallback) {
                    Color::RGB c(paramBuffer[0], paramBuffer[1], paramBuffer[2]);
                    colorCallback(c);
                }
                break;

            case ID::buttonEvent: 
                if (buttonEventCallback) {
                    uint8_t buttonIndex = paramBuffer[0] - '0' - 1;
                    bool pressed = paramBuffer[1] == '1';
                    ButtonEvent e(buttonIndex, pressed);
                    buttonEventCallback(e);
                }
                break;

            default:
                // The rest are handled elswhere, except for ID::none, which doesn't even make sense.
                break;
        }

        reset();
    }

    void Parser::executeTextCommand() {
        if (textCallback) {
            ParamBuffer buffer;
            strncpy(buffer, paramBuffer, paramBufferSize);
            buffer[paramBufferSize - 1] = 0;
            textCallback(buffer);
        }

        reset();
    }

    void Parser::executeError(const char* message) {
        if (errorCallback) {
            errorCallback(message);
        }

        reset();
    }
        
    //////////////////////////////////////////
    // helpers
    //////////////////////////////////////////
    bool Parser::isBluefruitChecksumValid(uint8_t checksum) {
        uint8_t sum = 0;

        // We already know this is a bluefruit command, so add the sentinel value and command code.
        sum += uint8_t(CmdType::bluefruit);
        sum += uint8_t(currentCommand.code);

        // Now parse the params buffer.
        for (int i = 0; i < currentCommand.paramLength; i++) {
            sum += paramBuffer[i];
        }

        uint8_t crc = checksum & ~sum;
        LOGFMT("Calculated checksum: %d\n", crc);
        return checksum == crc;        
    }
}
