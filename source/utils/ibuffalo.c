#ifdef HW_RVL
#include <gccore.h>

// Buffalo iBuffalo Classic USB Gamepad for PC (SNES Controller)
#define IBUFFALO_VID 0x0583
#define IBUFFALO_PID 0x2060	
static bool setup = false;
static bool replugRequired = false;
static s32 deviceId = 0;
static u8 endpoint = 0;
static u8 bMaxPacketSize = 0;
static u32 jpIbuffalo[2];

static bool isIbuffaloGamepad(usb_device_entry dev)
{
	return dev.vid == IBUFFALO_VID && dev.pid == IBUFFALO_PID;
}

static u8 getEndpoint(usb_devdesc devdesc)
{
	if (devdesc.configurations == NULL || devdesc.configurations->interfaces == NULL ||
			devdesc.configurations->interfaces->endpoints == NULL)
	{
		return -1;
	}
	return devdesc.configurations->interfaces->endpoints->bEndpointAddress;
}

static int removal_cb(int result, void *usrdata)
{
	s32 fd = (s32) usrdata;
	if (fd == deviceId)
	{
		deviceId = 0;
	}
	return 1;
}

static void open()
{
	if (deviceId != 0)
	{
		return;
	}

	usb_device_entry dev_entry[8];
	u8 dev_count;
	if (USB_GetDeviceList(dev_entry, 8, USB_CLASS_HID, &dev_count) < 0)
	{
		return;
	}

	for (int i = 0; i < dev_count; ++i)
	{
		if (!isIbuffaloGamepad(dev_entry[i]))
		{
			continue;
		}
		s32 fd;
		if (USB_OpenDevice(dev_entry[i].device_id, dev_entry[i].vid, dev_entry[i].pid, &fd) < 0)
		{
			continue;
		}

		usb_devdesc devdesc;
		if (USB_GetDescriptors(fd, &devdesc) < 0)
		{
			// You have to replug the controller!
			replugRequired = true;
			USB_CloseDevice(&fd);
			break;
		}

		deviceId = fd;
		replugRequired = false;
		endpoint = getEndpoint(devdesc);
		bMaxPacketSize = devdesc.bMaxPacketSize0;
		USB_DeviceRemovalNotifyAsync(fd, &removal_cb, (void*) fd);
		break;
	}

	setup = true;
}

void Ibuffalo_ScanPads()
{
	if (deviceId == 0)
	{
		return;
	}

	uint8_t ATTRIBUTE_ALIGN(32) buf[bMaxPacketSize];
	s32 res = USB_ReadIntrMsg(deviceId, endpoint, sizeof(buf), buf);
	if (res < 0)
	{
		return;
	}

	// buf[0] contains the port returned
	// you have to make 2 calls to get the status, even if you're only interested in one port
	// because it is not sure which port is returned first

	// 1 = Right port
	// 2 = Left port

	// Button layout
	// A=5,2F
	// B=5,4F
	// X=5,1F
	// Y=5,8F
	// Select=6,10 
	// Start=6,20  
	// Up=4,00
	// Right=3,FF
	// Left=3,00
	// Down=4,FF
	// L=6,04
	// R=6,08

	u32 jp = 0;

	jp |= (buf[2] == 1)   ? PAD_BUTTON_A : 0;
	jp |= (buf[2] == 2)   ? PAD_BUTTON_B : 0;
	jp |= (buf[2] == 8)   ? PAD_BUTTON_Y : 0;
	jp |= (buf[2] == 4)   ? PAD_BUTTON_X : 0;
	jp |= (buf[2] == 16)  ? PAD_TRIGGER_L : 0;
	jp |= (buf[2] == 32)  ? PAD_TRIGGER_R : 0;
	jp |= (buf[2] == 128) ? PAD_BUTTON_START : 0;
	jp |= (buf[2] == 64)  ? PAD_TRIGGER_Z : 0;
	jp |= (buf[0] == 0)   ? PAD_BUTTON_LEFT : 0;
	jp |= (buf[0] == 255) ? PAD_BUTTON_RIGHT : 0;
	jp |= (buf[1] == 0)   ? PAD_BUTTON_UP : 0;
	jp |= (buf[1] == 255) ? PAD_BUTTON_DOWN : 0;

	jpIbuffalo[0] = jp;
}

u32 Ibuffalo_ButtonsHeld(int chan)
{
	if(!setup)
	{
		open();
	}
	if (deviceId == 0)
	{
		return 0;
	}
	return jpIbuffalo[chan];
}

char* Ibuffalo_Status()
{
	open();
	if (replugRequired)
		return "please replug";
	return deviceId ? "connected" : "not found";
}

#endif
