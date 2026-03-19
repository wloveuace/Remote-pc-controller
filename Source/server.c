#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <winsock.h>
#include <powrprof.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib , "PowrProf.lib")

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

BOOL ServerState = TRUE;

typedef struct {
    SOCKET clientSock;
    char buffer[2048];
} CLIENT_IO;

typedef struct {
    HANDLE file;
    DWORD size;
    char* data;
} FILE_INFO;


void cleanupFile(FILE_INFO* fi) {
    if (!fi) return;

    if (fi->file && fi->file != INVALID_HANDLE_VALUE)
        CloseHandle(fi->file);

    if (fi->data)
        free(fi->data);

    fi->file = INVALID_HANDLE_VALUE;
    fi->data = NULL;
}

SOCKET initializeServerSocket(void) {
    SOCKET serverSock;
    sockaddr_in serverInfo;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        printf("Failed to create server socket\n");
        return INVALID_SOCKET;
    }

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(80);
    serverInfo.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        printf("Failed to bind server");
        closesocket(serverSock);
        return INVALID_SOCKET;
    }

    if (listen(serverSock, 20) == SOCKET_ERROR) {
        printf("Failed to listen");
        closesocket(serverSock);
        return INVALID_SOCKET;
    }

    printf("[+] Server started on port 80\n");
    return serverSock;
}

void sendHttpResponse(SOCKET clientSock, const char* content, const char* body, int length) {
    char header[512];

    sprintf(header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        content,
        length);

    send(clientSock, header, (int)strlen(header), 0);
    send(clientSock, body, length, 0);
}

int serverReadFile(const char* filename, FILE_INFO* fileInfo) {
    HANDLE file = INVALID_HANDLE_VALUE;
    char* data = NULL;

    file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        printf("couldnt get file handle");
        return 0;
    }

    DWORD fSz = GetFileSize(file, NULL);
    if (fSz == INVALID_FILE_SIZE)
        goto FAIL;

    data = (char*)malloc(fSz);
    if (!data)
        goto FAIL;

    DWORD bytesRead = 0;
    if (!ReadFile(file, data, fSz, &bytesRead, NULL) || bytesRead != fSz)
        goto FAIL;

    fileInfo->file = file;
    fileInfo->size = fSz;
    fileInfo->data = data;

    return 1;

FAIL:
    if (data) free(data);
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    return 0;
}

void handleRequest(SOCKET clientSock, const char* request) {
    FILE_INFO fi = { 0 };

    if (strstr(request, "GET / ") || strstr(request, "GET /HTTP")) {
        if (!serverReadFile("index.html", &fi)) {
            printf("Error reading file");
            return;
        }

        sendHttpResponse(clientSock, "text/html", fi.data, fi.size);
        cleanupFile(&fi);
        return;
    }

    if (strstr(request, "GET /style.css")) {
        if (!serverReadFile("style.css", &fi)) {
            printf("Error reading file");
            return;
        }

        sendHttpResponse(clientSock, "text/css", fi.data, fi.size);
        cleanupFile(&fi);
        return;
    }

    if (strstr(request, "GET /script.js")) {
        if (!serverReadFile("script.js", &fi)) {
            printf("Error reading file");
            return;
        }

        sendHttpResponse(clientSock, "application/javascript", fi.data, fi.size);
        cleanupFile(&fi);
        return;
    }

    // ------------------------

    if (strstr(request, "GET /mute")) {
        printf("Muting pc....\n");
        system("setvol 0");
    }

    if (strstr(request, "GET /sleep")) {
        SetSuspendState(0, 0, 0);
    }

    if (strstr(request, "GET /hutdown")) {
        SetSuspendState(0, 0, 1);
    }

    if (strstr(request, "GET /volume")) {
        int volume;
        char command[100];
        sscanf(request, "GET /volume?value=%d", &volume);
        sprintf(command, "setvol %d", volume);

        system(command);
    }
}

void clientHandlerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work) {
    CLIENT_IO* clientIo = (CLIENT_IO*)Context;

    if (!clientIo)
        return;

    int bytes = recv(clientIo->clientSock, clientIo->buffer, sizeof(clientIo->buffer) - 1, 0);

    if (bytes > 0) {
        clientIo->buffer[bytes] = '\0';
        printf("[Request]\n%s\n", clientIo->buffer);

        handleRequest(clientIo->clientSock, clientIo->buffer);
    }

    if (clientIo->clientSock != INVALID_SOCKET)
        closesocket(clientIo->clientSock);

    free(clientIo);
}

void serverStartService(SOCKET serverSock) {
    PTP_POOL tpPool = CreateThreadpool(NULL);
    if (!tpPool) {
        printf("Failed to create thread pool\n");
        return;
    }

    SetThreadpoolThreadMaximum(tpPool, 50);
    SetThreadpoolThreadMinimum(tpPool, 5);

    while (ServerState) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            printf("Failed to accept client: %d\n", WSAGetLastError());
            continue;
        }

        printf("[+] Client connected: %s:%d\n",
            inet_ntoa(clientAddr.sin_addr),
            ntohs(clientAddr.sin_port));

        CLIENT_IO* clientIo = (CLIENT_IO*)malloc(sizeof(CLIENT_IO));
        if (!clientIo) {
            closesocket(clientSock);
            continue;
        }

        clientIo->clientSock = clientSock;

        PTP_WORK work = CreateThreadpoolWork(clientHandlerCallback, clientIo, NULL);
        if (!work) {
            printf("Failed to create work item\n");
            closesocket(clientSock);
            free(clientIo);
            continue;
        }

        SubmitThreadpoolWork(work);
        CloseThreadpoolWork(work);
    }

    CloseThreadpool(tpPool);
}

int main() {
    WSADATA wsData;

    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET serverSock = initializeServerSocket();
    if (serverSock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    serverStartService(serverSock);

    if (serverSock != INVALID_SOCKET)
        closesocket(serverSock);

    WSACleanup();
    return 0;
}