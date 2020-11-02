// dllmain.cpp : Define o ponto de entrada para da aplicação DLL.
// Objective: Complete resolution/window size/mode upgrade for NFS U2 with configuration on the game menu
// Unfinished mod
#include <injector\assembly.hpp>
#include <injector\calling.hpp>
#include <injector\hooking.hpp>
#include <injector\utility.hpp>
#include <injector\injector.hpp>
#include <fstream>
#include <vector>

auto sub_50C900 = injector::cstd<uint32_t(uint32_t, uint32_t)>::call<0x0050C900>;
auto sub_537BE0 = injector::cstd<uint32_t(uint32_t, const char*, uint32_t)>::call<0x00537BE0>;
uint32_t sub_537BE0_asm = 0x00537BE0;
uint32_t &resolutionID = *(uint32_t*)0x00870D1C;
uint8_t &menuContinue = *(uint8_t*)0x008709D1;

//int resWidth[] = { 640, 800, 1024, 1280, 1280, 1360, 1366, 1920 };
//int resHeight[] = { 480, 600, 768, 768, 1024, 768, 768, 1080 };
//int resSupported[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//int resNum = sizeof(resWidth) / sizeof(int);
std::fstream resOverrideLog("res.log", std::ios::out | std::ios::trunc);

struct videoModes
{
	int width, height, refreshRate;
};

std::vector<DISPLAY_DEVICE> getDisplays()
{
	std::vector<DISPLAY_DEVICE> result;

	for (int deviceNum = 0; /* */; deviceNum++)
	{
		DISPLAY_DEVICE disp;
		ZeroMemory(&disp, sizeof(disp));
		disp.cb = sizeof(DISPLAY_DEVICE);

		if (EnumDisplayDevices(nullptr, deviceNum, &disp, 0))
		{
			result.push_back(disp);
		}
		else
		{
			break;
		}
	}

	return result;
}

std::vector<videoModes> getVideoModes()
{
	std::vector<videoModes> result;

	auto disps = getDisplays();

	for (int modeIndex = 0; /* */; modeIndex++)
	{
		DEVMODEW dm;

		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);
		
		if (!EnumDisplaySettingsW(disps[0].DeviceName, modeIndex, &dm))
			break;

		videoModes mode;

		mode.width = dm.dmPelsWidth;
		mode.height = dm.dmPelsHeight;
		mode.refreshRate = dm.dmDisplayFrequency;

		bool add = true;

		for (auto &m : result)
		{
			if (m.height == mode.height && mode.width == m.width)
			{
				add = false;
				break;
			}
		}

		if (add)
			result.push_back(mode);
	}

	return result;
}

std::vector<videoModes> videoResAvail;

void __fastcall resolutionOverrideHookCpp(uintptr_t thisptr, uint32_t, uint32_t v3)
{
	sub_50C900(*(uint32_t*)(thisptr + 44), 0xA9A76BBA);
	auto &aaa = *(uint32_t*)(thisptr + 48);

	static char buffer[128];

	auto &mode = videoResAvail[resolutionID];

	sprintf(buffer, "%d%s%d", mode.width, "x", mode.height);

	sub_537BE0(aaa, buffer, v3);
}

void __declspec(naked) windowedHook()
{
	__asm
	{
		pushad
		pushfd

		mov edx, 0x0086F890
		mov[edx], 1


		popfd
		popad
		ret
	}
}

void *temp;

void *potassio;

void fun()
{

	{
		char buffer[256];

		int result = sprintf(buffer, "%.8X\n", potassio);

		resOverrideLog.write(buffer, result);
		resOverrideLog.flush();
	}

	static char buffer[128];

	auto &mode = videoResAvail[resolutionID];

	sprintf(buffer, "%d%s%d", mode.width, "x", mode.height);

	temp = buffer;
}

void menuContinueTest()
{
	if (resolutionID < videoResAvail.size())
	{
		menuContinue = 1;
	}
	else
	{
		resolutionID = videoResAvail.size() - 1;
	}
}

void __declspec(naked) hookMenuContinue()
{
	__asm
	{
		pushad
		pushfd

		call menuContinueTest

		popfd
		popad
		retn
	}
}

void __declspec(naked) resolutionOverrideHook()
{
	__asm
	{
		mov eax, [esp+4]
		mov potassio, eax
		mov     ecx, [esi + 30h]

		pushad
		pushfd

		call fun

		popfd
		popad

		push temp
		push ecx

		call sub_537BE0_asm

		add     esp, 8
		pop     esi
		retn
	}

}

void __stdcall resolutionSelectHook(int *a1, int *a2)
{
	auto &mode = videoResAvail[resolutionID];
	*a1 = mode.width;
	*a2 = mode.height;
}

void hook()
{
	DWORD old;
	injector::UnprotectMemory(0x0049B690, 128, old);
	injector::MakeNOP(0x0049B6A9, 0x0049B6B5 - 0x0049B6A9);
	//injector::MakeNOP(0x0049B66F, 0x0049B67A - 0x0049B66F);
	//injector::MakeNOP(0x0049B655, 0x0049B65C - 0x0049B655);
	injector::MakeNOP(0x005BF610, 0x005BF625 - 0x005BF610);
	injector::MakeNOP(0x0049B655, 0x0049B66F - 0x0049B655);
	injector::MakeNOP(0x0049B620, 0x0049B633 - 0x0049B620);
	injector::MakeJMP(0x0049B6A9, resolutionOverrideHook);
	injector::WriteMemory<uint8_t>(0x0049B620, 0x40, true);
	injector::MakeJMP(0x0049B621, 0x0049B650);
	//injector::MakeCALL(0x0049B66F, hookMenuContinue);
	//injector::MakeJMP(0x0049B655, 0x0049B66F);
	injector::MakeJMP(0x005BF610, resolutionSelectHook);
	injector::WriteMemory<uint8_t>(0x0049B614+2, videoResAvail.size() - 1, true);
	//injector::WriteMemory(0x00791F3C, resolutionOverrideHook);

	videoResAvail = getVideoModes();



	/*
	* Windowed hook
	*/

	injector::MakeNOP(0x005B9AAB, 6);

	injector::MakeCALL(0x005B9AAB, windowedHook);

	const LONG windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX*/;

	LONG l = injector::ReadMemory<LONG>(0x005D279A + 1, true);
	l |= windowStyle;
	injector::WriteMemory<LONG>(0x005D279A + 1, l, true);

	injector::WriteMemory<LONG>(0x005D2664 + 1, windowStyle, true);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hook();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

