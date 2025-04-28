#include "Settings.h"
#include "tchar.h"

using namespace std;

#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001

#define KEY_C     0x43
#define KEY_Q     0x51

#define KEY_1     0x31
#define KEY_2     0x32
#define KEY_3     0x33
#define KEY_4     0x34

const TCHAR szWinClass[] = _T("Win32SimpleApp");
const TCHAR szWinName[] = _T("Tic_Tac_Toe");
HWND hwnd;               // window handle
HBRUSH hBrush;           // current brush handle

UINT myMessage = RegisterWindowMessageW(L"MyMessage");
UINT closeMessage = RegisterWindowMessageW(L"Win");

HANDLE drawThread;
HANDLE startDraw;
BOOL gradientIsDrawing = FALSE;

HANDLE blockWindow;  // mutex to block a window

/* service info to write into shared memory */
struct Service
{
    int count;
    int move;
};

Service* ptrService;
Settings settings;

int figure = 1; // first window will use Circles

/* parameters for gradient brash */
double gradientFirstParam = 1;
double gradientSecondParam = 250;
double gradientRatio = 500;

void ChangeBGColor(short a, int R, int G, int B)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    if (a > 0)
    {
        if (R < 255)
            settings.gridColor = RGB(R += 5, G, B);
        else if (G < 255)
            settings.gridColor = RGB(R, G += 5, B);
        else if (B < 255)
            settings.gridColor = RGB(R, G, B += 5);
    }
    else if (a < 0)
    {
        if (R > 0)
            settings.gridColor = RGB(R -= 5, G, B);
        else if (G > 0)
            settings.gridColor = RGB(R, G -= 5, B);
        else if (B > 0)
            settings.gridColor = RGB(R, G, B -= 5);
    }
    InvalidateRect(hwnd, &rc, TRUE);
}

void DrawGrid()
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);
    hdc = BeginPaint(hwnd, &ps);
    hdc = GetDC(hwnd);

    HPEN pen = CreatePen(PS_SOLID, 2, settings.gridColor);
    SelectObject(hdc, pen);

    for (double i = 1; i < settings.fieldSize; i++)
    {
        double y = rc.bottom / settings.fieldSize;
        double x = rc.right / settings.fieldSize;
        MoveToEx(hdc, rc.left, y * i, (LPPOINT)NULL);
        LineTo(hdc, rc.right, y * i);
        MoveToEx(hdc, x * i, rc.top, (LPPOINT)NULL);
        LineTo(hdc, x * i, rc.bottom);
    }

    DeleteObject(pen);
    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
}

void DrawObjects()
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);
    hdc = BeginPaint(hwnd, &ps);
    hdc = GetDC(hwnd);

    HPEN pen = CreatePen(PS_SOLID, 2, settings.objectColor);
    SelectObject(hdc, pen);

    double lenX = (rc.right - rc.left) / settings.fieldSize;
    double lenY = (rc.bottom - rc.top) / settings.fieldSize;

    for (int i = 0; i < settings.fieldSize; i++)
    {
        for (int j = 0; j < settings.fieldSize; j++)
        {
            if (lenX <= lenY && settings.arr[i * (int)settings.fieldSize + j] == 1)
            {
                int oX = lenX * (i);
                int oY = ((lenY - lenX) / 2) + lenY * (j);
                Ellipse(hdc, rc.left + oX + 2, rc.top + oY + 2, rc.left + oX + lenX - 2, rc.top + oY + lenX - 2);
            }
            else if (lenY <= lenX && settings.arr[i * (int)settings.fieldSize + j] == 1)
            {
                int oY = lenY * (j);
                int oX = ((lenX - lenY) / 2) + lenX * (i);
                Ellipse(hdc, rc.left + oX + 2, rc.top + oY + 2, rc.left + oX + lenY - 2, rc.top + oY + lenY - 2);
            }
            else if (lenX <= lenY && settings.arr[i * (int)settings.fieldSize + j] == 2)
            {
                int oX = lenX * (i);
                int oY = ((lenY - lenX) / 2) + lenY * (j);
                MoveToEx(hdc, rc.left + oX, rc.top + oY, (LPPOINT)NULL);
                LineTo(hdc, rc.left + oX + lenX, rc.top + oY + lenX);
                MoveToEx(hdc, rc.left + oX + lenX, rc.top + oY, (LPPOINT)NULL);
                LineTo(hdc, rc.left + oX, rc.top + oY + lenX);
            }
            else if (lenY <= lenX && settings.arr[i * (int)settings.fieldSize + j] == 2)
            {
                int oY = lenY * (j);
                int oX = ((lenX - lenY) / 2) + lenX * (i);
                MoveToEx(hdc, rc.left + oX, rc.top + oY, (LPPOINT)NULL);
                LineTo(hdc, rc.left + oX + lenY, rc.top + oY + lenY);
                MoveToEx(hdc, rc.left + oX + lenY, rc.top + oY, (LPPOINT)NULL);
                LineTo(hdc, rc.left + oX, rc.top + oY + lenY);
            }
        }
    }

    DeleteObject(pen);
    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
}

