module DDllTest;

import core.sys.windows.windows;
import core.sys.windows.dll;
import core.stdc.stdio;
import core.memory;

__gshared HINSTANCE g_hInst;

extern (Windows)
BOOL DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved) {

	switch(ulReason) {

		case DLL_PROCESS_ATTACH:
			GC.disable(); // We disable Garbage Collecion
			printf("Attaching DLL");
			g_hInst = hInstance;
			break;
		default:
			printf("Something went wrong...");
	}

	return TRUE;
}


