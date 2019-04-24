#include <windows.h> 
#include <stdio.h>
#include <setupapi.h>
#include <strsafe.h>
#include <winusb.h>
//#define NDEBUG
#include <assert.h>

static GUID GUID_DEVINTERFACE_USB_DEVICE =
{ 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };

void MyApp_PrintDeviceSpeed(WINUSB_INTERFACE_HANDLE WinUsbHandle) {
	UCHAR DeviceSpeed;
	ULONG BufSize = 1;
	BOOL bResult = WinUsb_QueryDeviceInformation(
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
  		default: printf("Unrecognized speed %d\n", DeviceSpeed);
	}
}

void MyApp_PrintPipeList(WINUSB_INTERFACE_HANDLE WinUsbHandle) {
	USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
//  	ZeroMemory( &InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR) );
	
	WINUSB_PIPE_INFORMATION Pipe;
//  	ZeroMemory( &Pipe, sizeof(WINUSB_PIPE_INFORMATION) );
	
	BOOL bResult = WinUsb_QueryInterfaceSettings(WinUsbHandle, 0, &InterfaceDescriptor); 
  	assert(bResult == TRUE);
	for(UCHAR index = 0; index < InterfaceDescriptor.bNumEndpoints; ++index) {
		bResult = WinUsb_QueryPipe(WinUsbHandle, 0, index, &Pipe);
		assert(bResult == TRUE);
		printf("Pipe #%d, PipeId %d, ", index, Pipe.PipeId);
		switch(Pipe.PipeType) {
			case UsbdPipeTypeControl: printf("PipeType: UsbdPipeTypeControl"); break;
			case UsbdPipeTypeIsochronous: printf("PipeType: UsbdPipeTypeIsochronous"); break;
			case UsbdPipeTypeBulk: 
				printf("PipeType: UsbdPipeTypeBulk, "); 
				if(USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
		            printf("Direction: In");
	          	if(USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
		            printf("Direction: Out");
				break;
			case UsbdPipeTypeInterrupt: printf("PipeType: UsbdPipeTypeInterrupt"); break;
		}
		printf("\n");
	}
}

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
	// WinUsbHandle is now valid. Do NOT confuse WinUsbHandle with DeviceHandle
	
	// Print USB speed
	MyApp_PrintDeviceSpeed(WinUsbHandle);
	
	// Print pipes  
	MyApp_PrintPipeList(WinUsbHandle);
	
	/*
		#define STLINK_RX_EP          (1|ENDPOINT_IN)
		#define STLINK_TX_EP          (2|ENDPOINT_OUT)
		#define STLINK_TRACE_EP       (3|ENDPOINT_IN)
	*/
	
	// Teardowns
	WinUsb_Free(WinUsbHandle);
	CloseHandle(DeviceHandle);
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
				if (pInterfaceDetailData == NULL) 
					pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalAlloc(LPTR, BufSize);
				else pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalReAlloc(pInterfaceDetailData, BufSize, LPTR);
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

