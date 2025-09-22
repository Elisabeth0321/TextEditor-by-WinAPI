// TextEditor.cpp : Defines the entry point for the application.
// -*- coding: utf-8 -*-

#include "framework.h"
#include "TextEditor.h"
#include <commdlg.h>
#include <commctrl.h>
#include <locale.h>

// Подключаем необходимые библиотеки
#pragma comment(lib, "comctl32.lib")

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

// Переменные для текстового редактора
HWND hEditControl = NULL;
WCHAR currentFileName[MAX_PATH] = { 0 };
BOOL isFileModified = FALSE;
BOOL hasFileName = FALSE;

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

// Функции для текстового редактора
void                CreateEditControl(HWND hParent);
void                ResizeEditControl(HWND hParent);
BOOL                OpenTextFile(HWND hWnd);
BOOL                SaveTextFile(HWND hWnd);
BOOL                SaveTextFileAs(HWND hWnd);
void                CutText();
void                CopyText();
void                PasteText();
void                SetFileModified(BOOL modified);
BOOL                PromptSaveChanges(HWND hWnd);
void                UpdateWindowTitle(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Устанавливаем русскую локаль для правильного отображения кириллицы
    setlocale(LC_ALL, "Russian_Russia.1251");
    SetConsoleCP(65001); // UTF-8
    SetConsoleOutputCP(65001); // UTF-8
    
    // Устанавливаем системную локаль для правильного отображения Unicode
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), SORT_DEFAULT));
    
    // Устанавливаем процесс в режим DPI-aware для правильного отображения текста
    SetProcessDPIAware();
    
    // Инициализируем Common Controls для правильной работы диалогов
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    // Загружаем заголовок и имя класса из ресурсов
    CHAR titleAnsi[MAX_LOADSTRING];
    LoadStringA(hInstance, IDS_APP_TITLE, titleAnsi, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, titleAnsi, -1, szTitle, MAX_LOADSTRING);
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
        szTitle,                    // lpWindowName - используем загруженный из ресурсов заголовок
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
        
        // Создаем многострочный EDIT-контрол
        CreateEditControl(hWnd);
        UpdateWindowTitle(hWnd);
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
            // Проверяем, нужно ли сохранить изменения
            if (isFileModified && !PromptSaveChanges(hWnd))
            {
                break; // Пользователь отменил операцию
            }
            OpenTextFile(hWnd);
        }
        break;
        case IDM_FILE_SAVE:
        {
            if (hasFileName)
            {
                SaveTextFile(hWnd);
            }
            else
            {
                SaveTextFileAs(hWnd);
            }
        }
        break;
        case IDM_EDIT_CUT:
            CutText();
            break;
        case IDM_EDIT_COPY:
            CopyText();
            break;
        case IDM_EDIT_PASTE:
            PasteText();
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
        // Обработка уведомлений от EDIT-контрола
        if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == hEditControl)
        {
            SetFileModified(TRUE);
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
        ResizeEditControl(hWnd);
        break;
    case WM_CLOSE:
        // Проверяем, нужно ли сохранить изменения перед выходом
        if (isFileModified && !PromptSaveChanges(hWnd))
        {
            return 0; // Отменяем закрытие
        }
        DestroyWindow(hWnd);
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

// Создание многострочного EDIT-контрола
void CreateEditControl(HWND hParent)
{
    hEditControl = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hParent,
        NULL,
        hInst,
        NULL
    );

    if (hEditControl)
    {
        // Устанавливаем шрифт по умолчанию
        HFONT hFont = CreateFontW(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Consolas"
        );
        SendMessage(hEditControl, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        // Устанавливаем фокус на EDIT-контрол
        SetFocus(hEditControl);
    }
}

// Изменение размера EDIT-контрола
void ResizeEditControl(HWND hParent)
{
    if (hEditControl)
    {
        RECT rect;
        GetClientRect(hParent, &rect);
        SetWindowPos(hEditControl, NULL, 0, 0, 
                    rect.right, rect.bottom, 
                    SWP_NOZORDER);
    }
}

// Открытие текстового файла
BOOL OpenTextFile(HWND hWnd)
{
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = { 0 };
    WCHAR titleBuffer[MAX_LOADSTRING];
    
    // Загружаем заголовок из ресурсов
    CHAR titleAnsi[MAX_LOADSTRING];
    LoadStringA(hInst, IDS_OPEN_FILE_TITLE, titleAnsi, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, titleAnsi, -1, titleBuffer, MAX_LOADSTRING);
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Текстовые файлы\0*.txt\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = titleBuffer;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_EXPLORER;
    ofn.lpfnHook = FileDialogHook;

    if (GetOpenFileName(&ofn))
    {
        HANDLE hFile = CreateFileW(
            szFile,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE)
            {
                // Выделяем память для содержимого файла
                CHAR* buffer = (CHAR*)malloc(fileSize + 1);
                if (buffer)
                {
                    DWORD bytesRead;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL))
                    {
                        buffer[bytesRead] = '\0';
                        
                        // Конвертируем в Unicode
                        int wideSize = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
                        WCHAR* wideBuffer = (WCHAR*)malloc(wideSize * sizeof(WCHAR));
                        if (wideBuffer)
                        {
                            MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wideBuffer, wideSize);
                            
                            // Устанавливаем текст в EDIT-контрол
                            SetWindowTextW(hEditControl, wideBuffer);
                            
                            // Сохраняем имя файла
                            wcscpy_s(currentFileName, MAX_PATH, szFile);
                            hasFileName = TRUE;
                            SetFileModified(FALSE);
                            UpdateWindowTitle(hWnd);
                            
                            free(wideBuffer);
                        }
                    }
                    free(buffer);
                }
            }
            CloseHandle(hFile);
            return TRUE;
        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось открыть файл", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
    return FALSE;
}

