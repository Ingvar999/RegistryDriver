/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_Registry,
    0x5a14a6bf,0xb021,0x4c94,0xb4,0xbf,0xb0,0x37,0x1b,0x64,0x6f,0x2f);
// {5a14a6bf-b021-4c94-b4bf-b0371b646f2f}