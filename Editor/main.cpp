#include "Windows/ProjectSelector.h"
#include "Windows/MainWindow.h"

#include "../Engine/Classes/StringConverter.h"

#ifdef _WIN32
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
	AllocConsole();
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	freopen("CON", "r", stdin);

	if (argc > 1 && argv[1] == CP_SYS("/d"))
		::ShowWindow(::GetConsoleWindow(), SW_SHOW);
	else
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif

	GX::ProjectSelector* projectSelector = new GX::ProjectSelector();

	if (!projectSelector->start())
	{
		GX::MainWindow* mainWindow = new GX::MainWindow();
		mainWindow->start();
	}

	return 0;
}