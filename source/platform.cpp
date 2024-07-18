#include "platform.h"

#ifdef _WIN32

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <malloc.h>


unsigned int GetCoresCount() {

    typedef BOOL(WINAPI* LPFN_GLPI)(
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
        PDWORD);

    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD processorCoreCount = 0;
    DWORD byteOffset = 0;

    glpi = (LPFN_GLPI)GetProcAddress( GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
    if (NULL == glpi){
        _tprintf(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
        return 0;
    }

    while (!done){
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER){
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
            }
            else {
                _tprintf(TEXT("\nError in GetCoresCount(): %d\n"), GetLastError());
                return 0;
            }
        }
        else{
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength){
        switch (ptr->Relationship) {

        case RelationProcessorCore:
            processorCoreCount++;
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buffer);

    return processorCoreCount;
}

#ifdef NVIDIA_GRAPHICS_CARD
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#elif AMD_GRAPHICS_CARD
extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif	//type of graphics card

#else       //_win32
#include <thread>

unsigned int GetCoresCount() {
    return std::thread::hardware_concurrency();
}


#endif