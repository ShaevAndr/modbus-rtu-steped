#ifndef ERRORCODES_H
#define ERRORCODES_H

typedef enum
{
    ecNoError       = 0x00,
    ecBadCS         = 0x01,
    ecBadLength     = 0x02,
    ecBadType       = 0x03,
    ecBadCommand    = 0x04,
    ecBadParams     = 0x05,
    ecBadComRun     = 0x06,
    ecNotReady      = 0x07,
    ecNoProcess     = 0x08,
    ecNoSuchParam   = 0x09,
    ecInternalError = 0x0A,
    ecWriteROParam  = 0x0B,

    ecBusError      = 0x20,
    ecInterrupted   = 0x21,
    ecBadAnswer     = 0x22,

    ecRetryCountReached  = 0xFD,
    ecNoAnswer      = 0xFE,
    ecInnerError    = 0xFF
} ECodes;

#endif // ERRORCODES_H
