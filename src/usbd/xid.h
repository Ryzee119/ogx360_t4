#ifndef XID_H_
#define XID_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* Digital Button Masks */
#define XID_DUP (1 << 0)
#define XID_DDOWN (1 << 1)
#define XID_DLEFT (1 << 2)
#define XID_DRIGHT (1 << 3)
#define XID_START (1 << 4)
#define XID_BACK (1 << 5)
#define XID_LS (1 << 6)
#define XID_RS (1 << 7)

typedef struct __attribute__((packed))
{
    uint8_t startByte; //Always 0x00
    uint8_t bLength;
    uint8_t dButtons;
    uint8_t reserved;
    uint8_t A;
    uint8_t B;
    uint8_t X;
    uint8_t Y;
    uint8_t BLACK;
    uint8_t WHITE;
    uint8_t L;
    uint8_t R;
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;
} USB_XboxGamepad_InReport_t;

typedef struct __attribute__((packed))
{
    uint8_t startByte; //Always 0x00
    uint8_t bLength;
    uint16_t lValue;
    uint16_t rValue;
} USB_XboxGamepad_OutReport_t;

bool xid_send_report_ready(void);
bool xid_send_report(USB_XboxGamepad_InReport_t *report, uint16_t len);
bool xid_get_report(USB_XboxGamepad_OutReport_t *report, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif