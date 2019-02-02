module DDllTest;

import core.sys.windows.windows;
import core.sys.windows.wincon;
import core.sys.windows.dll;
	
import std.stdio: writeln, stdout, freopen;
import core.memory;

__gshared HINSTANCE g_hInst;

shared bool testLoop = false;

// Main Thread function
void mainThread() {

	writeln("Just Teasing the Syren ;)...");
    while(testLoop) {
        writeln("Sleeping every second...");
		if(GetAsyncKeyState(VK_DELETE)) {
			writeln("[DLL_TEST]: <VK_DELETE> pressed. Exiting...");
			testLoop = false;
		}
        Sleep(1000); // Sleep Every Second
	}

	FreeLibraryAndExitThread(g_hInst, 0);
}

extern (Windows)
BOOL DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved) {

	final switch(ulReason) {
		
		case DLL_PROCESS_ATTACH:
		{
			GC.disable(); // We disable Garbage Collecion
			//DisableThreadLibraryCalls(hInstance); // I did this to try to fix DLL_THREAD_ATTACH
			//being called constantly but It didnt fix anything.
			dll_process_attach(hInstance, true);
			AllocConsole(); // We Allocate a Console
			// We get Standard output Capacities for the console we just allocated
			freopen("CONOUT$", "w", stdout.getFP());
			writeln("Successfully Injected DLL to process.");
			testLoop = true;
			auto main_thread = new Thread(&mainThread);
			main_thread.start();
			return TRUE;
		}
		break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH: // ? Why is this being called constantly?
			break;
		case DLL_THREAD_DETACH:
			break; 
	}

	g_hInst = hInstance;
	return TRUE;
}


