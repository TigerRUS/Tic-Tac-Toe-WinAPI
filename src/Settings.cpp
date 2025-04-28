#include "Settings.h"

using namespace std;

const char* config_file = "Config.txt";

void SetParam(string line, Settings* settings)
{
    size_t pos;
    int r, g, b;

    pos = line.find("=");

    if (line.find("FieldSize") != string::npos)
        settings->fieldSize = stoi(line.substr(pos + 1));

    else if (line.find("WindowWidth") != string::npos)
        settings->width = stoi(line.substr(pos + 1));

    else if (line.find("WindowHeight") != string::npos)
        settings->height = stoi(line.substr(pos + 1));

    else if (line.find("BackgroundColor") != string::npos)
    {
        string color = line.substr(pos + 1);
        color.erase(0, color.find("(") + 1);
        color.erase(color.find(")"), 1);

        r = stoi(color.substr(0, color.find(",")));
        color.erase(0, color.find(",") + 1);

        g = stoi(color.substr(0, color.find(",")));
        color.erase(0, color.find(",") + 1);

        b = stoi(color);

        settings->BGColor = RGB(r, g, b);
    }

    else if (line.find("GridColor") != string::npos)
    {
        string color = line.substr(pos + 1);
        color.erase(0, color.find("(") + 1);
        color.erase(color.find(")"), 1);

        r = stoi(color.substr(0, color.find(",")));
        color.erase(0, color.find(",") + 1);

        g = stoi(color.substr(0, color.find(",")));
        color.erase(0, color.find(",") + 1);

        b = stoi(color);

        settings->gridColor = RGB(r, g, b);
    }
}

void ReadFile1(Settings* settings)
{
    HANDLE hFile;
    hFile = CreateFileA(config_file,
        GENERIC_READ,           // open for reading
        0,                      // sharing mode
        NULL,                   // no security
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,  // normal file
        0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        cout << "Config file not found" << endl;
        return;
    }

    HANDLE hMap;
    hMap = CreateFileMappingW(hFile,
        NULL,                   // security mode
        PAGE_READONLY,          //open mode
        0,
        0,
        NULL);                  // object name

    if (hMap == NULL)
    {
        cout << "CreateFileMapping error: " << GetLastError();
        CloseHandle(hFile);
        return;
    }

    char* ptrFileMap;
    ptrFileMap = (char*)MapViewOfFile(hMap,
        FILE_MAP_READ,
        0,
        0,
        0);                 //number of bytes

    if (ptrFileMap == NULL)
    {
        cout << "MapViewOfFile error: " << GetLastError();
        CloseHandle(hMap);
        CloseHandle(hFile);
        return;
    }
    else
    {
        string line = "";
        size_t pos;
        int r, g, b;

        int fileSize = GetFileSize(hFile, nullptr);

        for (int i = 0; i < fileSize; i++)
        {
            if (ptrFileMap[i] != '\n')
            {
                line += ptrFileMap[i];
            }
            else
            {
                SetParam(line, settings);
                line = "";
            }
        }
        SetParam(line, settings);

        UnmapViewOfFile(ptrFileMap);
        CloseHandle(hMap);
        CloseHandle(hFile);
    }
}

void ReadFile2(Settings* settings)
{
    FILE* file;
    if (fopen_s(&file, config_file, "r") == 0)
    {
        fseek(file, 0, SEEK_END);
        int len = ftell(file);
        char buffer[256];

        fseek(file, 0, SEEK_SET);
        fread(buffer, sizeof(char), len, file);

        string line = "";
        size_t pos;
        int r, g, b;

        for (int i = 0; i < len; i++)
        {
            if (buffer[i] != '\n')
            {
                line += buffer[i];
            }
            else
            {
                SetParam(line, settings);
                line = "";
            }
        }
        SetParam(line, settings);

        fclose(file);
    }
    else
        cout << "Config file not found" << endl;
}

void ReadFile3(Settings* settings) {
    ifstream file(config_file);

    if (file.is_open())
    {
        string line;
        size_t pos;
        int r, g, b;

        while (!file.eof())
        {
            getline(file, line);
            SetParam(line, settings);
        }
    }
    else
    {
        cout << "Config file not found" << endl;
    }
    file.close();
}

