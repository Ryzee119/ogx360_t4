#ifndef XID_DUKE_H_
#define XID_DUKE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "tusb.h"

/* https://github.com/Cxbx-Reloaded/Cxbx-Reloaded/blob/master/src/core/hle/XAPI/XapiCxbxr.h */
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYMAINWEAPON      0x0001
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYFIRE            0x0002
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYLOCKON          0x0004
#define    CXBX_SBC_GAMEPAD_W0_EJECT                   0x0008
#define    CXBX_SBC_GAMEPAD_W0_COCKPITHATCH            0x0010
#define    CXBX_SBC_GAMEPAD_W0_IGNITION                0x0020
#define    CXBX_SBC_GAMEPAD_W0_START                   0x0040
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONOPENCLOSE       0x0080
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONMAPZOOMINOUT    0x0100
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONMODESELECT      0x0200
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONSUBMONITOR      0x0400
#define    CXBX_SBC_GAMEPAD_W0_MAINMONZOOMIN           0x0800
#define    CXBX_SBC_GAMEPAD_W0_MAINMONZOOMOUT          0x1000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONFSS             0x2000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONMANIPULATOR     0x4000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONLINECOLORCHANGE 0x8000

#define    CXBX_SBC_GAMEPAD_W1_WASHING                 0x0001
#define    CXBX_SBC_GAMEPAD_W1_EXTINGUISHER            0x0002
#define    CXBX_SBC_GAMEPAD_W1_CHAFF                   0x0004
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONTANKDETACH      0x0008
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONOVERRIDE        0x0010
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONNIGHTSCOPE      0x0020
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF1              0x0040
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF2              0x0080
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF3              0x0100
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONMAIN           0x0200
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONSUB            0x0400
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONMAGAZINE       0x0800
#define    CXBX_SBC_GAMEPAD_W1_COMM1                   0x1000
#define    CXBX_SBC_GAMEPAD_W1_COMM2                   0x2000
#define    CXBX_SBC_GAMEPAD_W1_COMM3                   0x4000
#define    CXBX_SBC_GAMEPAD_W1_COMM4                   0x8000

#define    CXBX_SBC_GAMEPAD_W2_COMM5                   0x0001
#define    CXBX_SBC_GAMEPAD_W2_LEFTJOYSIGHTCHANGE      0x0002
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEFILTERCONTROL     0x0004
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEOXYGENSUPPLY      0x0008
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEFUELFLOWRATE      0x0010
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEBUFFREMATERIAL    0x0020
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEVTLOCATION        0x0040


typedef struct __attribute__((packed))
{
	uint8_t zero;
	uint8_t bLength;
	uint16_t dButtons[3];
	uint16_t aimingX;       //0 to 2^16 left to right
	uint16_t aimingY;       //0 to 2^16 top to bottom
	int16_t rotationLever;
	int16_t sightChangeX;
	int16_t sightChangeY;
	uint16_t leftPedal;      //Sidestep, 0x0000 to 0xFF00
	uint16_t middlePedal;    //Brake, 0x0000 to 0xFF00
	uint16_t rightPedal;     //Acceleration, 0x0000 to oxFF00
	int8_t tunerDial;        //0-15 is from 9oclock, around clockwise
	int8_t gearLever;        //7-13 is gears R,1,2,3,4,5
} USB_SteelBattalion_InReport_t;

typedef struct
{
	uint8_t zero;
	uint8_t bLength;
	uint8_t CockpitHatch_EmergencyEject;
	uint8_t Start_Ignition;
	uint8_t MapZoomInOut_OpenClose;
	uint8_t SubMonitorModeSelect_ModeSelect;
	uint8_t MainMonitorZoomOut_MainMonitorZoomIn;
	uint8_t Manipulator_ForecastShootingSystem;
	uint8_t Washing_LineColorChange;
	uint8_t Chaff_Extinguisher;
	uint8_t Override_TankDetach;
	uint8_t F1_NightScope;
	uint8_t F3_F2;
	uint8_t SubWeaponControl_MainWeaponControl;
	uint8_t Comm1_MagazineChange;
	uint8_t Comm3_Comm2;
	uint8_t Comm5_Comm4;
	uint8_t GearR_;
	uint8_t Gear1_GearN;
	uint8_t Gear3_Gear2;
	uint8_t Gear5_Gear4;
	uint8_t dummy;
} USB_SteelBattalion_OutReport_t;

static const tusb_desc_device_t STEELBATTALION_DESC_DEVICE =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0110,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x0A7B,
        .idProduct = 0xD000,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x00,
        .iProduct = 0x00,
        .iSerialNumber = 0x00,

        .bNumConfigurations = 0x01
};

static const uint8_t STEELBATTALION_DESC_CONFIGURATION[] =
    {
        0x09,       //bLength
        0x02,       //bDescriptorType
        0x20, 0x00, //wTotalLength
        0x01,       //bNumInterfaces
        0x01,       //bConfigurationValue
        0x00,       //iConfiguration
        0x80,       //bmAttributes
        0xFA,       //bMaxPower

        //Interface Descriptor
        0x09, //bLength
        0x04, //bDescriptorType
        0x00, //bInterfaceNumber
        0x00, //bAlternateSetting
        0x02, //bNumEndPoints
        0x58, //bInterfaceClass
        0x42, //bInterfaceSubClass
        0x00, //bInterfaceProtocol
        0x00, //iInterface

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x82,       //bEndpointAddress (IN endpoint 1)
        0x03,       //bmAttributes (Transfer: Interrupt / Synch: None / Usage: Data)
        0x20, 0x00, //wMaxPacketSize (1 x 32 bytes)
        0x04,       //bInterval (4 frames)

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x01,       //bEndpointAddress (IN endpoint 1)
        0x03,       //bmAttributes (Transfer: Interrupt / Synch: None / Usage: Data)
        0x20, 0x00, //wMaxPacketSize (1 x 32 bytes)
        0x04        //bInterval (4 frames)
};

static const uint8_t STEELBATTALION_DESC_XID[] = {
    0x10,
    0x42,
    0x00, 0x01,
    0x80,
    0x01,
    0x1A,
    0x16,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

bool xid_send_report_ready(void);
bool xid_send_report(USB_SteelBattalion_InReport_t *report, uint16_t len);
bool xid_get_report(USB_SteelBattalion_OutReport_t *report, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif