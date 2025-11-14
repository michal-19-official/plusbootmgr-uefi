//Plus boot mgr uefi version forked From KNNSpeed's "Simple UEFI Bootloader":
// https://github.com/KNNSpeed/Simple-UEFI-Bootloader
// V2.3 13.11 2025

#include "Bootloader.h"

STATIC CONST CHAR16 AppleFirmwareVendor[6] = L"Apple";
UINT8 IsApple = 0;

//==================================================================================================================================
//  efi_main: Main Function
//==================================================================================================================================
//
// Loader's "main" function. This bootloader's program entry point is defined as efi_main per UEFI application convention.
//

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  // ImageHandle is this program's own EFI_HANDLE
  // SystemTable is the EFI system table of the machine

  // Initialize the GNU-EFI library
  InitializeLib(ImageHandle, SystemTable);
/*
  From InitializeLib:

  ST = SystemTable;
  BS = SystemTable->BootServices;
  RT = SystemTable->RuntimeServices;

*/
  EFI_STATUS Status;

  // Do a preliminary screen clear, always
  Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  if(EFI_ERROR(Status))
  {
    Print(L"NOTE: Could not clear the screen, so there may be some system text above this line.\r\n");
  }

#ifdef DISABLE_UEFI_WATCHDOG_TIMER
  // Disable watchdog timer for debugging
  Status = BS->SetWatchdogTimer (0, 0, 0, NULL);
  if(EFI_ERROR(Status))
  {
    Print(L"Error stopping watchdog, timeout still counting down...\r\n");
  }
#endif

  // Thanks to the rEFIt 0.14 project for having figured this out long ago. ConsoleControlProtocol is needed to switch Mac boot graphics to text mode.
  if(compare(ST->FirmwareVendor, AppleFirmwareVendor, 6))
  {
    IsApple = 1;
    EFI_CONSOLE_CONTROL_PROTOCOL * ConsoleMode = NULL;
    Status = LibLocateProtocol(&ConsoleControlProtocol, (VOID**)&ConsoleMode); // Find the handle that corresponds to this protocol. There's only 1.
    if(EFI_ERROR(Status))
    {
      ConsoleMode = NULL;

  #ifdef MAIN_DEBUG_ENABLED
      Print(L"Console Control Protocol not located. It may not be supported.\r\n");
  #endif
    }

    if(ConsoleMode != NULL)
    {
      EFI_CONSOLE_CONTROL_SCREEN_MODE Current_Mode;

      ConsoleMode->GetMode(ConsoleMode, &Current_Mode, NULL, NULL);

      if(Current_Mode != EfiConsoleControlScreenText)
      {
  #ifdef MAIN_DEBUG_ENABLED
        Print(L"Console control protocol located & now setting text mode...\r\n");
  #endif

        ConsoleMode->SetMode(ConsoleMode, EfiConsoleControlScreenText);

  #ifdef MAIN_DEBUG_ENABLED
        Print(L"Text mode set.\r\n");
  #endif
      }
  #ifdef MAIN_DEBUG_ENABLED
      else
      {
        Print(L"Output already in text mode.\r\n");
      }
  #endif
    }
  }
  // End text mode

  // Print out general system info
  EFI_TIME Now;
  Status = RT->GetTime(&Now, NULL);
  if(EFI_ERROR(Status))
  {
    Print(L"Error getting time...\r\n");
    return Status;
  }

  Print(L"%02hhu/%02hhu/%04hu - %02hhu:%02hhu:%02hhu.%u\r\n\n", Now.Month, Now.Day, Now.Year, Now.Hour, Now.Minute, Now.Second, Now.Nanosecond); // GNU-EFI apparently has a print function for time... Oh well.
#ifdef MAIN_DEBUG_ENABLED
  #ifdef MEMORY_DEBUG_ENABLED // Very slow memory debug version
    Print(L"plus boot mgr - V%u.%u DEBUG (Memory)\r\n", MAJOR_VER, MINOR_VER);
  #else // Standard debug version
    Print(L"plus boot mgr - V%u.%u DEBUG\r\n", MAJOR_VER, MINOR_VER);
  #endif
#else
  #ifdef FINAL_LOADER_DEBUG_ENABLED // Lite debug version
    Print(L"plus boot mgr - V%u.%u DEBUG (Lite)\r\n", MAJOR_VER, MINOR_VER);
  #else // Release version
    Print(L"plus boot mgr - V%u.%u\r\n", MAJOR_VER, MINOR_VER);
  #endif
#endif
  Print(L"copyright (c) 2025 michal19\r\n");
  Print(L"Original copyright (c) 2017-2019 KNNSpeed\r\n\n");
  Print(L"For software licensing information and related usage terms, please refer to the LICENSE file found at https://github.com/michal-19-official/plusbootmgr-uefi.\r\n\n");

  

#ifdef MAIN_DEBUG_ENABLED
  Print(L"EFI System Table Info\r\n   Signature: 0x%lx\r\n   UEFI Revision: 0x%08x\r\n   Header Size: %u Bytes\r\n   CRC32: 0x%08x\r\n   Reserved: 0x%x\r\n", ST->Hdr.Signature, ST->Hdr.Revision, ST->Hdr.HeaderSize, ST->Hdr.CRC32, ST->Hdr.Reserved);
  Print(L"   Firmware Vendor: %s\r\n   Firmware Revision: 0x%08x\r\n\n", ST->FirmwareVendor, ST->FirmwareRevision);

  // Configuration table info
  Print(L"%llu system configuration tables are available.\r\n", ST->NumberOfTableEntries);
#endif


#ifdef MAIN_DEBUG_ENABLED
  Keywait(L"\0");

  // Search for ACPI tables
  UINT8 RSDPfound = 0;
  UINTN RSDP_index = 0;

  // This print is for debugging
  for(UINTN i=0; i < ST->NumberOfTableEntries; i++)
  {
    Print(L"Table %llu GUID: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", i,
            ST->ConfigurationTable[i].VendorGuid.Data1,
            ST->ConfigurationTable[i].VendorGuid.Data2,
            ST->ConfigurationTable[i].VendorGuid.Data3,
            ST->ConfigurationTable[i].VendorGuid.Data4[0],
            ST->ConfigurationTable[i].VendorGuid.Data4[1],
            ST->ConfigurationTable[i].VendorGuid.Data4[2],
            ST->ConfigurationTable[i].VendorGuid.Data4[3],
            ST->ConfigurationTable[i].VendorGuid.Data4[4],
            ST->ConfigurationTable[i].VendorGuid.Data4[5],
            ST->ConfigurationTable[i].VendorGuid.Data4[6],
            ST->ConfigurationTable[i].VendorGuid.Data4[7]);

    if (compare(&ST->ConfigurationTable[i].VendorGuid, &Acpi20TableGuid, 16))
    {
      Print(L"RSDP 2.0 found!\r\n");
      RSDP_index = i;
      RSDPfound = 2;
    }
  }
  // If no RSDP 2.0, check for 1.0
  if(!RSDPfound)
  {
    for(UINTN i=0; i < ST->NumberOfTableEntries; i++)
    {
      if (compare(&ST->ConfigurationTable[i].VendorGuid, &AcpiTableGuid, 16))
      {
        Print(L"RSDP 1.0 found!\r\n");
        RSDP_index = i;
        RSDPfound = 1;
      }
    }
  }

  if(!RSDPfound)
  {
    Print(L"System has no RSDP.\r\n");
  }

  Keywait(L"\0");

  // View memmap before too much happens to it
  print_memmap();
  Keywait(L"Done printing MemMap.\r\n");
#endif

  // Create graphics structure
  GPU_CONFIG *Graphics;
  Status = ST->BootServices->AllocatePool(EfiLoaderData, sizeof(GPU_CONFIG), (void**)&Graphics);
  if(EFI_ERROR(Status))
  {
    Print(L"Graphics AllocatePool error. 0x%llx\r\n", Status);
    return Status;
  }

#ifdef MAIN_DEBUG_ENABLED
  Print(L"Graphics struct allocated\r\n");
#endif

  // Set up graphics
  Status = InitUEFI_GOP(ImageHandle, Graphics);
  if(EFI_ERROR(Status))
  {
    Print(L"InitUEFI_GOP error. 0x%llx\r\n", Status);
    Keywait(L"\0");
    return Status;
  }

#ifdef MAIN_DEBUG_ENABLED
  Keywait(L"InitUEFI_GOP finished.\r\n");

  // Data verification
  Print(L"Config table address: 0x%llx\r\n", ST->ConfigurationTable);
  Print(L"Data at RSDP (first 16 bytes): 0x%016llx%016llx\r\n", *(EFI_PHYSICAL_ADDRESS*)(((UINT64)ST->ConfigurationTable[RSDP_index].VendorTable) + 8), *(EFI_PHYSICAL_ADDRESS*)ST->ConfigurationTable[RSDP_index].VendorTable);
#endif

  // Load a program and exit boot services, then pass a loader block to that program's entry point to execute the program
  Status = GoTime(ImageHandle, Graphics, ST->ConfigurationTable, ST->NumberOfTableEntries, ST->Hdr.Revision);

  // Pause to evaluate any errors
  Keywait(L"GoTime returned...\r\n");
  return Status;
}

//==================================================================================================================================
//  Keywait: Pause
//==================================================================================================================================
//
// A simple pause function that waits for user input before continuing.
// Adapted from http://wiki.osdev.org/UEFI_Bare_Bones
//
// Note: Does not take format modifier arguments like %s, %d, etc., only plain strings.
//

EFI_STATUS Keywait(CHAR16 *String)
{
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;
  Print(String);

  Status = ST->ConOut->OutputString(ST->ConOut, L"Press any key to continue...");
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  // Clear keystroke buffer
  Status = ST->ConIn->Reset(ST->ConIn, FALSE);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  // Poll for key
  while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);

  // Clear keystroke buffer (this is just a pause)
  Status = ST->ConIn->Reset(ST->ConIn, FALSE);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Print(L"\r\n");

  return Status;
}
