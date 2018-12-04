#include <stdio.h>

#include "Registry.h"
#include "Driver.h"
#include "Device.h"

NTSTATUS WorkWithRegistry(IN WDFDEVICE device) {
	NTSTATUS status;
	PDEVICE_CONTEXT context;
	OBJECT_ATTRIBUTES attributes;
	IO_STATUS_BLOCK statusBlock;
	LARGE_INTEGER size;
	UNICODE_STRING uString;

	context = DeviceGetContext(device);
	status = CmRegisterCallback(ChangeRegistryCallback, context, &context->CallbackID);
	if (NT_SUCCESS(status)) {
		RtlInitUnicodeString(&uString, L"\\DosDevices\\C:\\LogRegistry.txt");
		InitializeObjectAttributes(&attributes, &uString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		context->FileHandle = 0;
		status = ZwCreateFile(&context->FileHandle, GENERIC_WRITE, &attributes, &statusBlock,
								&size, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_WRITE | FILE_SHARE_READ, 
								FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status)) {
			CmUnRegisterCallback(context->CallbackID);
		}
	}

	return status;
}
      
NTSTATUS RegistryCleanup(IN WDFDEVICE device) {
	NTSTATUS status;
	PDEVICE_CONTEXT context;

	context = DeviceGetContext(device);
	status = CmUnRegisterCallback(context->CallbackID);
	status = ZwClose(context->FileHandle);

	return status;
}

NTSTATUS ChangeRegistryCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2) {
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT context;
	IO_STATUS_BLOCK statusBlock;
	SIZE_T bufSize = -1;
	WCHAR message[255];
	TIME systemTime, localTime;
	unsigned int microseconds, miliseconds, seconds, minutes, hours;

	PREG_CREATE_KEY_INFORMATION infoCreateKey;
	PREG_DELETE_VALUE_KEY_INFORMATION infoDeleteValue;
	PREG_SET_VALUE_KEY_INFORMATION infoSetValue;

	context = (PDEVICE_CONTEXT)CallbackContext;
	KeQuerySystemTimePrecise(&systemTime);
	ExSystemTimeToLocalTime(&systemTime, &localTime);
	microseconds = (localTime.LowPart % 10000) / 10;
	miliseconds = (localTime.LowPart % 10000000) / 10000;
	seconds = (localTime.QuadPart % 600000000) / 10000000;
	minutes = (localTime.QuadPart % 36000000000) / 600000000;
	hours = (localTime.QuadPart % 864000000000)  / 36000000000;

	switch ((REG_NOTIFY_CLASS)Argument1) {

	case RegNtPreCreateKeyEx:{
		infoCreateKey = (PREG_CREATE_KEY_INFORMATION)Argument2;
		bufSize = swprintf(message, L"[%d:%d:%d:%d:%d Create Key] %s\r", 
			hours, minutes, seconds, miliseconds, microseconds, infoCreateKey->CompleteName->Buffer);
	} break;

	case RegNtDeleteValueKey: {
		infoDeleteValue = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
		bufSize = swprintf(message, L"[%d:%d:%d:%d:%d Delete Value] %s\r",
			hours, minutes, seconds, miliseconds, microseconds, infoDeleteValue->ValueName->Buffer);
	} break;

	case RegNtSetValueKey: {
		infoSetValue = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
		bufSize = swprintf(message, L"[%d:%d:%d:%d:%d Set Value] %s\r",
			hours, minutes, seconds, miliseconds, microseconds, infoSetValue->ValueName->Buffer);
	} break;
	}

	if (bufSize != -1) {
		status = ZwWriteFile(context->FileHandle, NULL, NULL, NULL, &statusBlock, message, bufSize * sizeof(WCHAR), NULL, NULL);
	}

	return status;
}