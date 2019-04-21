#include <windows.h> 
#include <stdio.h>
#include <setupapi.h>
#include <strsafe.h>
#include <winusb.h>

static GUID GUID_DEVINTERFACE_USB_DEVICE =
{ 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };

void MyApp_ProcessDeviceAtPath(LPCWSTR path) {
	HANDLE DeviceHandle = CreateFileW(
		path, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL
	);
	if(DeviceHandle == INVALID_HANDLE_VALUE) {
		printf("Error while opening file: %d\n", GetLastError());
		return;
	}
	WINUSB_INTERFACE_HANDLE WinUsbHandle;
	BOOL bResult = WinUsb_Initialize(DeviceHandle, &WinUsbHandle);
	if(bResult == FALSE) {
		printf("WinUsb initialize error: %d\n", GetLastError());
		return;
	}
	// WinUsbHandle is now valid
	// Get USB speed
	UCHAR DeviceSpeed;
	ULONG BufSize = 1;
	bResult = WinUsb_QueryDeviceInformation(
		WinUsbHandle, 
		DEVICE_SPEED, 
		&BufSize, 
		&DeviceSpeed
	);
	if (bResult == FALSE) {
	    printf("Error getting device speed: %d.\n", GetLastError());
		return;
	}
	switch(DeviceSpeed) {
		case LowSpeed: printf("Device speed: %d (Low speed).\n", DeviceSpeed); break;
		case FullSpeed: printf("Device speed: %d (Full speed).\n", DeviceSpeed); break;
		case HighSpeed: printf("Device speed: %d (High speed).\n", DeviceSpeed); break;
  		default: printf("Unknown speed %d\n", DeviceSpeed);
	}
}

int main() {
	// Get device interface info set handle for all devices
	// attached to system
	HDEVINFO hDevInfo = SetupDiGetClassDevs(
		&GUID_DEVINTERFACE_USB_DEVICE, 
		NULL, 
		NULL, 
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);
	if (hDevInfo == INVALID_HANDLE_VALUE){
		printf("SetupDiClassDevs() failed. GetLastError() " \
		"returns: 0x%x\n", GetLastError());
		return 1;
	}
	printf("Device info set handle for all devices attached to " \
	"system: 0x%x\n", hDevInfo);
	// Retrieve context structure for one interface of a device 
	// information set
	DWORD dwIndex = 0;
	SP_DEVICE_INTERFACE_DATA DevInterfaceData = { sizeof(SP_DEVICE_INTERFACE_DATA) };
	SP_DEVINFO_DATA DevInfoData = { sizeof(SP_DEVINFO_DATA) };
	PSP_DEVICE_INTERFACE_DETAIL_DATA_W pInterfaceDetailData;
	DWORD BufSize = 0;
	while(TRUE) {
		printf("BufSize: %d\n", BufSize);
		BOOL bRet = SetupDiEnumDeviceInterfaces(
			hDevInfo, 
			NULL, 
			&GUID_DEVINTERFACE_USB_DEVICE, 
			dwIndex, 
			&DevInterfaceData
		);
		if(bRet == FALSE) {
			if (GetLastError() == ERROR_NO_MORE_ITEMS) break;
			else printf("SetupDiEnumDeviceInterfaces failed " \
			"GetLastError() returns: 0x%x\n", GetLastError());
		}
		printf("%d: %d 0x%x\n", dwIndex, bRet, DevInterfaceData.InterfaceClassGuid);
		BOOL bRet_det_1 = SetupDiGetDeviceInterfaceDetailW(
			hDevInfo,
			&DevInterfaceData,
			pInterfaceDetailData,
			BufSize, 
			&BufSize,
			&DevInfoData
		);
		if(bRet_det_1 == FALSE) {
			if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if (pInterfaceDetailData != NULL) LocalFree(pInterfaceDetailData); 
				pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalAlloc(LPTR, BufSize);
				pInterfaceDetailData->cbSize = sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA_W);
				continue;
			} else {
				printf("Unexpected Error: %d\n", GetLastError());
				continue;
			}
		}
		// use %ls, not %s
	    printf("Device path: %ls\n", pInterfaceDetailData->DevicePath);  
		MyApp_ProcessDeviceAtPath(pInterfaceDetailData->DevicePath);
		dwIndex++;
	} 
	printf("Number of device interface sets representing all " \
	"devices attached to system: 0x%x\n", dwIndex);
	// Clean-up
	SetupDiDestroyDeviceInfoList(hDevInfo);
	if (pInterfaceDetailData != NULL) LocalFree(pInterfaceDetailData);
	return 0;
}

