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

#define ENDPOINT_IN  0x80
#define ENDPOINT_OUT 0x00
#define STLINK_RX_EP	(1|ENDPOINT_IN)
#define STLINK_TX_EP    (2|ENDPOINT_OUT)
#define STLINK_TRACE_EP (3|ENDPOINT_IN)

#define STLINK_GET_VERSION             0xF1
#define STLINK_DEBUG_COMMAND           0xF2
#define STLINK_DFU_COMMAND             0xF3
#define STLINK_SWIM_COMMAND            0xF4
#define STLINK_GET_CURRENT_MODE        0xF5
#define STLINK_GET_TARGET_VOLTAGE      0xF7

#define STLINK_SG_SIZE        (31)
#define STLINK_CMD_SIZE_V2    (16)
#define STLINK_DATA_SIZE      (4096)

typedef struct _STLinkV2 {
	WINUSB_INTERFACE_HANDLE winusb_handle;
	UCHAR cmd_buf[STLINK_SG_SIZE];
	size_t cmd_idx;
	UCHAR data_buf[STLINK_DATA_SIZE];
	size_t data_idx;
} *MYAPP_STLINKV2_HANDLE;

MYAPP_STLINKV2_HANDLE MyApp_CreateSTLinkV2Handle(WINUSB_INTERFACE_HANDLE winusb_handle) {
	MYAPP_STLINKV2_HANDLE ans = (MYAPP_STLINKV2_HANDLE)malloc(sizeof(_STLinkV2));
	ZeroMemory(ans, sizeof(_STLinkV2));
	ans->winusb_handle = winusb_handle;
	return ans;
}

void MyApp_FreeSTLinkV2Handle(MYAPP_STLINKV2_HANDLE handle) {
	free(handle);
}

void MyAppPrivate_STLinkV2_GetDescriptor(MYAPP_STLINKV2_HANDLE handle, UCHAR index, USHORT lang_id, PULONG LengthTransferred) {
	BOOL bResult = WinUsb_GetDescriptor(
		handle->winusb_handle,
		USB_STRING_DESCRIPTOR_TYPE,
		index, 
		lang_id,
		handle->cmd_buf,
		STLINK_SG_SIZE,
		LengthTransferred
	);
	if(bResult == FALSE) {
		printf("Error occurred in GetDescriptor: %d\n", GetLastError());
		return;
	}
}

void MyApp_ProcessSTLinkV2(MYAPP_STLINKV2_HANDLE handle) {
	ULONG LengthTransferredOut, LengthTransferredIn;
	MyAppPrivate_STLinkV2_GetDescriptor(handle, 0x00, LANG_NEUTRAL, &LengthTransferredOut);
	USHORT lang_id = *(USHORT*)(handle->cmd_buf+2);
	MyAppPrivate_STLinkV2_GetDescriptor(handle, 0x03, lang_id, &LengthTransferredOut);
	printf("Device serial number: [");
	for(int i=0;i<LengthTransferredOut;i+=2) printf("%lc", handle->cmd_buf[i]);
	printf("]\n");
	memset(handle->cmd_buf, 0, STLINK_SG_SIZE);
	handle->cmd_buf[0] = STLINK_GET_VERSION;
	HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	OVERLAPPED overlapped;
	overlapped.hEvent = hEvent;
	BOOL bResult = WinUsb_WritePipe(
		handle->winusb_handle,
		STLINK_TX_EP,
		handle->cmd_buf,
		STLINK_CMD_SIZE_V2,
		NULL,//&LengthTransferred,
		&overlapped
	);
	if(bResult == FALSE) {
		if(GetLastError() != ERROR_IO_PENDING) {
			printf("Error occurred while writing pipe: %d\n", GetLastError());
			return;
		}
	}
	bResult = WinUsb_GetOverlappedResult(
		handle->winusb_handle,
		&overlapped,
		&LengthTransferredOut,
		FALSE // bWait
	);
	bResult = WinUsb_ReadPipe(
		handle->winusb_handle,
		STLINK_RX_EP,
		handle->data_buf,
		STLINK_DATA_SIZE,
		NULL,
		&overlapped
	);
	if(bResult == FALSE) {
		if(GetLastError() != ERROR_IO_PENDING) {
			printf("Error occurred while reading pipe: %d\n", GetLastError());
			return;
		}
	}
	bResult = WinUsb_GetOverlappedResult(
		handle->winusb_handle,
		&overlapped,
		&LengthTransferredIn,
		TRUE // bWait
	);
	if(bResult == FALSE) {
		printf("Error occurred while waiting: %d\n", GetLastError());
		return;
	}
	printf("%d bytes written, %d bytes read!\n", LengthTransferredOut, LengthTransferredIn);
}

