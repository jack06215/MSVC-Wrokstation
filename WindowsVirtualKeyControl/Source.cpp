#include <iostream>
#include <windows.h>
#include <winuser.h>
#include "SendKeys.h"


void waitForKey();

int main(char* argv[], int argc)
{
	std::string runApplication = "{DELAY=33}@rnotepad{ENTER}";
	CSendKeys keySender;
	//LPCTSTR commend = CA2W(runApplication.c_str());
	//keySender.AppActivate(_T("Windows Media Player"));
	//keySender.SendKeys(_T("{DELAY=200}Hello we{BS}orld!"));
	//keySender.SendKeys(_T("{DELAY=100}@rnotepad{ENTER}{appactivate Notepad}{DELAY 100}This is the sample code that demonstrate how to send keyboard to application%ha{ BEEP 1999 1000 }{ESC}% {DOWN 5}"));
	keySender.SendKeys(_T("{DELAY=100}@fseptember{DELAY=100}"));
	Sleep(60000);
	keySender.SendKeys(_T("^s"));
	
	//keySender.SendKeys(commend);
	return 0;
}

void waitForKey()
{
	std::cout << "Welcome to C++! Press the escape key to exit.\n";
	for (;;)
	{
		if (GetKeyState(VK_F1) == 1)
		{
			std::cout << "Goodbye!" << std::endl;
			exit(0);
		}
	}
}