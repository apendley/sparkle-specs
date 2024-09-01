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
        error,
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

        void reset();

        // Call this for every byte read from incoming serial stream
        void rx(char b);

        // Returns true if parser has started parsing a command.
        // Returns false if parser is in `waitForPrefix` or `complete` state
        bool isBusy() const;

        //////////////////////////////////////////////
        // Command detection and parameter extraction
        ////////////////////////////////////////////// 
        // After calling rx(), call this to determine if a valid command has been recognized.
        // If return value is ID::none, there is not a valid command ready.
        ID getCommand() const;

        // Otherwise, the parameters may be extracted using the functions below based on the parameter type.

        // ID::buttonEvent
        // Return the button event, or event with an invalid index if not in 'complete' state, 
        // or if params are not valid.
        ButtonEvent readButtonEvent() const;

        // ID::color
        // Returns black if not in 'complete' state, or if params are not valid.
        Color::RGB readColor() const;

        // ID::error
        // Returns false and sets buffer[0] to the '\0' if if not in 'complete' state, or if params are not valid.
        bool readString(char* buffer, size_t bufferSize) const;

        inline bool readString(ParamBuffer& buffer) const {
            return readString(buffer, paramBufferSize);
        }

    // RX methods
    private:
        // Sets error state, putting the message in the parameter buffer, and setting the parameter type to string.
        void error(const char* message);        
    
        // State processing methods
        using RxMethod = void (Parser::*)(char);
        void rxWaitForPrefix(char rxByte);
        void rxWaitForCode(char rxByte);
        void rxReadBluefruit(char rxByte);
        void rxReadText(char rxByte);
        void rxComplete(char rxByte);
        void rxError(char rxByte);

    // Helpers
    private:
        bool isChecksumValid(uint8_t checksum);

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
            // UartCommand::ButtonEvent
            buttonEvent,

            // Color::RGB
            color,

            // Pointer to string buffer of up to paramBufferSize bytes (including terminating null)
            string
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

    // Member variables
    private:
        ParamBuffer paramBuffer = {0};
        uint32_t rxPos = 0;
        
        CmdType currentCommandType = CmdType::none;
        RxMethod rxMethod = &Parser::rxWaitForPrefix;;

        static const CommandDescription nullCommand;
        CommandDescription currentCommand = nullCommand;

        static const CommandDescription bluefruitCommands[];
        static const int bluefruitCommandsCount;
    };
}
