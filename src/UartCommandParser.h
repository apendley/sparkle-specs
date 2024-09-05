#pragma once

#include "Color.h"
#include "ButtonEvent.h"

// Useful types and information.
namespace UartCommand {
    enum class ID: uint8_t {
        none = 0,
        color,
        buttonEvent,
        text,
    };

    // Includes terminating null
    static constexpr size_t paramBufferSize = 128;
    typedef char ParamBuffer[paramBufferSize];
}

// Parser class
namespace UartCommand {
    class Parser {
    public:
        Parser() = default;

        // Resets state machine
        void reset();

        // Call this for every byte read from incoming serial stream
        inline void rx(char b) {
            (this->*rxMethod)(b);
        }

        // Returns true if parser has not detected a command in the stream yet.
        // Returns false once the parser begins parsing a command from the stream.
        bool isIdle() const;

        // Callbacks

        void setColorCallback(void (*cb)(const Color::RGB&)) {
            colorCallback = cb;
        }

        void setButtonEventCallback(void (*cb)(const ButtonEvent&)) {
            buttonEventCallback = cb;
        }

        void setTextCallback(void (*cb)(const char*)) {
            textCallback = cb;
        }

        void setErrorCallback(void (*cb)(const char*)) {
            errorCallback = cb;
        }

private:
    // State machine methods
    private:
        using RxMethod = void (Parser::*)(char);
        void rxWaitForPrefix(char rxByte);
        void rxWaitForCode(char rxByte);
        void rxReadBluefruit(char rxByte);
        void rxReadText(char rxByte);

        void executeBluefruitCommand();
        void executeTextCommand();
        void executeError(const char* message);                        

    // Helpers
    private:
        bool isBluefruitChecksumValid(uint8_t checksum);

    // Types
    private:
        enum class CmdType: char {
            // "null" command type.
            none = 0,

            // All commands coming from the Bluefruit connect app start with '!'
            bluefruit = '!',

            // Text starts with '$', and continues until terminated with a '\n' character, or the buffer is full.
            text = '$',
        };

        enum class ParamType: uint8_t {
            buttonEvent,
            color,
            string,
        };    

        struct CommandDescription {
            // Character code for command.
            char code;

            // Command identifier;
            ID identifier;

            // Type of parameter used by this command.
            Parser::ParamType paramType;

            // Number of bytes to read for parameters.
            uint16_t paramLength;
        };

    // Static data
    private:
        static const CommandDescription nullCommand;
        static const CommandDescription bluefruitCommands[];
        static const int bluefruitCommandsCount;

    // Member variables
    private:
        ParamBuffer paramBuffer = {0};
        uint32_t rxPos = 0;
        
        CmdType currentCommandType = CmdType::none;
        RxMethod rxMethod = &Parser::rxWaitForPrefix;;

        CommandDescription currentCommand = nullCommand;

        void (*colorCallback)(const Color::RGB&) = nullptr;
        void (*buttonEventCallback)(const ButtonEvent&) = nullptr;
        void (*textCallback)(const char*) = nullptr;
        void (*errorCallback)(const char*) = nullptr;
    };
}