void ReadFile4(Settings* settings) {
    HANDLE hFile;
    hFile = CreateFileA(config_file,
        GENERIC_READ,
        0,
        NULL,                  // no security
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, // normal file
        0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        cout << "Config file not found" << endl;
        return;
    }
    else
    {
        DWORD bytesRead = 0;
        char buffer[256];

        ReadFile(hFile, buffer, 256, &bytesRead, NULL);

        string line = "";
        size_t pos;
        int r, g, b;

        for (int i = 0; i < (int)bytesRead; i++)
        {
            if (buffer[i] != '\n')
            {
                line += buffer[i];
            }
            else
            {
                SetParam(line, settings);
                line = "";
            }
        }
        SetParam(line, settings);
        CloseHandle(hFile);
    }
}

string CreateParamLine(Settings* settings)
{
    string str = "WindowWidth=" + to_string(settings->width) + "\nWindowHeight=" + to_string(settings->height) + "\nFieldSize=" + to_string((int)settings->fieldSize) + "\nBackgroundColor=RGB(" + to_string(GetRValue(settings->BGColor)) + ", " + to_string(GetGValue(settings->BGColor)) + ", " + to_string(GetBValue(settings->BGColor)) + ")\n" + "GridColor=RGB(" + to_string(GetRValue(settings->gridColor)) + ", " + to_string(GetGValue(settings->gridColor)) + ", " + to_string(GetBValue(settings->gridColor)) + ")";
    return str;
}

void WriteFile1(Settings* settings)
{
    HANDLE hFile;
    hFile = CreateFileA(config_file,
        GENERIC_READ | GENERIC_WRITE,
        0,                          // sharing mode
        NULL,                       // no security
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,      // normal file
        0);

    string str = CreateParamLine(settings);
    const char* buffer = str.c_str();

    if (hFile == INVALID_HANDLE_VALUE)
    {
        cout << "CreateFile error: " << GetLastError() << endl;
        return;
    }

    HANDLE hMap;
    hMap = CreateFileMappingW(hFile,
        NULL,                       // security mode
        PAGE_READWRITE,             //open mode
        0,
        strlen(buffer),
        NULL);                      //object name

    if (hMap == NULL)
    {
        cout << "CreateFileMapping error: " << GetLastError() << endl;
        CloseHandle(hFile);
        return;
    }

    LPVOID ptrFileMap;
    ptrFileMap = MapViewOfFile(hMap,
        FILE_MAP_WRITE,
        0,
        0,
        strlen(buffer));            //number of bytes

    if (ptrFileMap == NULL)
    {
        cout << "MapViewOfFile error: " << GetLastError() << endl;
        CloseHandle(hMap);
        CloseHandle(hFile);
        return;
    }
    else
    {
        CopyMemory(ptrFileMap, buffer, strlen(buffer));

        UnmapViewOfFile(ptrFileMap);
        CloseHandle(hMap);
        CloseHandle(hFile);
    }
}

void WriteFile2(Settings* settings)
{
    FILE* file;
    if (fopen_s(&file, config_file, "w") == 0)
    {
        string str = CreateParamLine(settings);
        const char* buffer = str.c_str();

        fwrite(buffer, sizeof(buffer[0]), strlen(buffer), file);

        fclose(file);
    }
    else
        cout << GetLastError() << endl;
}

void WriteFile3(Settings* settings) {
    ofstream file(config_file);

    if (file.is_open())
    {
        file << CreateParamLine(settings);

        file.close();
    }
    else
        cout << "Error: " << GetLastError() << endl;
}

void WriteFile4(Settings* settings) {
    HANDLE hFile;
    hFile = CreateFileA(config_file,
        GENERIC_WRITE,
        0,
        NULL,                       // no security
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        cout << "Config file not found" << endl;
        return;
    }
    else
    {
        DWORD bytesRead = 0;

        string str = CreateParamLine(settings);
        const char* buffer = str.c_str();

        WriteFile(hFile, buffer, strlen(buffer), NULL, NULL);

        CloseHandle(hFile);
    }
}

void ReadConfigFromFile(int fileMode, Settings* settings)
{
    switch (fileMode) {
    case 1: ReadFile1(settings); break;
    case 2: ReadFile2(settings); break;
    case 3: ReadFile3(settings); break;
    case 4: ReadFile4(settings); break;
    }
}

void WriteConfigToFile(int fileMode, Settings* settings)
{
    switch (fileMode) {
    case 1: WriteFile1(settings); break;
    case 2: WriteFile2(settings); break;
    case 3: WriteFile3(settings); break;
    case 4: WriteFile4(settings); break;
    }
}