bool DrawEllipse(LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);
    hdc = BeginPaint(hwnd, &ps);
    hdc = GetDC(hwnd);

    bool created = FALSE;

    HPEN pen = CreatePen(PS_SOLID, 2, settings.objectColor);
    SelectObject(hdc, pen);

    double lenX = (rc.right - rc.left) / settings.fieldSize;
    double lenY = (rc.bottom - rc.top) / settings.fieldSize;

    if (settings.arr[(int)(LOWORD(lParam) / lenX) * (int)settings.fieldSize + (int)(HIWORD(lParam) / lenY)] == 0)
    {
        settings.arr[(int)(LOWORD(lParam) / lenX) * (int)settings.fieldSize + (int)(HIWORD(lParam) / lenY)] = 1;

        if (lenX <= lenY)
        {
            int oX = lenX * (int)(LOWORD(lParam) / lenX);
            int oY = ((lenY - lenX) / 2) + lenY * (int)(HIWORD(lParam) / lenY);
            Ellipse(hdc, rc.left + oX + 2, rc.top + oY + 2, rc.left + oX + lenX - 2, rc.top + oY + lenX - 2);
        }
        else if (lenY <= lenX)
        {
            int oY = lenY * (int)(HIWORD(lParam) / lenY);
            int oX = ((lenX - lenY) / 2) + lenX * (int)(LOWORD(lParam) / lenX);
            Ellipse(hdc, rc.left + oX + 2, rc.top + oY + 2, rc.left + oX + lenY - 2, rc.top + oY + lenY - 2);
        }
        created = TRUE;
    }
    else
        created = FALSE;

    DeleteObject(pen);
    EndPaint(hwnd, &ps);

    return created;
}

