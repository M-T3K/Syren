#include <iostream>
#include <Windows.h>
#include <experimental/filesystem>
#include <TlHelp32.h>
#include <fstream>
#include <ios>

#include "Definitions.h"
#include "fmt.hpp"

#define __DEFAULT_DLL       L""
#define __DEFAULT_PROCESS   L""
#define __DEFAULT_LOGFILE   L"Syren.log"

// For InjectionFlags_
typedef uint8_t InjectionFlags;

enum InjectionFlags_ {

    InjectionFlags_NoFlags      = 0,
    InjectionFlags_Verbose      = 1,
    InjectionFlags_LogFile     = 1 << 1,
    InjectionFlags_Experimental = 1 << 2

};

// Global since it will be used in all functions

InjectionFlags Flags = InjectionFlags_NoFlags;  // No Flags yet
std::fstream   LogFile; 

typedef struct {
    HANDLE   pHandle;
    wchar_t  name[MAX_PATH];
    DWORD    id;
} Process_t;

auto inline file_exists(const wchar_t *file) -> bool {

    if( Flags & InjectionFlags_Verbose) printf("Checking if %ws exists...\n", file);
    if( Flags & InjectionFlags_Experimental) return std::experimental::filesystem::exists(file);
    
    std::ifstream input{file};
    return input.good();
}

auto validate_process(Process_t *proc) -> bool {

    if( Flags & InjectionFlags_Verbose) printf("Checking if %ws is a valid process...\n", proc->name);

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if( snap == INVALID_HANDLE_VALUE) {
        
        printf("Handle is Invalid...\n");
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Couldn't get a Handle to the Snapshot of processes.");
        return false;
    }

    if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Beginning Lookup for Process '%ws' using HANDLE 0x%p.", proc->name, snap);

    if( Process32FirstW(snap, &entry)) {

        while(Process32NextW(snap, &entry)){
            
            if( Flags & InjectionFlags_Verbose) printf("Process(%ws)<%d>\n", entry.szExeFile, entry.th32ProcessID);
            if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Current Process :: <exe='%ws', id='%d'>.", entry.szExeFile, entry.th32ProcessID);

            if( wcscmp(entry.szExeFile, proc->name) == 0) {

                proc->id = entry.th32ProcessID;

                proc->pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                // @Check if there's an error when handle is nullptr. I assume printf is capable of handling all these things.
                printf("Found Process(%ws)<%d> (with Handle 0x%p).\n", proc->name, proc->id, proc->pHandle);

                if(proc->pHandle == nullptr && (Flags & InjectionFlags_LogFile)) fmt::fprintLn(LogFile, "Found Process '%ws'<id='%d'> but created HANDLE was invalid.", proc->name, proc->id);
                return proc->pHandle != nullptr; 
            }
        }
        
    }

    return false;
}

