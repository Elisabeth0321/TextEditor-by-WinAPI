// GraphicsEditor.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TextEditor.h"
#include <commdlg.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define TIMER_IDLE 1
#define IDLE_TIMEOUT 5000

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Переменные для темного экрана
HWND hDarkScreen = NULL;
HWND hMainWnd = NULL;
POINT spritePos = { 100, 100 };
POINT spriteVelocity = { 2, 2 };
RECT clientRect;
BOOL isDarkScreenActive = FALSE;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    DarkScreenProc(HWND, UINT, WPARAM, LPARAM);
void                ShowDarkScreen(HWND hParent);
void                HideDarkScreen();
void                UpdateSprite();
void                CenterDialog(HWND hDlg);
UINT_PTR CALLBACK   FileDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TEXTEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEXTEDITOR));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEXTEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TEXTEDITOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEXTEDITOR));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    // Создаем главное окно с помощью CreateWindowEx
    hMainWnd = CreateWindowExW(
        0,                          // dwExStyle
        szWindowClass,              // lpClassName
        szTitle,                    // lpWindowName
        WS_OVERLAPPEDWINDOW,        // dwStyle
        CW_USEDEFAULT,              // x
        0,                          // y
        CW_USEDEFAULT,              // nWidth
        0,                          // nHeight
        nullptr,                    // hWndParent
        nullptr,                    // hMenu
        hInstance,                  // hInstance
        nullptr                     // lpParam
    );

    if (!hMainWnd)
    {
        return FALSE;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Устанавливаем таймер для отслеживания неактивности
        SetTimer(hWnd, TIMER_IDLE, IDLE_TIMEOUT, NULL);
        GetClientRect(hWnd, &clientRect);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_FILE_OPEN:
        {
            OPENFILENAME ofn;
            WCHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Все файлы\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrTitle = L"Открыть файл";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            ofn.lpfnHook = FileDialogHook;
            ofn.lCustData = 0;

            if (GetOpenFileName(&ofn))
            {
                MessageBox(hWnd, L"Файл выбран для открытия", L"Информация", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
        case IDM_FILE_SAVE:
        {
            OPENFILENAME ofn;
            WCHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Все файлы\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrTitle = L"Сохранить файл";
            ofn.Flags = OFN_OVERWRITEPROMPT;
            ofn.lpfnHook = FileDialogHook;
            ofn.lCustData = 0;

            if (GetSaveFileName(&ofn))
            {
                MessageBox(hWnd, L"Файл выбран для сохранения", L"Информация", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
        case IDM_EDIT_CUT:
            MessageBox(hWnd, L"Вырезать", L"Правка", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EDIT_COPY:
            MessageBox(hWnd, L"Копировать", L"Правка", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EDIT_PASTE:
            MessageBox(hWnd, L"Вставить", L"Правка", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
        if (wParam == TIMER_IDLE && !isDarkScreenActive)
        {
            ShowDarkScreen(hWnd);
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
        // Сбрасываем таймер при активности пользователя
        if (isDarkScreenActive)
        {
            HideDarkScreen();
        }
        KillTimer(hWnd, TIMER_IDLE);
        SetTimer(hWnd, TIMER_IDLE, IDLE_TIMEOUT, NULL);
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &clientRect);
        break;
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_IDLE);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        CenterDialog(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// Функция для показа темного экрана
void ShowDarkScreen(HWND hParent)
{
    if (isDarkScreenActive) return;

    // Получаем размеры родительского окна
    RECT parentRect;
    GetWindowRect(hParent, &parentRect);

    // Создаем темное окно
    hDarkScreen = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"STATIC",
        L"",
        WS_POPUP | WS_VISIBLE,
        parentRect.left,
        parentRect.top,
        parentRect.right - parentRect.left,
        parentRect.bottom - parentRect.top,
        hParent,
        NULL,
        hInst,
        NULL
    );

    if (hDarkScreen)
    {
        SetWindowLongPtr(hDarkScreen, GWLP_WNDPROC, (LONG_PTR)DarkScreenProc);
        SetTimer(hDarkScreen, 2, 50, NULL);
        isDarkScreenActive = TRUE;
        ShowCursor(FALSE);
    }
}

// Функция для скрытия темного экрана
void HideDarkScreen()
{
    if (!isDarkScreenActive) return;

    if (hDarkScreen)
    {
        KillTimer(hDarkScreen, 2);
        DestroyWindow(hDarkScreen);
        hDarkScreen = NULL;
    }

    isDarkScreenActive = FALSE;
    ShowCursor(TRUE);
}

// Процедура окна для темного экрана
LRESULT CALLBACK DarkScreenProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Заливаем экран черным цветом
        RECT rect;
        GetClientRect(hWnd, &rect);
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rect, blackBrush);
        DeleteObject(blackBrush);

        // Рисуем движущийся спрайт (белый круг)
        HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, whiteBrush);

        Ellipse(hdc, spritePos.x - 10, spritePos.y - 10,
            spritePos.x + 10, spritePos.y + 10);

        SelectObject(hdc, oldBrush);
        DeleteObject(whiteBrush);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
        if (wParam == 2)
        {
            UpdateSprite();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
        HideDarkScreen();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Функция для обновления позиции спрайта
void UpdateSprite()
{
    RECT rect;
    GetClientRect(hDarkScreen, &rect);

    // Обновляем позицию спрайта
    spritePos.x += spriteVelocity.x;
    spritePos.y += spriteVelocity.y;

    // Проверяем столкновения с границами
    if (spritePos.x <= 10 || spritePos.x >= rect.right - 10)
    {
        spriteVelocity.x = -spriteVelocity.x;
    }
    if (spritePos.y <= 10 || spritePos.y >= rect.bottom - 10)
    {
        spriteVelocity.y = -spriteVelocity.y;
    }

    // Ограничиваем позицию в пределах окна
    if (spritePos.x < 10) spritePos.x = 10;
    if (spritePos.x > rect.right - 10) spritePos.x = rect.right - 10;
    if (spritePos.y < 10) spritePos.y = 10;
    if (spritePos.y > rect.bottom - 10) spritePos.y = rect.bottom - 10;
}

// Функция для центрирования диалогового окна на экране
void CenterDialog(HWND hDlg)
{
    RECT dialogRect;
    GetWindowRect(hDlg, &dialogRect);

    // Получаем размеры экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Вычисляем позицию для центрирования
    int dialogWidth = dialogRect.right - dialogRect.left;
    int dialogHeight = dialogRect.bottom - dialogRect.top;
    int x = (screenWidth - dialogWidth) / 2;
    int y = (screenHeight - dialogHeight) / 2;

    // Устанавливаем новую позицию
    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

// Hook-функция для центрирования диалогов открытия/сохранения файлов
UINT_PTR CALLBACK FileDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        CenterDialog(hDlg);
        break;
    }
    return 0;
}

