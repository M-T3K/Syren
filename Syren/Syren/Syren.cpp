#include <iostream>
#include <Windows.h>
#include <experimental/filesystem>
#include <TlHelp32.h>

#include "Definitions.h"

typedef struct {
    HANDLE   pHandle;
    wchar_t  name[MAX_PATH];
    DWORD    id;
} Process_t;

auto inline file_exists(const wchar_t *file) -> bool {

    std::wcout << "The file is: " << file << std::endl;
    return std::experimental::filesystem::exists(file);
}

auto validate_process(Process_t *proc) -> bool {

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if( snap == INVALID_HANDLE_VALUE) {

        return false;
    }

    if( Process32FirstW(snap, &entry)) {

        while(Process32NextW(snap, &entry)){
            
            std::wcout << "Process(" << entry.szExeFile << ")<" << proc->id << ", " << entry.th32ProcessID << ">" << std::endl;

            if( wcscmp(entry.szExeFile, proc->name) == 0) {

                proc->id = entry.th32ProcessID;

                proc->pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                std::cout << "pHandle := 0x" << proc->pHandle << std::endl;

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

    if( !OpenProcessToken(curr_process, TOKEN_ADJUST_PRIVILEGES, &token_handle)) {

        CloseHandle(curr_process); // Prevent memleak
        return false;
    }

    if( !LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {

        CloseHandle(token_handle);  // Prevent memleak
        CloseHandle(curr_process);
        return false;
    }

    new_privileges.PrivilegeCount           = 1;
    new_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    new_privileges.Privileges[0].Luid       = luid;

    BOOL adj = AdjustTokenPrivileges(token_handle, false, &new_privileges, sizeof(new_privileges), nullptr, nullptr);

    if( token_handle) {

        CloseHandle(token_handle);  // Prevent memleak
        CloseHandle(curr_process);
    }

    return adj;
}

auto inject_loadlibrary(Process_t *proc, wchar_t *path) -> bool {

    // Lower Scope Lambda to fix potential memory leaks when leaving HANDLEs open
    auto release_memory = [](HANDLE handle, LPVOID addressOfAlloc) -> void {

        if( addressOfAlloc) VirtualFreeEx(handle, addressOfAlloc, 0, MEM_RELEASE);
        if( handle) CloseHandle(handle);         
    };

    HANDLE remote_thread = {nullptr};
    HANDLE *proc_handle = &(proc->pHandle);
    DWORD exit_code = 0;
    size_t szPath = (wcslen(path) + 1) * sizeof(wchar_t *);

    auto dllPathAddress = VirtualAllocEx(proc->pHandle, nullptr, szPath, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if( !dllPathAddress) {
        
        release_memory(proc->pHandle, nullptr);
        return false;
    }

    if( !WriteProcessMemory(proc->pHandle, dllPathAddress, path, szPath, nullptr)) {

        release_memory(proc->pHandle, dllPathAddress);
        return false;
    }

    remote_thread = CreateRemoteThread(proc->pHandle, 
                                       nullptr,
                                       0,
                                       (LPTHREAD_START_ROUTINE)(LoadLibrary),
                                       dllPathAddress,
                                       0,
                                       nullptr);
    if( !remote_thread) {

        release_memory(proc->pHandle, dllPathAddress);
        return false;
    }

    if( WaitForSingleObject(remote_thread, 5000) == WAIT_OBJECT_0) {

        GetExitCodeThread(remote_thread, &exit_code);
    }

    if( remote_thread) {

        CloseHandle(remote_thread);
    }
    if( dllPathAddress) {

        // ? Do we really want to close proc->pHandle here?
        // Here are my thoughts: I am considering having an option to inject multiple DLLs in the future.
        // If that was the case, we would do well in keeping the Handle Open to avoid getting the same handle 
        // multiple times.
        // Until that is done/reconsidered, I will close the handle to prevent further memleaks.
        // -Kiwii
        // VirtualFreeEx(proc->pHandle, dllPathAddress, 0, MEM_RELEASE);
        release_memory(proc->pHandle, dllPathAddress);
    }
    
    return exit_code != 0;
}


// Syren.exe [-h] dll.dll proc.exe 
auto wmain(int argc, wchar_t *argv[]) -> int {


    // @info
    // These values are hardcoded. We dont really want that, but for the time being it gets the job done.
    // -Kiwii
    
    if(argc == 2 && wcscmp(argv[1], L"-h")) {

        std::cout << "Syren.exe [-h]: Prints Additional Information" << std::endl;
        std::cout << "Syren.exe [dll_name] [proc_name]: to inject <dll_name> into <proc_name>" << std::endl;            
        exit(STATUS_OK);
    }

    if(argc > 3) {

        std::cout << "Incorrect Number of Arguments" << std::endl;
        exit(ERROR_ARGS_INVALID);
    }

    Process_t proc = {0};
    wchar_t dll_path[MAX_PATH] = { 0 };

    {
        //@unsafe This is potentially unsafe
        auto tDLL      = (argc >= 2) ? argv[1] : L"C.dll";
        auto tPROCESS  = (argc == 3) ? argv[2] : L"csgo.exe";

        proc.id = 0;
        wcscpy_s(proc.name, sizeof(proc.name), tPROCESS);
        std::wcout << "Process Name :'" << proc.name << "'" << std::endl;
        proc.pHandle = {nullptr};
        _wfullpath(dll_path, tDLL, MAX_PATH);
    }

    gain_debug_privileges();

    if( !file_exists(dll_path)) {

        exit(ERROR_DLL_INVALID);
    }

    if(!validate_process(&proc)) {

        exit(ERROR_PROCESS_INVALID);
    }

    if( !inject_loadlibrary(&proc, dll_path)) {

        exit(ERROR_FAILED_INJECTION);
    }

    return STATUS_OK;
}










// // Syren.exe [-h] dll.dll proc.exe 
// // Freakin' Windows with its Wchar ...
// int wmain(int argc, wchar_t *argv[]) {

//     // argv[1] => dll
//     // argv[2] => proc

    
//     Process_t *proc = {0}; 
//     wcscpy_s(proc->name, sizeof(proc->name), L"csgo.exe");

//     const wchar_t *dll_name  = L"C.dll";

//     if( !gain_debug_privileges()) {

//         std::cerr << "Could not gain Debug Privilege. Maybe it's alread enabled." << std::endl;
//     }

//     if( !file_exists(dll_name)) {

//         std::cerr << "DLL could not be validated. Does it exist?" << std::endl;
//         exit(ERROR_DLL_INVALID);
//     }

//     if( !validate_process(proc)) {

//         std::cerr << "Process could not be validated. Is it running?" << std::endl;
//         exit(ERROR_PROCESS_INVALID);
//     }

//     wchar_t dll_path[MAX_PATH];
//     _wfullpath(dll_path, dll_name, MAX_PATH);

//     if( !inject(proc, dll_path)) {

//         std::cerr << "Injection Failed" << std::endl;
//         exit(ERROR_FAILED_INJECTION);
//     }

// 	std::cin.get();
// 	exit(STATUS_OK);
// }