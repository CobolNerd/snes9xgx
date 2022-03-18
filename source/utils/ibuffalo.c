#ifdef HW_RVL
#include <gccore.h>
//#include "utils\log.h"

// iBuffalo SNES Controller
#define IBUFFALO_VID 0x0583
#define IBUFFALO_PID 0x2060	

static bool setup = false;
static bool replugRequired = false;
static s32 deviceId = 0;
static u8 endpoint = 0;
static u8 bMaxPacketSize = 0;
static u32 jpIbuffalo[2];

//char message[4096000];

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
	//LogToFile("before deviceId != 0\n");

	//LogToFile("deviceId: ");
	//sprintf(message, "%d", deviceId);
	//LogToFile(message);
	//LogToFile("\n");

	if (deviceId != 0)
	{
		return;
	}

	//LogToFile("after deviceId != 0\n");

	usb_device_entry dev_entry[8];
	u8 dev_count;

	s32 aux = USB_GetDeviceList(dev_entry, 8, USB_CLASS_HID, &dev_count) < 0;

	//LogToFile("after USB_GetDeviceList(dev_entry, 8, USB_CLASS_HID, &dev_count) < 0\n");

	//LogToFile("aux: ");
	//sprintf(message, "%d", aux);
	//LogToFile(message);
	//LogToFile("\n");

	if (aux)
	{
		return;
	}

	//LogToFile("after USB_GetDeviceList(dev_entry, 8, USB_CLASS_HID, &dev_count) < 0\n");

	//LogToFile("dev_count: ");
	//sprintf(message, "%d", dev_count);
	//LogToFile(message);
	//LogToFile("\n");

	for (int i = 0; i < dev_count; ++i)
	{
		bool bisIbuffaloGamepad = isIbuffaloGamepad(dev_entry[i]);

		//LogToFile("isIbuffaloGamepad: ");
		//sprintf(message, "%d", bisIbuffaloGamepad);
		//LogToFile(message);
		//LogToFile("\n");

		if (!bisIbuffaloGamepad)
		{
			continue;
		}
		s32 fd;

		//LogToFile("dev_entry[i].device_id: ");
		//sprintf(message, "%d", dev_entry[i].device_id);
		//LogToFile(message);
		//LogToFile("\n");
		//LogToFile("dev_entry[i].vid: ");
		//sprintf(message, "%d", dev_entry[i].vid);
		//LogToFile(message);
		//LogToFile("\n");
		//LogToFile("dev_entry[i].pid: ");
		//sprintf(message, "%d", dev_entry[i].pid);
		//LogToFile(message);
		//LogToFile("\n");//LogToFile("fd: ");
		//sprintf(message, "%d", fd);
		//LogToFile(message);
		//LogToFile("\n");

		s32 sUSB_OpenDevice = USB_OpenDevice(dev_entry[i].device_id, dev_entry[i].vid, dev_entry[i].pid, &fd);

		//LogToFile("sUSB_OpenDevice: ");
		//sprintf(message, "%d", sUSB_OpenDevice);
		//LogToFile(message);
		//LogToFile("\n");

		if (sUSB_OpenDevice < 0)
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

		//LogToFile("after if (USB_GetDescriptors(fd, &devdesc) < 0)\n");

		deviceId = fd;
		replugRequired = false;
		endpoint = getEndpoint(devdesc);
		bMaxPacketSize = devdesc.bMaxPacketSize0;

		// LogToFile("bMaxPacketSize: ");
		// sprintf(message, "%d", bMaxPacketSize);
		// LogToFile(message);
		// LogToFile("\n");

		USB_DeviceRemovalNotifyAsync(fd, &removal_cb, (void*) fd);

		//LogToFile("after USB_DeviceRemovalNotifyAsync(fd, &removal_cb, (void*) fd)\n");
		break;
	}

	//LogToFile(message);

	setup = true;

	//LogToFile("after setup = true\n");
}

//#define bMaxPacketSize2 8
//uint8_t ATTRIBUTE_ALIGN(32) bufAux[bMaxPacketSize2] = {NULL};
//bool bufChanged = false;

