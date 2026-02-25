// CheckServicePermission.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
using namespace std;

int main(int argc, char* argv[])
{

	LoadLibraryA("D:\\tools\\SysinternalsSuite\\procdump.exe");
	cout << GetLastError() << endl;


	if (argc < 2) {
		cout << "Usage: CheckServicePermission.exe ServiceName" << endl;
		return 0;
	}
	cout << "Now checking the permission " << argv[1] << endl;
	
	
	SC_HANDLE sc_handle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (sc_handle != 0) {
		cout << "Success Open SCM SC_MANAGER_CONNECT " << endl;
	}
	else
	{
		cout << "The windows error code is " << GetLastError() << endl;
	}

	SC_HANDLE service_handle = OpenServiceA(
		sc_handle,
		argv[1],
		SERVICE_ALL_ACCESS
	);
	
	if (service_handle == NULL) {
		cout << "Failed to open service: " << GetLastError() << endl;
		return 1;
	}
	else
	{
		cout << "Success Open "<< argv[1] << " SERVICE_ALL_ACCESS " << endl;
		cout << "This "<< "\"" << argv[1] << "\"" << " server was vulnerable by Privilege escalation " << endl;
	}
}