void MyApp_ProcessDeviceAtPath(LPCWSTR path) {
	HANDLE DeviceHandle = CreateFileW(
		path, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL
	);
	if(DeviceHandle == INVALID_HANDLE_VALUE) {
		printf("Error while opening file: %d\n", GetLastError());
		return;
	}
	WINUSB_INTERFACE_HANDLE WinUsbHandle;
	// Do NOT confuse WinUsbHandle with DeviceHandle
	BOOL bResult = WinUsb_Initialize(DeviceHandle, &WinUsbHandle);
	if(bResult == FALSE) {
		printf("WinUsb initialize error: %d\n", GetLastError());
		CloseHandle(DeviceHandle);
		return;
	}
	// WinUsbHandle is now valid. 
	
	// Print description
	USB_DEVICE_DESCRIPTOR desc;
	ULONG len = 0;
	WinUsb_GetDescriptor(
		WinUsbHandle,
		USB_DEVICE_DESCRIPTOR_TYPE,
		0,
		LANG_NEUTRAL,
		(PUCHAR)&desc,
		sizeof(USB_DEVICE_DESCRIPTOR),
		&len // cannot be null
	);
	printf("Descriptor: bcdUSB: 0x%X, bDeviceClass: 0x%X, " \
		"bDeviceSubClass: 0x%X, bDeviceProtocol: 0x%X, bMaxPacketSize0: 0x%X, " \
		"idVendor: 0x%X, idProduct: 0x%X, bcdDevice: 0x%X, " \
		"iManufacturer: 0x%X, iProduct: 0x%X, iSerialNumber: 0x%X, " \
		"bNumConfigurations: 0x%X, \n",
		desc.bcdUSB, desc.bDeviceClass,
		desc.bDeviceSubClass, desc.bDeviceProtocol, desc.bMaxPacketSize0,
		desc.idVendor, desc.idProduct, desc.bcdDevice,
		desc.iManufacturer, desc.iProduct, desc.iSerialNumber,
		desc.bNumConfigurations
	);
	
	// Print USB speed
	MyApp_PrintDeviceSpeed(WinUsbHandle);
	
	// Print pipes  
	MyApp_PrintPipeList(WinUsbHandle);
	
#define STLINK_VID		(0x0483) 
#define STLINK_V2_PID   (0x3748)
	if(desc.idVendor == STLINK_VID) {
		switch(desc.idProduct) {
			case STLINK_V2_PID: 
				printf("STLinkV2 (0x0483, 0x3748) detected!\n"); 
				MYAPP_STLINKV2_HANDLE handle = MyApp_CreateSTLinkV2Handle(WinUsbHandle);
				MyApp_ProcessSTLinkV2(handle); 
				MyApp_FreeSTLinkV2Handle(handle);
				break;
		}
	} 
	
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
	DWORD BufSize = 0, BufTail = 0;
	while(TRUE) {
		printf("BufSize: %d, BufTail: %d\n", BufSize, BufTail);
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
			&BufTail,
			&DevInfoData
		);
		if(bRet_det_1 == FALSE) {
			if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if (pInterfaceDetailData == NULL) 
					pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalAlloc(LPTR, BufTail);
				else pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalReAlloc(pInterfaceDetailData, BufTail, LPTR);
				pInterfaceDetailData->cbSize = sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA_W);
				BufSize = BufTail;
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