auto gain_debug_privileges() -> bool {

    HANDLE curr_process = GetCurrentProcess();
    HANDLE token_handle;
    LUID luid;
    TOKEN_PRIVILEGES new_privileges;

    if( Flags & InjectionFlags_Verbose) printf("Opening Current Process...\n");

    if( !OpenProcessToken(curr_process, TOKEN_ADJUST_PRIVILEGES, &token_handle)) {
        
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Couldn't Open Tokens with HANDLE 0x%p.", curr_process);
        CloseHandle(curr_process); // Prevent memleak
        return false;
    }

    if( Flags & InjectionFlags_Verbose) printf("Looking for SE_DEBUG privileges\n");
    if( !LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {

        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Couldn't find Privilege Value for SE_DEBUG with HANDLE 0x%p.", curr_process);
        
        CloseHandle(token_handle);  // Prevent memleak
        CloseHandle(curr_process);
        return false;
    }

    if( Flags & InjectionFlags_Verbose) printf("Adjusting privileges: enabling debug privilege...\n");

    new_privileges.PrivilegeCount           = 1;
    new_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    new_privileges.Privileges[0].Luid       = luid;

    BOOL adj = AdjustTokenPrivileges(token_handle, false, &new_privileges, sizeof(new_privileges), nullptr, nullptr);

    if( Flags & InjectionFlags_Verbose) printf("Closing Open Handles...\n");

    if( !adj && Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Couldn't adjust Token Privileges. PROCESS HANDLE: 0x%p, TOKEN HANDLE: 0x%p.", curr_process, token_handle);

    if( token_handle) {

        CloseHandle(token_handle);  // Prevent memleak
        CloseHandle(curr_process);
    }
    if( Flags & InjectionFlags_Verbose) printf("Done!\n");

    return adj;
}

auto injectLoadlibrary(Process_t *proc, wchar_t *path) -> bool {

    // Lower Scope Lambda to fix potential memory leaks when leaving HANDLEs open
    auto release_memory = [](HANDLE handle, LPVOID addressOfAlloc) -> void {

        if( addressOfAlloc) VirtualFreeEx(handle, addressOfAlloc, 0, MEM_RELEASE);
        if( handle) CloseHandle(handle);         
    };

    HANDLE remote_thread = {nullptr};
    DWORD exit_code = 0;
    size_t sz_path = (wcslen(path) + 1) * sizeof(wchar_t *);
    // Kernel32.dll -> LoadLibrary (This is unnecessary it seems)
    //auto load_library_address = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

    if( Flags & InjectionFlags_Verbose) printf("Allocating space for the DLL Path...\n");
    auto dllpath_address = VirtualAllocEx(proc->pHandle, nullptr, sz_path, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if( !dllpath_address) {
        
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Couldn't allocate memory for the path to the DLL.");
        release_memory(proc->pHandle, nullptr);
        return false;
    }

    if( Flags & InjectionFlags_Verbose) printf("Successfully allocated dll path at 0x%p\nWritting process memory...", dllpath_address);

    if( !WriteProcessMemory(proc->pHandle, dllpath_address, path, sz_path, nullptr)) {

        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Failed to write the path to the Dll (%ws) to memory at address 0x%p.", path, dllpath_address);
        release_memory(proc->pHandle, dllpath_address);
        return false;
    }

    if( Flags & InjectionFlags_Verbose) printf("Done!\nCreating Remote Thread to the process (With Handle 0x%p)\n", proc->pHandle);

    remote_thread = CreateRemoteThread(proc->pHandle, 
                                       nullptr,
                                       0,
                                       (LPTHREAD_START_ROUTINE) LoadLibrary,
                                       dllpath_address,
                                       0,
                                       nullptr);
    if( !remote_thread) {

        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Failed to create a Remote Thread to the process :: '%ws'<HANDLE='0x%p', id='%d'> at memory address 0x%p.", proc->name, proc->pHandle, proc->id, dllpath_address);
		std::cerr << GetLastError() << std::endl;
        release_memory(proc->pHandle, dllpath_address);
        return false;
    }

    if( Flags & InjectionFlags_Verbose) printf("Done!\n");


    if( WaitForSingleObject(remote_thread, 5000) == WAIT_OBJECT_0) {

        GetExitCodeThread(remote_thread, &exit_code);
    }

    if( Flags & InjectionFlags_Verbose) printf("Closing Handles and releasing memory...\n");

    if( remote_thread) {

        CloseHandle(remote_thread);
    }
    if( dllpath_address) {

        // ? Do we really want to close proc->pHandle here?
        // Here are my thoughts: I am considering having an option to inject multiple DLLs in the future.
        // If that was the case, we would do well in keeping the Handle Open to avoid getting the same handle 
        // multiple times.
        // Until that is done/reconsidered, I will close the handle to prevent further memleaks.
        // -Kiwii
        // VirtualFreeEx(proc->pHandle, dllpath_address, 0, MEM_RELEASE);
        release_memory(proc->pHandle, dllpath_address);
    }

    if( Flags & InjectionFlags_Verbose) printf("Done!\n");    
    return exit_code != 0;
}

auto injectManualMap() -> bool;


// Syren.exe [-h] dll.dll proc.exe 
auto wmain(int argc, wchar_t *argv[]) -> int {

    printf("argc = %d\n", argc);
    if(argc == 2 && ((wcscmp(argv[1], L"-h") == 0) || (wcscmp(argv[1], L"--help") == 0))) {

        printf("USAGE: Syren.exe [-flags] [-d/--dll [name.dll]] [ -p/--process [ProcessExeFile]]\n");
        printf("List of [-flags]:\n");
        printf("\t[-h/--help]:             Prints helpful information about Syren and its usage.\n"); 
        printf("\t[-e/--experimental]:     Makes the program use experimental C++17/C++2a features. By default, Syren uses normal C++14 features.\n");          
        printf("\t[-v/--verbose]:          Prints additional information to the console whenever possible.\n");
        printf("\t[-l/--log]:              If using this option, Syren will log all information inside a log file. By default it is [Syren.log]. The Longer version is [--log].\n");          
        printf("\t[-d/--dll]:              Identifies the next argument as the DLL to inject. The longer version is [--dll].\n");
        printf("\t[-p/--process]:          Identifies the next argument as the Target Process. The longer version is [--process].\n");
        exit(STATUS_OK);
    }

    // @info
    // These values are hardcoded. We dont really want that, but for the time being it gets the job done.
    // -Kiwii

    auto dll_idx  = 0;
    auto proc_idx = 0;
    auto log_idx  = 0;

    // Parsing Command Line Arguments

    for( auto i = 1; i < argc; ++i) {

        if((wcscmp(argv[i], L"-v") == 0) || (wcscmp(argv[i], L"--verbose") == 0)) {
            
            Flags |= InjectionFlags_Verbose;
        }        
        else if((wcscmp(argv[i], L"-e") == 0) || (wcscmp(argv[i], L"--experimental") == 0)) {

            Flags |= InjectionFlags_Experimental;
        }
        else if((wcscmp(argv[i], L"-l") == 0) || (wcscmp(argv[i], L"--log") == 0)) {

            Flags |= InjectionFlags_LogFile;

            if (argc > i + 1 && (wcsncmp(argv[i + 1], L"-", 1) != 0)) {
                
                log_idx = ++i;
            }

        }
        else {

            try {

                if((i + 1 < argc)) {

                    // For now we only Inject one dll, to at most 1 process
                    // As long as this doesnt change we gucci Hardcoding a value of 1.
                    if(((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"--dll") == 0)) && (dll_idx == 0)) {

                        dll_idx = ++i;
                        // printf("arg%d = %ws\n", i, argv[i]);
                        if( argc > i && (wcsncmp(argv[i], L"-", 1) == 0)) {

                            throw fmt::error("Expected argument %i (%ws) to refer to a file's name but received a flag.", i, argv[i]);
                        }
                    }
                    else if(((wcscmp(argv[i], L"-p") == 0) || (wcscmp(argv[i], L"--process") == 0)) && (proc_idx == 0)) {
                        
                        proc_idx = ++i;
                        // printf("arg%d = %ws\n", i, argv[i]);
                        if( argc > i && (wcsncmp(argv[i], L"-", 1) == 0)) {

                            throw fmt::error("Expected argument %i (%ws) to refer to a process' executable but received a flag.", i, argv[i]);
                        }
                    }
                }
                else {

                    throw fmt::error("Argument %i (%ws) is Invalid.", i, argv[i]);
                }

            } catch(const fmt::error& err) {

                printf("ERROR: ERROR_ARGS_INVALID -> %s\n", err.what());
                exit(ERROR_ARGS_INVALID);
            }

        }
        
    }

    // END OF cl-args parsing

    Process_t proc = {0};
    wchar_t dll_path[MAX_PATH] = { 0 };


    {
        //@unsafe This is potentially unsafe
        auto tDLL     = (dll_idx  != 0) ? argv[dll_idx]  : __DEFAULT_DLL;
        auto tPROCESS = (proc_idx != 0) ? argv[proc_idx] : __DEFAULT_PROCESS;
        auto tLOG     = (log_idx  != 0) ? argv[log_idx]  : __DEFAULT_LOGFILE;
        if( Flags & InjectionFlags_LogFile) LogFile.open(tLOG, std::ios::out | std::ios::app);

        if( Flags & InjectionFlags_Verbose) printf("DLL Selected: %ws\nProcess Selected: %ws\n", tDLL, tPROCESS);

        proc.id = 0;
        wcscpy_s(proc.name, sizeof(proc.name), tPROCESS);
        proc.pHandle = {nullptr};

        if( Flags & InjectionFlags_Experimental) {
			
            if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Using experimental C++ features.");

			using namespace std::experimental::filesystem;

            path p {tDLL};
            wcscpy_s(dll_path, absolute(p).c_str());
        }
        else _wfullpath(dll_path, tDLL, MAX_PATH);
    }

    printf("DLL Complete Path: %ws\nTarget Process: %ws\n", dll_path, proc.name);

    if( Flags & InjectionFlags_Verbose) printf("Attempting to gain debug Privileges...\n");
    if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "DLL Complete Path: %ws\nTarget Process: %ws\nAttempting to gain Debug Privileges...", dll_path, proc.name);

    if( !gain_debug_privileges()) {

        printf("ERROR: Could not obtain debug privileges.");
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "ERROR: Could not obtain debug privileges.");
		exit(ERROR_DEBUG_PRIVILEGES_MISSING);
    }

    if( !file_exists(dll_path)) {

		printf("File %ws does not exist.\n", dll_path);
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "ERROR: File %ws does not exist.", dll_path);
        exit(ERROR_DLL_INVALID);
    }

    if(!validate_process(&proc)) {

		printf("Process %ws could not be validated. Is it running?\n", proc.name);
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "ERROR: Process %ws could not be validated.", proc.name);
        exit(ERROR_PROCESS_INVALID);
    }

    if( Flags & InjectionFlags_Verbose) printf("Injecting...\n");
    if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "Injecting %ws to target process<%ws, %d>.", dll_path, proc.name, proc.id);

    if( !injectLoadlibrary(&proc, dll_path)) {

		printf("[LoadLibrary]: Injection of %ws to process %ws with id %d could not be completed.\n", dll_path, proc.name, proc.id);
        if( Flags & InjectionFlags_LogFile) fmt::fprintLn(LogFile, "ERROR: Injection of %ws to target process<%ws, %d> failed.", dll_path, proc.name, proc.id);        
        exit(ERROR_FAILED_INJECTION);
    }

    printf("DLL: %ws Successfully Injected to Process<%ws, %d>.\n", dll_path, proc.name, proc.id);

    if(LogFile.is_open()) LogFile.close();    // Avoid memleak
    
    return STATUS_OK;
}
