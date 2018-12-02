#include "driver.h"
#include "driver.tmh"
#include "Registry.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, RegistryEvtDeviceAdd)
#pragma alloc_text (PAGE, RegistryEvtDriverContextCleanup)
#endif


NTSTATUS DriverEntry( _In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    WPP_INIT_TRACING( DriverObject, RegistryPath );

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = RegistryEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, RegistryEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &config, WDF_NO_HANDLE);

    if (!NT_SUCCESS(status)) {
        WPP_CLEANUP(DriverObject);
        return status;
    }

    return status;
}

NTSTATUS RegistryEvtDeviceAdd(_In_  WDFDRIVER  Driver,_Inout_ PWDFDEVICE_INIT DeviceInit)
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(Driver);
	PAGED_CODE();
    status = RegistryCreateDevice(DeviceInit);

	if (NT_SUCCESS(status)) {
		status = WorkWithRegistry();
	}

    return status;
}

VOID RegistryEvtDriverContextCleanup(_In_ WDFOBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();

    WPP_CLEANUP( WdfDriverWdmGetDriverObject( (WDFDRIVER) DriverObject) );
}