bool DrawCross(LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);
    hdc = BeginPaint(hwnd, &ps);
    hdc = GetDC(hwnd);

    HPEN pen = CreatePen(PS_SOLID, 2, settings.objectColor);
    SelectObject(hdc, pen);

    bool created = FALSE;

    double lenX = (rc.right - rc.left) / settings.fieldSize;
    double lenY = (rc.bottom - rc.top) / settings.fieldSize;

    if (settings.arr[(int)(LOWORD(lParam) / lenX) * (int)settings.fieldSize + (int)(HIWORD(lParam) / lenY)] == 0)
    {
        settings.arr[(int)(LOWORD(lParam) / lenX) * (int)settings.fieldSize + (int)(HIWORD(lParam) / lenY)] = 2;

        if (lenX <= lenY)
        {
            int oX = lenX * (int)(LOWORD(lParam) / lenX);
            int oY = ((lenY - lenX) / 2) + lenY * (int)(HIWORD(lParam) / lenY);
            MoveToEx(hdc, rc.left + oX, rc.top + oY, (LPPOINT)NULL);
            LineTo(hdc, rc.left + oX + lenX, rc.top + oY + lenX);
            MoveToEx(hdc, rc.left + oX + lenX, rc.top + oY, (LPPOINT)NULL);
            LineTo(hdc, rc.left + oX, rc.top + oY + lenX);
        }
        else if (lenY <= lenX)
        {
            int oY = lenY * (int)(HIWORD(lParam) / lenY);
            int oX = ((lenX - lenY) / 2) + lenX * (int)(LOWORD(lParam) / lenX);
            MoveToEx(hdc, rc.left + oX, rc.top + oY, (LPPOINT)NULL);
            LineTo(hdc, rc.left + oX + lenY, rc.top + oY + lenY);
            MoveToEx(hdc, rc.left + oX + lenY, rc.top + oY, (LPPOINT)NULL);
            LineTo(hdc, rc.left + oX, rc.top + oY + lenY);
        }
        created = TRUE;
    }
    else
        created = FALSE;

    DeleteObject(pen);
    EndPaint(hwnd, &ps);

    return created;
}

