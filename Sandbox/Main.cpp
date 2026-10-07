#include "pch.h"
#include "SandboxApp.h"

#include <Windows.h>

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE,
	PWSTR,
	int showCommand)
{
	SandboxApp app{ instance };
	return app.Run(showCommand);
}
