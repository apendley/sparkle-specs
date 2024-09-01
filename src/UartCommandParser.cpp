#include <Arduino.h>
#include "UartCommandParser.h"

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

    bool Parser::isBusy() const {
        return (rxMethod != &Parser::rxWaitForPrefix) && (rxMethod != &Parser::rxComplete);
    }
        
    void Parser::rx(char rxByte) {
        // if (rxByte == '\n') {
        //     Serial.println("<newline>");
        // }
        // else {
        //     Serial.println(rxByte);
        // }  

        (this->*rxMethod)(rxByte);
    }

    ID Parser::getCommand() const {
        if (rxMethod == &Parser::rxError) {
            return ID::error;
        }
        
        if (rxMethod != &Parser::rxComplete) {
            return ID::none;
        }
        
        switch (currentCommandType) {
            case CmdType::bluefruit:
                return currentCommand.identifier;

            case CmdType::text:
                return ID::text;

            default:
                return ID::none;
        }
    }

    //////////////////////////////////////////
    // RX
    //////////////////////////////////////////
    void Parser::rxWaitForPrefix(char rxByte) {
        // Probably a better way to do this
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
                error("Unrecognized UART command");
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
                    error("unrecognized Bluefruit Connect command\n");
                }
            }
            break;
                
            default:
                error("Unrecognized command code type");
                break;
        }
    }
    
    void Parser::rxReadBluefruit(char rxByte) {
        // Serial.printf("rxPos: %d, rxByte: %d\n", rxPos, rxByte);

        if ((rxPos + 1) >= currentCommand.paramLength) {
            // rxByte should contain the checksum
            // Serial.printf("checksum: %d, isValid: %d\n", rxByte, isChecksumValid(rxByte));

            if (isChecksumValid(rxByte)) {
                rxMethod = &Parser::rxComplete;
            } else {
                error("Invalid Bluefruit CRC");
            }
        } else {
            paramBuffer[rxPos] = rxByte;
            rxPos++;
        }
    }
    
    void Parser::rxReadText(char rxByte) {
        if (rxByte == '\n') {
            // Serial.println("\nend of text");
            paramBuffer[rxPos] = 0;
            rxMethod = &Parser::rxComplete;
            return;
        }

        paramBuffer[rxPos] = rxByte;
        rxPos++;

        if (rxPos >= paramBufferSize) {
            error("Parameter buffer full");
        }
    }
    
    void Parser::rxComplete(char rxByte) {
        // All input is ignored until state is reset.
    }

    void Parser::rxError(char rxByte) {
        // All input is ignored until state is reset.
    }

    void Parser::error(const char* message) {
        if (message != nullptr) {
            strcpy(paramBuffer, message);
        }
        
        currentCommand = nullCommand;
        rxMethod = &Parser::rxError;
    }
        

    //////////////////////////////////////////
    // Parameter extraction
    //////////////////////////////////////////
    ButtonEvent Parser::readButtonEvent() const {
        if (rxMethod != &Parser::rxComplete || currentCommand.paramType != ParamType::buttonEvent) {
            return ButtonEvent();
        }

        uint8_t buttonIndex = paramBuffer[0] - '0' - 1;
        bool pressed = paramBuffer[1] == '1';
        return ButtonEvent(buttonIndex, pressed);
    }


    Color::RGB Parser::readColor() const {
        if (rxMethod != &Parser::rxComplete || currentCommand.paramType != ParamType::color) {
            return Color::RGB();
        }
        
        uint8_t red = paramBuffer[0];
        uint8_t green = paramBuffer[1];
        uint8_t blue = paramBuffer[2];
        return Color::RGB(red, green, blue);
    }

    bool Parser::readString(char* buffer, size_t bufferSize) const {
        bool isStateAllowed = (rxMethod == &Parser::rxComplete) || (rxMethod == &Parser::rxError);
        
        if (isStateAllowed == false || currentCommand.paramType != ParamType::string) {
            buffer[0] = 0;
            return false;
        }

        if (paramBuffer[0] == 0) {
            buffer[0] = 0;
            return false;
        }

        size_t minBufferSize = min(bufferSize, paramBufferSize);
        strncpy(buffer, paramBuffer, minBufferSize);
        buffer[minBufferSize - 1] = 0;
        return true;
    }

    //////////////////////////////////////////
    // helpers
    //////////////////////////////////////////
    bool Parser::isChecksumValid(uint8_t checksum) {
        uint8_t sum = 0;

        // We already know this is a bluefruit command, so add the sentinel value and command code.
        sum += uint8_t(CmdType::bluefruit);
        sum += uint8_t(currentCommand.code);

        // Now parse the params buffer.
        for (int i = 0; i < currentCommand.paramLength; i++) {
            sum += paramBuffer[i];
        }

        uint8_t crc = checksum & ~sum;
        // Serial.printf("Calculated crc: %d\n", crc);
        return checksum == crc;        
    }
}