void CheckWinner1()
{
    int line = 0;
    int column = 0;
    int count = 0;
    int count2 = 0;

    bool win = FALSE;

    for (int i = 0; i < settings.fieldSize; i++)
    {
        for (int j = 0; j < settings.fieldSize; j++)
        {
            if (settings.arr[i * (int)settings.fieldSize + j] == 1)
                column++;
            if (settings.arr[j * (int)settings.fieldSize + i] == 1)
                line++;
            if (column == settings.fieldSize || line == settings.fieldSize)
            {
                win = TRUE;
                break;
            }
        }
        if (settings.arr[i * (int)settings.fieldSize + i] == 1)
            count++;
        if (settings.arr[i * (int)settings.fieldSize + ((int)settings.fieldSize - 1 - i)] == 1)
            count2++;
        if (count == settings.fieldSize || count2 == settings.fieldSize)
        {
            win = TRUE;
            break;
        }
        column = 0;
        line = 0;
    }

    if (win)
    {
        WaitForSingleObject(blockWindow, INFINITE);
        MessageBox(hwnd, _T("Circles won!"), _T("Gameover"), MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        PostMessage(HWND_BROADCAST, closeMessage, 0, 0);
    }
}

void CheckWinner2()
{
    int line = 0;
    int column = 0;
    int count = 0;
    int count2 = 0;

    bool win = FALSE;

    for (int i = 0; i < settings.fieldSize; i++)
    {
        for (int j = 0; j < settings.fieldSize; j++)
        {
            if (settings.arr[i * (int)settings.fieldSize + j] == 2)
                column++;
            if (settings.arr[j * (int)settings.fieldSize + i] == 2)
                line++;
            if (column == settings.fieldSize || line == settings.fieldSize)
            {
                win = TRUE;
                break;
            }
        }
        if (settings.arr[i * (int)settings.fieldSize + i] == 2)
            count++;
        if (settings.arr[i * (int)settings.fieldSize + ((int)settings.fieldSize - 1 - i)] == 2)
            count2++;
        if (count == settings.fieldSize || count2 == settings.fieldSize)
        {
            win = TRUE;
            break;
        }
        column = 0;
        line = 0;
    }

    if (win)
    {
        WaitForSingleObject(blockWindow, INFINITE);
        MessageBox(hwnd, _T("Сrosses won!"), _T("Gameover"), MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        PostMessage(HWND_BROADCAST, closeMessage, 0, 0);
    }
}

void CheckGameover()
{
    CheckWinner1();
    CheckWinner2();

    int count = 0;

    for (int i = 0; i < settings.fieldSize; i++)
    {
        for (int j = 0; j < settings.fieldSize; j++)
        {
            if (settings.arr[i * (int)settings.fieldSize + j] != 0)
                count++;
        }
    }

    if (count == settings.fieldSize * settings.fieldSize)
    {
        WaitForSingleObject(blockWindow, INFINITE);
        MessageBox(hwnd, _T("А draw!!!"), _T("Gameover"), MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        PostMessage(HWND_BROADCAST, closeMessage, 0, 0);
    }
}

HBRUSH CreateGradientBrush(COLORREF top, COLORREF bottom)
{
    HDC hdc;
    hdc = GetDC(hwnd);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    HBRUSH Brush = NULL;

    HDC hdcmem = CreateCompatibleDC(hdc);
    HBITMAP hbitmap = CreateCompatibleBitmap(hdc, width, height);

    SelectObject(hdcmem, hbitmap);

    int r1 = GetRValue(top), r2 = GetRValue(bottom), g1 = GetGValue(top), g2 = GetGValue(bottom), b1 = GetBValue(top), b2 = GetBValue(bottom);
    for (int i = 0; i < height; i++)
    {
        RECT temp;
        int r, g, b;
        r = int(r1 + double(i * (r2 - r1) / height));
        g = int(g1 + double(i * (g2 - g1) / height));
        b = int(b1 + double(i * (b2 - b1) / height));
        Brush = CreateSolidBrush(RGB(r, g, b));
        temp.left = 0;
        temp.top = i;
        temp.right = width;
        temp.bottom = i + 1;
        FillRect(hdcmem, &temp, Brush);
        DeleteObject(Brush);
    }
    HBRUSH pattern = CreatePatternBrush(hbitmap);
    
    DeleteDC(hdcmem);
    DeleteObject(Brush);
    DeleteObject(hbitmap);
    ReleaseDC(hwnd, hdc);

    return pattern;
}

uint32_t rgb(double ratio)
{
    int normalized = int(ratio * 256 * 6);
    int region = normalized / 256;
    int x = normalized % 256;

    uint8_t r = 0, g = 0, b = 0;
    switch (region)
    {
    case 0: r = 255; g = 0;   b = 0;   g += x; break;
    case 1: r = 255; g = 255; b = 0;   r -= x; break;
    case 2: r = 0;   g = 255; b = 0;   b += x; break;
    case 3: r = 0;   g = 255; b = 255; g -= x; break;
    case 4: r = 0;   g = 0;   b = 255; r += x; break;
    case 5: r = 255; g = 0;   b = 255; b -= x; break;
    }
    return r + (g << 8) + (b << 16);
}

DWORD WINAPI DrawBackground(void* lParam) {
    while (true)
    {
        WaitForSingleObject(startDraw, INFINITE);

        while (gradientIsDrawing)
        {
            DeleteObject((HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)CreateGradientBrush(rgb(gradientFirstParam / gradientRatio), rgb(gradientSecondParam / gradientRatio))));
            InvalidateRect(hwnd, NULL, TRUE);

            Sleep(30);

            if (++gradientFirstParam == gradientRatio) gradientFirstParam = 1;
            if (++gradientSecondParam == gradientRatio) gradientSecondParam = 1;
        }
        ResetEvent(startDraw);
    };
    return 0;
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    RECT wrc;

    DWORD result;

    if (message == myMessage)
    {
        InvalidateRect(hwnd, NULL, TRUE);
    }
    if (message == closeMessage)
    {
        PostQuitMessage(0);
    }

    switch (message)
    {
    case WM_SIZE:
        GetWindowRect(hwnd, &wrc);
        settings.width = wrc.right - wrc.left;
        settings.height = wrc.bottom - wrc.top;

        GetClientRect(hwnd, &rc);
        DeleteObject((HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)CreateGradientBrush(rgb(gradientFirstParam / gradientRatio), rgb(gradientSecondParam / gradientRatio))));
        InvalidateRect(hwnd, &rc, TRUE);
        break;
    case WM_PAINT:
        DrawGrid();
        DrawObjects();
        return 0;
    case WM_LBUTTONUP:
        result = WaitForSingleObject(blockWindow, 0);
        if (result != WAIT_OBJECT_0)
            return 0;

        if (ptrService->move == figure)
        {
            if (figure == 1)
            {
                if (DrawEllipse(lParam))
                {
                    ptrService->move = 2;
                    PostMessage(HWND_BROADCAST, myMessage, 0, 0);
                }
            }
            else
            {
                if (DrawCross(lParam))
                {
                    ptrService->move = 1;
                    PostMessage(HWND_BROADCAST, myMessage, 0, 0);
                }
            }
            CheckGameover();
        }
        else
            MessageBox(hwnd, _T("Another player's move"), _T("Notice"), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL);
        ReleaseMutex(blockWindow);
        return 0;
    case WM_RBUTTONUP:
        result = WaitForSingleObject(blockWindow, 0);
        if (result != WAIT_OBJECT_0)
            return 0;

        if (ptrService->move == figure)
        {
            if (figure == 2)
            {
                if (DrawCross(lParam))
                {
                    ptrService->move = 1;
                    PostMessage(HWND_BROADCAST, myMessage, 0, 0);
                }
            }
            else
            {
                if (DrawEllipse(lParam))
                {
                    ptrService->move = 2;
                    PostMessage(HWND_BROADCAST, myMessage, 0, 0);
                }
            }
            CheckGameover();
        }
        else
            MessageBox(hwnd, _T("Another player's move"), _T("Notice"), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL);
        ReleaseMutex(blockWindow);
        return 0;
    case WM_MOUSEWHEEL:
        ChangeBGColor(GET_WHEEL_DELTA_WPARAM(wParam), GetRValue(settings.gridColor), GetGValue(settings.gridColor), GetBValue(settings.gridColor));
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            settings.BGColor = RGB(
                rand() % 256,
                rand() % 256,
                rand() % 256
            );
            GetClientRect(hwnd, &rc);
            DeleteObject((HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)CreateSolidBrush(settings.BGColor)));
            InvalidateRect(hwnd, &rc, TRUE);
        }
        else if (wParam == VK_SPACE)
        {
            if (gradientIsDrawing = gradientIsDrawing ^ 1)
                SetEvent(startDraw);
        }
        else if (wParam == KEY_Q && (GetAsyncKeyState(VK_CONTROL) & KEY_SHIFTED))
            PostQuitMessage(0);
        else if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        switch (wParam)
        {
        case KEY_1: SetThreadPriority(drawThread, THREAD_MODE_BACKGROUND_BEGIN); cout << drawThread; break;
        case KEY_2: SetThreadPriority(drawThread, THREAD_PRIORITY_LOWEST); cout << GetThreadPriority(drawThread); break;
        case KEY_3: SetThreadPriority(drawThread, THREAD_PRIORITY_NORMAL); cout << GetThreadPriority(drawThread); break;
        case KEY_4: SetThreadPriority(drawThread, THREAD_PRIORITY_HIGHEST); cout << GetThreadPriority(drawThread); break;
        }
        return 0;
    case WM_DESTROY:
        PostMessage(HWND_BROADCAST, closeMessage, 0, 0);
        PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
        return 0;
    }

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char* argv[])
{
    BOOL bMessageOk;
    MSG message;                    // message to the application are saved=
    WNDCLASS wincl = { 0 };         // Data structure for the windowclass=

    int fileMode = 0;  // default parameter to config file mode (0 to disable configuration saving)

    /* Getting arguments from command line (format: --fileMode=1 --fieldSize=2) */
    size_t pos;
    string str;

    for (int i = 0; i < argc; i++)
    {
        str += argv[i];
    }

    pos = str.find("--fileMode=");
    if (pos != string::npos)
    {
        fileMode = stoi(str.substr(pos + 11));
    }

    ReadConfigFromFile(fileMode, &settings);

    pos = str.find("--fieldSize=");
    if (pos != string::npos)
    {
        settings.fieldSize = stoi(str.substr(pos + 12));
    }

    if (settings.fieldSize <= 0)
    {
        cout << "Wrong field size";
        exit(100);
    }

    /* Shared Memory */
    HANDLE hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(int) * settings.fieldSize * settings.fieldSize + sizeof(Service),
        L"SharedMemory");

    if (hMapFile == NULL)
    {
        cout << "CreateFileMapping error: " << GetLastError() << endl;
        return 100;
    }

    PVOID ptrMap = MapViewOfFile(hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(int) * settings.fieldSize * settings.fieldSize + sizeof(Service));

    /* write service info to shared memory */
    ptrService = (Service*)ptrMap + (int)(sizeof(int) * settings.fieldSize * settings.fieldSize);

    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        settings.arr = (int*)MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            sizeof(int) * settings.fieldSize * settings.fieldSize);

        for (int i = 0; i < settings.fieldSize; i++)
        {
            for (int j = 0; j < settings.fieldSize; j++)
            {
                settings.arr[i * (int)settings.fieldSize + j] = 0;
            }
        }

        ptrService->count = 1;
        ptrService->move = 1;
    }
    else
    {
        settings.arr = (int*)MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            sizeof(int) * settings.fieldSize * settings.fieldSize);

        ptrService->count += 1;
    }

    if (ptrService->count > 2)
    {
        MessageBox(NULL, _T("Нельзя создать более 2 окон"), _T("Tic_Tac_Toe"),
            MB_OK | MB_SETFOREGROUND);

        UnmapViewOfFile(settings.arr);
        CloseHandle(hMapFile);
        return 0;
    }
    figure = ptrService->count;

    srand(time(0));

    /* Harcode show command num when use non-winapi entrypoint */
    int nCmdShow = SW_SHOW;

    HINSTANCE hThisInstance = GetModuleHandle(NULL);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szWinClass;
    wincl.lpfnWndProc = WindowProcedure;      // This function is called by Windows

    /* Use custom brush to paint the background of the window */
    hBrush = CreateSolidBrush(settings.BGColor);
    wincl.hbrBackground = hBrush;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClass(&wincl))
        return 0;

    /* The window class */
    hwnd = CreateWindow(
        szWinClass,             // Classname
        szWinName,              // Title Text
        WS_OVERLAPPEDWINDOW,    // default window
        CW_USEDEFAULT,          // Windows decides the position
        CW_USEDEFAULT,          // where the window ends up on the screen
        settings.width,         // The programs width
        settings.height,        // and height in pixels
        HWND_DESKTOP,           // The window is a child-window to desktop
        NULL,                   // No menu
        hThisInstance,          // Program Instance handler
        NULL                    // No Window Creation data
    );

    ShowWindow(hwnd, nCmdShow);

    /* Thread of drawing background */
    drawThread = CreateThread(NULL, 0, DrawBackground, (void*)hwnd, 0, NULL);
    if (drawThread == NULL)
    {
        cout << "CreateThread error: " << GetLastError() << endl;
        return 0;
    }
    startDraw = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (startDraw == NULL)
    {
        cout << "CreateEvent error: " << GetLastError() << endl;
        return 0;
    }
    blockWindow = CreateMutex(NULL, FALSE, TEXT("NameOfMutexObject"));
    if (blockWindow == NULL)
    {
        cout << "CreateMutex error: " << GetLastError() << endl;
        return 0;
    }

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((bMessageOk = GetMessage(&message, NULL, 0, 0)) != 0)
    {
        if (bMessageOk == -1)
        {
            puts("Suddenly, GetMessage failed!");
            break;
        }
        /* Translate virtual-key message into character message */
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    /* Save Settings to file*/
    WriteConfigToFile(fileMode, &settings);

    /* Memory clean up */
    UnmapViewOfFile(settings.arr);
    CloseHandle(hMapFile);

    UnmapViewOfFile(ptrMap);
    CloseHandle(drawThread);
    CloseHandle(startDraw);
    CloseHandle(blockWindow);

    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);

    return 0;
}