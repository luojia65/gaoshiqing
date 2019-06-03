#include <windows.h> 
#include <stdio.h>
#include <setupapi.h>
#include <strsafe.h>
#include <winusb.h>
//#define NDEBUG
#include <assert.h>
#include <devguid.h>
//static GUID GUID_DEVINTERFACE_USB_DEVICE =
//{ 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
//static GUID GUID_DEVCLASS_USB =
//{ 0x36FC9E60L, 0xC465, 0x11CF, { 0x80, 0x56, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

int main() 
{		
	HDEVINFO hDevInfo;  
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(DeviceInfoData)};   
 
	// get device class information handle
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_USB,0, 0, DIGCF_PRESENT);       
	if (hDevInfo == INVALID_HANDLE_VALUE) {         
		int res = GetLastError();     
		return res;
	}  
 
	// enumerute device information
	char *buf = NULL; 
	DWORD CurSize = 0, ReqSize = 0;
	int i = 0; 
	while(SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData) == TRUE) {		
		DWORD DataT;     
		int ans = SetupDiGetDeviceRegistryPropertyA(
			hDevInfo, 
			&DeviceInfoData, 
			SPDRP_DEVICEDESC, 
			&DataT, 
			(LPBYTE) buf, 
			CurSize, 
			&ReqSize
		);
		// get device description information
		if (ans == FALSE) {
			if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if(buf == NULL) buf = (char *)LocalAlloc(LPTR, ReqSize);
				else buf = (char *)LocalReAlloc(buf, ReqSize, LMEM_MOVEABLE);
				CurSize = ReqSize;
			} else {
				i++;
			}
			continue;
		}
		
		printf("USB device %d: %s\n", i, buf);
		++i;
	}
	LocalFree(buf);
 
	return 0;
}

