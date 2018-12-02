#include "driver.h"
#include "device.tmh"
#include "Registry.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, RegistryCreateDevice)
#pragma alloc_text (PAGE, RegistryEvtDevicePrepareHardware)
#endif


NTSTATUS RegistryCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit)
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;

    PAGED_CODE();

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware = RegistryEvtDevicePrepareHardware;
	pnpPowerCallbacks.EvtDeviceD0Exit = RegistryDeviceD0Exit;
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        deviceContext = DeviceGetContext(device);
        status = WdfDeviceCreateDeviceInterface( device, &GUID_DEVINTERFACE_Registry, NULL);
    }

    return status;
}

NTSTATUS RegistryEvtDevicePrepareHardware(IN WDFDEVICE Device, IN WDFCMRESLIST ResourceList, IN WDFCMRESLIST ResourceListTranslated) {
	NTSTATUS status;
	UNREFERENCED_PARAMETER(ResourceList);
	UNREFERENCED_PARAMETER(ResourceListTranslated);
	status = WorkWithRegistry(Device);
	return status;
}

NTSTATUS RegistryDeviceD0Exit(WDFDEVICE  Device, WDF_POWER_DEVICE_STATE  TargetState) {
	NTSTATUS status;
	UNREFERENCED_PARAMETER(TargetState);
	status = RegistryCleanup(Device);
	return status;
}