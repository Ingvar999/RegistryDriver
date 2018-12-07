#pragma once

EXTERN_C_START

typedef struct _DEVICE_CONTEXT
{
    WDFUSBDEVICE UsbDevice;
	LARGE_INTEGER CallbackID;
	HANDLE FileHandleLogRegistry;
	HANDLE FileHandleLogDriver;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

NTSTATUS RegistryCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);

EVT_WDF_DEVICE_PREPARE_HARDWARE RegistryEvtDevicePrepareHardware;
EVT_WDF_DEVICE_D0_EXIT  RegistryDeviceD0Exit;

EXTERN_C_END