// Сохранение текстового файла
BOOL SaveTextFile(HWND hWnd)
{
    if (!hasFileName)
    {
        return SaveTextFileAs(hWnd);
    }

    // Получаем текст из EDIT-контрола
    int textLength = GetWindowTextLengthW(hEditControl);
    WCHAR* textBuffer = (WCHAR*)malloc((textLength + 1) * sizeof(WCHAR));
    if (!textBuffer)
    {
        MessageBoxW(hWnd, L"Недостаточно памяти", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    GetWindowTextW(hEditControl, textBuffer, textLength + 1);

    // Конвертируем в UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, textBuffer, -1, NULL, 0, NULL, NULL);
    CHAR* utf8Buffer = (CHAR*)malloc(utf8Size);
    if (!utf8Buffer)
    {
        free(textBuffer);
        MessageBoxW(hWnd, L"Недостаточно памяти", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    WideCharToMultiByte(CP_UTF8, 0, textBuffer, -1, utf8Buffer, utf8Size, NULL, NULL);

    // Создаем/перезаписываем файл
    HANDLE hFile = CreateFileW(
        currentFileName,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    BOOL success = FALSE;
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if (WriteFile(hFile, utf8Buffer, (DWORD)strlen(utf8Buffer), &bytesWritten, NULL))
        {
            SetFileModified(FALSE);
            UpdateWindowTitle(hWnd);
            success = TRUE;
        }
        else
        {
            MessageBoxW(hWnd, L"Ошибка при записи файла", L"Ошибка", MB_OK | MB_ICONERROR);
        }
        CloseHandle(hFile);
    }
    else
    {
        MessageBoxW(hWnd, L"Не удалось создать файл", L"Ошибка", MB_OK | MB_ICONERROR);
    }

    free(textBuffer);
    free(utf8Buffer);
    return success;
}

// Сохранение файла с выбором имени
BOOL SaveTextFileAs(HWND hWnd)
{
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = { 0 };
    WCHAR titleBuffer[MAX_LOADSTRING];

    // Если есть текущее имя файла, используем его
    if (hasFileName)
    {
        wcscpy_s(szFile, MAX_PATH, currentFileName);
    }

    // Загружаем заголовок из ресурсов
    CHAR titleAnsi[MAX_LOADSTRING];
    LoadStringA(hInst, IDS_SAVE_FILE_TITLE, titleAnsi, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, titleAnsi, -1, titleBuffer, MAX_LOADSTRING);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Текстовые файлы\0*.txt\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = titleBuffer;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER;
    ofn.lpfnHook = FileDialogHook;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileName(&ofn))
    {
        wcscpy_s(currentFileName, MAX_PATH, szFile);
        hasFileName = TRUE;
        return SaveTextFile(hWnd);
    }
    return FALSE;
}

// Вырезание текста
void CutText()
{
    if (hEditControl)
    {
        SendMessage(hEditControl, WM_CUT, 0, 0);
    }
}

// Копирование текста
void CopyText()
{
    if (hEditControl)
    {
        SendMessage(hEditControl, WM_COPY, 0, 0);
    }
}

// Вставка текста
void PasteText()
{
    if (hEditControl)
    {
        SendMessage(hEditControl, WM_PASTE, 0, 0);
    }
}

// Установка флага изменения файла
void SetFileModified(BOOL modified)
{
    if (isFileModified != modified)
    {
        isFileModified = modified;
        UpdateWindowTitle(hMainWnd);
    }
}

// Запрос на сохранение изменений
BOOL PromptSaveChanges(HWND hWnd)
{
    if (!isFileModified)
    {
        return TRUE; // Нет изменений, продолжаем
    }

    WCHAR message[512];
    if (hasFileName)
    {
        swprintf_s(message, 512, L"Файл \"%s\" был изменен.\n\nСохранить изменения?", 
                  wcsrchr(currentFileName, L'\\') ? wcsrchr(currentFileName, L'\\') + 1 : currentFileName);
    }
    else
    {
        wcscpy_s(message, 512, L"Документ был изменен.\n\nСохранить изменения?");
    }

    int result = MessageBoxW(hWnd, message, L"Сохранение", 
                            MB_YESNOCANCEL | MB_ICONQUESTION);

    switch (result)
    {
    case IDYES:
        return SaveTextFile(hWnd);
    case IDNO:
        return TRUE;
    case IDCANCEL:
    default:
        return FALSE;
    }
}

// Обновление заголовка окна
void UpdateWindowTitle(HWND hWnd)
{
    WCHAR title[512];
    
    if (hasFileName)
    {
        const WCHAR* fileName = wcsrchr(currentFileName, L'\\');
        if (fileName)
            fileName = fileName + 1; // Пропускаем символ '\'
        else
            fileName = currentFileName;

        if (isFileModified)
        {
            swprintf_s(title, 512, L"%s* - %s", fileName, szTitle);
        }
        else
        {
            swprintf_s(title, 512, L"%s - %s", fileName, szTitle);
        }
    }
    else
    {
        // Если файл не открыт, показываем только название программы
        wcscpy_s(title, 512, szTitle);
    }

    SetWindowTextW(hWnd, title);
}

