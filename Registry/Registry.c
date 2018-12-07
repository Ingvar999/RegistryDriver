#include <stdio.h>

#include "Registry.h"
#include "Driver.h"
#include "Device.h"

typedef struct _LOG_TIME {
	DWORD32 microseconds, miliseconds, seconds, minutes, hours;

} LOG_TIME, *PLOG_TIME;

void InitLogTime(PLOG_TIME t) {
	TIME systemTime, localTime;

	KeQuerySystemTimePrecise(&systemTime);
	ExSystemTimeToLocalTime(&systemTime, &localTime);
	t->microseconds = (localTime.LowPart % 10000) / 10;
	t->miliseconds = (localTime.LowPart % 10000000) / 10000;
	t->seconds = (localTime.QuadPart % 600000000) / 10000000;
	t->minutes = (localTime.QuadPart % 36000000000) / 600000000;
	t->hours = (localTime.QuadPart % 864000000000) / 36000000000;
}

NTSTATUS WorkWithRegistry(IN WDFDEVICE device) {
	NTSTATUS status;
	PDEVICE_CONTEXT context;
	OBJECT_ATTRIBUTES attributes;
	IO_STATUS_BLOCK statusBlock;
	LARGE_INTEGER size;
	UNICODE_STRING uString;
	WCHAR message[255];
	LOG_TIME t;

	context = DeviceGetContext(device);
	status = CmRegisterCallback(ChangeRegistryCallback, context, &context->CallbackID);
	if (NT_SUCCESS(status)) {
		RtlInitUnicodeString(&uString, L"\\DosDevices\\C:\\LogRegistry.txt");
		InitializeObjectAttributes(&attributes, &uString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		context->FileHandleLogRegistry = 0;
		status = ZwCreateFile(&context->FileHandleLogRegistry, GENERIC_WRITE, &attributes, &statusBlock,
								&size, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_WRITE | FILE_SHARE_READ, 
								FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (NT_SUCCESS(status)) {
			RtlInitUnicodeString(&uString, L"\\DosDevices\\C:\\LogDriver.txt");
			InitializeObjectAttributes(&attributes, &uString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
			context->FileHandleLogDriver = 0;
			status = ZwCreateFile(&context->FileHandleLogDriver, GENERIC_WRITE, &attributes, &statusBlock,
				&size, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_WRITE | FILE_SHARE_READ,
				FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
			if (NT_SUCCESS(status)) {
				InitLogTime(&t);
				SIZE_T len = swprintf(message, L"[%d:%d:%d:%d:%d Start]\r",
					t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds);
				status = ZwWriteFile(context->FileHandleLogDriver, NULL, NULL, NULL, &statusBlock, message, len * sizeof(WCHAR), NULL, NULL);
			}
		}
		else {
			CmUnRegisterCallback(context->CallbackID);
		}
	}

	return status;
}
      
NTSTATUS RegistryCleanup(IN WDFDEVICE device) {
	NTSTATUS status;
	PDEVICE_CONTEXT context;
	IO_STATUS_BLOCK statusBlock;
	WCHAR message[255];
	LOG_TIME t;

	context = DeviceGetContext(device);

	InitLogTime(&t);
	SIZE_T len = swprintf(message, L"[%d:%d:%d:%d:%d Start]\r",
		t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds);
	ZwWriteFile(context->FileHandleLogDriver, NULL, NULL, NULL, &statusBlock, message, len * sizeof(WCHAR), NULL, NULL);

	CmUnRegisterCallback(context->CallbackID);
	status = ZwClose(context->FileHandleLogRegistry);
	status = ZwClose(context->FileHandleLogDriver);

	return status;
}

NTSTATUS ChangeRegistryCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2) {
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT context;
	IO_STATUS_BLOCK statusBlock;
	SIZE_T sizeToLog = -1;
	WCHAR message[255];
	LOG_TIME t;

	PREG_CREATE_KEY_INFORMATION infoCreateKey;
	PREG_DELETE_KEY_INFORMATION infoDeleteKey;
	PREG_DELETE_VALUE_KEY_INFORMATION infoDeleteValue;
	PREG_SET_VALUE_KEY_INFORMATION infoSetValue;

	context = (PDEVICE_CONTEXT)CallbackContext;

	switch ((REG_NOTIFY_CLASS)Argument1) {

	case RegNtPreCreateKeyEx:{
		infoCreateKey = (PREG_CREATE_KEY_INFORMATION)Argument2;
		InitLogTime(&t);
		sizeToLog = swprintf(message, L"[%d:%d:%d:%d:%d Create Key] %s\r", 
			t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds, infoCreateKey->CompleteName->Buffer);
	} break;

	case RegNtPreDeleteKey: {
		infoDeleteKey = (PREG_DELETE_KEY_INFORMATION)Argument2;
		InitLogTime(&t);
		KEY_NAME_INFORMATION infoKeyName;
		ULONG recieved;
		if (NT_SUCCESS(status = ZwQueryKey(
			infoDeleteKey->Object, KeyNameInformation, &infoKeyName, sizeof(KEY_NAME_INFORMATION), &recieved))) {

			DWORD32 length = infoKeyName.NameLength / sizeof(WCHAR);
			WCHAR c = infoKeyName.Name[length - 1];
			infoKeyName.Name[length - 1] = '\0';

			sizeToLog = swprintf(message, L"[%d:%d:%d:%d:%d Delete Key] %s%c\r",
				t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds, infoKeyName.Name, c);
		}
		else {
			SIZE_T len = swprintf(message, L"[%d:%d:%d:%d:%d Status %d] requaried %d\r",
				t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds, (ULONG)status, recieved - sizeof(KEY_NAME_INFORMATION));
			ZwWriteFile(context->FileHandleLogDriver, NULL, NULL, NULL, &statusBlock, message, len * sizeof(WCHAR), NULL, NULL);
		}
	} break;

	case RegNtDeleteValueKey: {
		infoDeleteValue = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
		InitLogTime(&t);
		sizeToLog = swprintf(message, L"[%d:%d:%d:%d:%d Delete Value] %s\r",
			t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds, infoDeleteValue->ValueName->Buffer);
	} break;

	case RegNtSetValueKey: {
		infoSetValue = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
		InitLogTime(&t);
		sizeToLog = swprintf(message, L"[%d:%d:%d:%d:%d Set Value] %s\r",
			t.hours, t.minutes, t.seconds, t.miliseconds, t.microseconds, infoSetValue->ValueName->Buffer);
	} break;
	}

	if (sizeToLog != -1) {
		status = ZwWriteFile(context->FileHandleLogRegistry, NULL, NULL, NULL, &statusBlock, message, sizeToLog * sizeof(WCHAR), NULL, NULL);
	}

	return status;
}