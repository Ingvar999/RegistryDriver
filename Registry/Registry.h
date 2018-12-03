#pragma once
#include <ntddk.h>

NTSTATUS WorkWithRegistry(IN WDFDEVICE);
NTSTATUS RegistryCleanup(IN WDFDEVICE);

EX_CALLBACK_FUNCTION ChangeRegistryCallback;