void Ibuffalo_ScanPads()
{
	if (deviceId == 0)
	{
		return;
	}

	uint8_t ATTRIBUTE_ALIGN(32) buf[bMaxPacketSize];
	s32 res = USB_ReadIntrMsg(deviceId, endpoint, sizeof(buf), buf);

	// LogToFile("res: ");
	// sprintf(message, "%d", res);
	// LogToFile(message);
	// LogToFile("\n");

	// char aux[4096];

	// if (bufAux[0] == NULL) {
	//  	for (int i = 0; i < bMaxPacketSize2; i++)
	// 	{
	// 		bufAux[i] = buf[i];
	// 	}
	// }
	// else {
	// 	if (buf[0] != bufAux[0] ||
	// 		buf[1] != bufAux[1] ||
	// 		buf[2] != bufAux[2] ||
	// 		buf[3] != bufAux[3] ||
	// 		buf[4] != bufAux[4] ||
	// 		buf[5] != bufAux[5] ||
	// 		buf[6] != bufAux[6] ||
	// 		buf[7] != bufAux[7]) {
	// 		bufChanged = true;
	// 		for (int i = 0; i < bMaxPacketSize2; i++)
	// 		{
	// 			bufAux[i] = buf[i];
	// 		}
	// 	}
	// 	else {
	// 		bufChanged = false;
	// 	}
		
	// 	if (bufChanged) {
	// 		// sprintf(message, "sizeof(buf) u: %u\n", sizeof(buf));
	// 		// strcat(aux, message);
	// 		// sprintf(message, "sizeof(buf) d: %d\n", sizeof(buf));
	// 		// strcat(aux, message);
	// 		sprintf(message, "############## buf CHANGED #################\n");
	// 		strcat(aux, message);

	// 		time_t timer;
	// 		char buffer[26];
	// 		struct tm* tm_info;

	// 		timer = time(NULL);
	// 		tm_info = localtime(&timer);

	// 		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	// 		puts(buffer);

	// 		sprintf(message, "Date: %s\n", buffer);
	// 		strcat(aux, message);

	// 		for (int i=0; i<bMaxPacketSize2; i++) {
	// 			if (buf[i] != 0) {
	// 				// if (i == 0 || i ==1) {
	// 				// 	if  (buf[0] != 128 && buf[0] != 129 && buf[1] && 128 && buf[1] != 129)
	// 				// }
	// 				sprintf(message, "buf[%i]: %u\n", i, buf[i]);
	// 				strcat(aux, message);
	// 			}
	// 		}

	// 		LogToFile(aux);
	// 	}
	// }

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
	// jp |= (buf[4] == 0x00) ? PAD_BUTTON_UP	: 0;
	// jp |= (buf[4] == 0xFF) ? PAD_BUTTON_DOWN  : 0;
	// jp |= (buf[3] == 0x00) ? PAD_BUTTON_LEFT  : 0;
	// jp |= (buf[3] == 0xFF) ? PAD_BUTTON_RIGHT : 0;

	// jp |= ((buf[5] & 0x2F) == 0x2F) ? PAD_BUTTON_A : 0;
	// jp |= ((buf[5] & 0x4F) == 0x4F) ? PAD_BUTTON_B : 0;
	// jp |= ((buf[5] & 0x1F) == 0x1F) ? PAD_BUTTON_X : 0;
	// jp |= ((buf[5] & 0x8F) == 0x8F) ? PAD_BUTTON_Y : 0;

	// jp |= ((buf[6] & 0x04) == 0x04) ? PAD_TRIGGER_L : 0;
	// jp |= ((buf[6] & 0x08) == 0x08) ? PAD_TRIGGER_R : 0;

	// jp |= ((buf[6] & 0x20) == 0x20) ? PAD_BUTTON_START : 0;
	// jp |= ((buf[6] & 0x10) == 0x10) ? PAD_TRIGGER_Z	: 0; // SNES select button maps to Z

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

	//if (jp != 0)
	//{
		//sprintf(message, "jp: %u\n", jp);

		//LogToFile(message);
	//}

	// Required, otherwise if the returned port isn't the one we are looking for, jp will be set to zero,
	// and held buttons are not possible

	jpIbuffalo[0] = jp;

	// LogToFile("jp: ");
	// sprintf(message, "%u", jp);
	// LogToFile(message);
	// LogToFile("\n");
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

	//LogToFile("start Ibuffalo_Status\n");

	open();

	// LogToFile("replugRequired: ");
	// sprintf(message, "%d", replugRequired);
	// LogToFile(message);
	// LogToFile("\n");

	// LogToFile("deviceId: ");
	// sprintf(message, "%d", deviceId);
	// LogToFile(message);
	// LogToFile("\n");

	// LogToFile("end Ibuffalo_Status\n");

	if (replugRequired)
		return "please replug";
	return deviceId ? "connected" : "not found";
}

#endif
