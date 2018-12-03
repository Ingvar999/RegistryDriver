#include "Registry.h"
#include "Driver.h"
#include "Device.h"
#include <stdio.h>


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
	SIZE_T bufSize;
	CHAR message[200];
	time_t ttime;
	PREG_CREATE_KEY_INFORMATION infoCreateKey;
	

	context = (PDEVICE_CONTEXT)CallbackContext;
	switch ((REG_NOTIFY_CLASS)Argument1) {
	case RegNtPreCreateKeyEx: case RegNtPreCreateKey:{

		infoCreateKey = (PREG_CREATE_KEY_INFORMATION)Argument2;
		//ttime = time(NULL);
		//gettime
		bufSize = sprintf(message, "%s Create Key\r", ctime(&ttime));
		if (bufSize != -1) {
			status = ZwWriteFile(context->FileHandle, NULL, NULL, NULL, &statusBlock, message, bufSize, NULL, NULL);
		}

	} break;
	}

	return status;
}