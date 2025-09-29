// TextEditor.cpp : Defines the entry point for the application.
// -*- coding: utf-8 -*-

#include "framework.h"
#include "TextEditor.h"
#include "RegistryManager.h"
#include "DarkScreenManager.h"
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
HWND hMainWnd = NULL;
RECT clientRect;
DarkScreenManager* g_pDarkScreenManager = nullptr;

// Переменные для текстового редактора
HWND hEditControl = NULL;
WCHAR currentFileName[MAX_PATH] = { 0 };
BOOL isFileModified = FALSE;
BOOL hasFileName = FALSE;

// Переменные для настроек
RegistryManager* g_pRegistryManager = nullptr;
LOGFONTW g_currentFont = { 0 };
COLORREF g_textColor = RGB(0, 0, 0);
COLORREF g_backgroundColor = RGB(255, 255, 255);
HFONT g_hCurrentFont = NULL;
HBRUSH g_hBackgroundBrush = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                CenterDialog(HWND hDlg);
UINT_PTR CALLBACK   FileDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Функции для текстового редактора
void                CreateEditControl(HWND hParent);
void                ResizeEditControl(HWND hParent);
BOOL                CreateNewFile(HWND hWnd);
BOOL                OpenTextFile(HWND hWnd);
BOOL                LoadFileContent(const WCHAR* filePath);
BOOL                SaveTextFile(HWND hWnd);
BOOL                SaveTextFileAs(HWND hWnd);
void                CutText();
void                CopyText();
void                PasteText();
void                SetFileModified(BOOL modified);
BOOL                PromptSaveChanges(HWND hWnd);
void                UpdateWindowTitle(HWND hWnd);
void                LoadFileDialogFilters(WCHAR* filterBuffer, int bufferSize);

// Функции для работы с настройками
void                LoadSettingsFromRegistry();
void                SaveSettingsToRegistry();
void                ApplyFontSettings();
void                ApplyColorSettings();
BOOL                ShowFontDialog();
BOOL                ShowColorDialog(BOOL isTextColor);

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

    // Инициализируем менеджер реестра
    g_pRegistryManager = new RegistryManager();
    
    // Инициализируем менеджер темного экрана
    g_pDarkScreenManager = new DarkScreenManager(hInstance);

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

    // Очищаем ресурсы
    if (g_hCurrentFont)
    {
        DeleteObject(g_hCurrentFont);
    }
    if (g_hBackgroundBrush)
    {
        DeleteObject(g_hBackgroundBrush);
    }
    if (g_pRegistryManager)
    {
        delete g_pRegistryManager;
    }
    if (g_pDarkScreenManager)
    {
        delete g_pDarkScreenManager;
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
        
        // Загружаем настройки из реестра (после создания EditControl)
        LoadSettingsFromRegistry();
        UpdateWindowTitle(hWnd);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_FILE_NEW:
        {
            // Проверяем, нужно ли сохранить изменения
            if (isFileModified && !PromptSaveChanges(hWnd))
            {
                break; // Пользователь отменил операцию
            }
            CreateNewFile(hWnd);
        }
        break;
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
        case IDM_SETTINGS_FONT:
            if (ShowFontDialog())
            {
                ApplyFontSettings();
                SaveSettingsToRegistry();
            }
            break;
        case IDM_SETTINGS_BG_COLOR:
            if (ShowColorDialog(FALSE))
            {
                ApplyColorSettings();
                SaveSettingsToRegistry();
            }
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            // Проверяем, нужно ли сохранить изменения перед выходом
            if (isFileModified && !PromptSaveChanges(hWnd))
            {
                break; // Пользователь отменил операцию
            }
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
        if (g_pDarkScreenManager)
        {
            g_pDarkScreenManager->handleTimer(hWnd, wParam);
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
        // Сбрасываем таймер при активности пользователя
        if (g_pDarkScreenManager)
        {
            g_pDarkScreenManager->handleUserActivity(hWnd);
        }
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &clientRect);
        ResizeEditControl(hWnd);
        break;
    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, g_textColor);
        SetBkColor(hdc, g_backgroundColor);
        
        // Удаляем старую кисть
        if (g_hBackgroundBrush)
        {
            DeleteObject(g_hBackgroundBrush);
        }
        
        // Создаем новую кисть
        g_hBackgroundBrush = CreateSolidBrush(g_backgroundColor);
        return (LRESULT)g_hBackgroundBrush;
    }
    case WM_CLOSE:
        // Проверяем, нужно ли сохранить изменения перед выходом
        if (isFileModified && !PromptSaveChanges(hWnd))
        {
            return 0; // Отменяем закрытие
        }
        DestroyWindow(hWnd);
        break;
    case WM_QUERYENDSESSION:
        // Проверяем, нужно ли сохранить изменения при завершении системы
        if (isFileModified && !PromptSaveChanges(hWnd))
        {
            return FALSE; // Отменяем завершение
        }
        return TRUE; // Разрешаем завершение
    case WM_DESTROY:
        if (g_pDarkScreenManager)
        {
            g_pDarkScreenManager->killIdleTimer(hWnd);
        }
        // Сохраняем настройки в реестр
        SaveSettingsToRegistry();
        PostQuitMessage(0);
        break;
    default:
        // Передаем сообщения в DarkScreenManager для обработки
        if (g_pDarkScreenManager)
        {
            LRESULT result = g_pDarkScreenManager->handleDarkScreenMessage(hWnd, message, wParam, lParam);
            if (result != 0)
            {
                return result;
            }
        }
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
        // Применяем настройки шрифта и цветов
        ApplyFontSettings();
        ApplyColorSettings();
        
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

// Создание нового файла
BOOL CreateNewFile(HWND hWnd)
{
    // Очищаем содержимое EDIT-контрола
    if (hEditControl)
    {
        SetWindowTextW(hEditControl, L"");
    }
    
    // Сбрасываем информацию о текущем файле
    currentFileName[0] = L'\0';
    hasFileName = FALSE;
    SetFileModified(FALSE);
    
    // Сохраняем состояние "новый файл" в реестре
    if (g_pRegistryManager)
    {
        g_pRegistryManager->SaveLastFileState(FALSE); // FALSE означает "новый файл"
    }
    
    // Обновляем заголовок окна
    UpdateWindowTitle(hWnd);
    
    // Устанавливаем фокус на EDIT-контрол
    if (hEditControl)
    {
        SetFocus(hEditControl);
    }
    
    return TRUE;
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
    
    // Загружаем фильтры из ресурсов
    WCHAR filterBuffer[MAX_LOADSTRING * 2];
    LoadFileDialogFilters(filterBuffer, MAX_LOADSTRING * 2);
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filterBuffer;
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = titleBuffer;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

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
                        
                        // Определяем кодировку файла
                        UINT codePage = CP_UTF8; // По умолчанию UTF-8
                        
                        // Проверяем BOM для UTF-8
                        if (bytesRead >= 3 && 
                            (unsigned char)buffer[0] == 0xEF && 
                            (unsigned char)buffer[1] == 0xBB && 
                            (unsigned char)buffer[2] == 0xBF)
                        {
                            codePage = CP_UTF8;
                            // Пропускаем BOM
                            memmove(buffer, buffer + 3, bytesRead - 3);
                            buffer[bytesRead - 3] = '\0';
                        }
                        // Проверяем BOM для UTF-16 LE
                        else if (bytesRead >= 2 && 
                                 (unsigned char)buffer[0] == 0xFF && 
                                 (unsigned char)buffer[1] == 0xFE)
                        {
                            // Файл уже в UTF-16 LE, читаем напрямую
                            WCHAR* wideBuffer = (WCHAR*)buffer;
                            SetWindowTextW(hEditControl, wideBuffer);
                            
                            // Сохраняем имя файла
                            wcscpy_s(currentFileName, MAX_PATH, szFile);
                            hasFileName = TRUE;
                            SetFileModified(FALSE);
                            
                            // Сохраняем состояние "файл открыт" в реестре
                            if (g_pRegistryManager)
                            {
                                g_pRegistryManager->SaveLastFileState(TRUE);
                            }
                            
                            UpdateWindowTitle(hWnd);
                            
                            free(buffer);
                            CloseHandle(hFile);
                            return TRUE;
                        }
                        // Проверяем BOM для UTF-16 BE
                        else if (bytesRead >= 2 && 
                                 (unsigned char)buffer[0] == 0xFE && 
                                 (unsigned char)buffer[1] == 0xFF)
                        {
                            // Конвертируем UTF-16 BE в UTF-16 LE
                            for (DWORD i = 0; i < bytesRead - 1; i += 2)
                            {
                                char temp = buffer[i];
                                buffer[i] = buffer[i + 1];
                                buffer[i + 1] = temp;
                            }
                            WCHAR* wideBuffer = (WCHAR*)buffer;
                            SetWindowTextW(hEditControl, wideBuffer);
                            
                            // Сохраняем имя файла
                            wcscpy_s(currentFileName, MAX_PATH, szFile);
                            hasFileName = TRUE;
                            SetFileModified(FALSE);
                            
                            // Сохраняем состояние "файл открыт" в реестре
                            if (g_pRegistryManager)
                            {
                                g_pRegistryManager->SaveLastFileState(TRUE);
                            }
                            
                            UpdateWindowTitle(hWnd);
                            
                            free(buffer);
                            CloseHandle(hFile);
                            return TRUE;
                        }
                        // Пробуем определить кодировку по содержимому
                        else
                        {
                            // Сначала пробуем UTF-8
                            int wideSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buffer, -1, NULL, 0);
                            if (wideSize > 0)
                            {
                                codePage = CP_UTF8;
                            }
                            else
                            {
                                // Если не UTF-8, пробуем системную кодировку (обычно CP1251 для русского)
                                wideSize = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
                                if (wideSize > 0)
                                {
                                    codePage = CP_ACP;
                                }
                                else
                                {
                                    // Последняя попытка - CP1252 (Latin-1)
                                    codePage = 1252;
                                }
                            }
                        }
                        
                        // Конвертируем в Unicode
                        int wideSize = MultiByteToWideChar(codePage, 0, buffer, -1, NULL, 0);
                        WCHAR* wideBuffer = (WCHAR*)malloc(wideSize * sizeof(WCHAR));
                        if (wideBuffer)
                        {
                            MultiByteToWideChar(codePage, 0, buffer, -1, wideBuffer, wideSize);
                            
                            // Устанавливаем текст в EDIT-контрол
                            SetWindowTextW(hEditControl, wideBuffer);
                            
                            // Сохраняем имя файла
                            wcscpy_s(currentFileName, MAX_PATH, szFile);
                            hasFileName = TRUE;
                            SetFileModified(FALSE);
                            
                            // Сохраняем состояние "файл открыт" в реестре
                            if (g_pRegistryManager)
                            {
                                g_pRegistryManager->SaveLastFileState(TRUE);
                            }
                            
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

// Загрузка содержимого файла без диалога выбора
BOOL LoadFileContent(const WCHAR* filePath)
{
    if (!hEditControl || !filePath || wcslen(filePath) == 0)
        return FALSE;
    
    // Проверяем существование файла
    DWORD fileAttributes = GetFileAttributesW(filePath);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES || (fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return FALSE;

    HANDLE hFile = CreateFileW(
        filePath,
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
                    
                    // Определяем кодировку файла
                    UINT codePage = CP_UTF8; // По умолчанию UTF-8
                    
                    // Проверяем BOM для UTF-8
                    if (bytesRead >= 3 && 
                        (unsigned char)buffer[0] == 0xEF && 
                        (unsigned char)buffer[1] == 0xBB && 
                        (unsigned char)buffer[2] == 0xBF)
                    {
                        codePage = CP_UTF8;
                        // Пропускаем BOM
                        memmove(buffer, buffer + 3, bytesRead - 3);
                        buffer[bytesRead - 3] = '\0';
                    }
                    // Проверяем BOM для UTF-16 LE
                    else if (bytesRead >= 2 && 
                             (unsigned char)buffer[0] == 0xFF && 
                             (unsigned char)buffer[1] == 0xFE)
                    {
                        // Файл уже в UTF-16 LE, читаем напрямую
                        WCHAR* wideBuffer = (WCHAR*)buffer;
                        SetWindowTextW(hEditControl, wideBuffer);
                        
                        free(buffer);
                        CloseHandle(hFile);
                        return TRUE;
                    }
                    // Проверяем BOM для UTF-16 BE
                    else if (bytesRead >= 2 && 
                             (unsigned char)buffer[0] == 0xFE && 
                             (unsigned char)buffer[1] == 0xFF)
                    {
                        // Конвертируем UTF-16 BE в UTF-16 LE
                        for (DWORD i = 0; i < bytesRead - 1; i += 2)
                        {
                            char temp = buffer[i];
                            buffer[i] = buffer[i + 1];
                            buffer[i + 1] = temp;
                        }
                        WCHAR* wideBuffer = (WCHAR*)buffer;
                        SetWindowTextW(hEditControl, wideBuffer);
                        
                        free(buffer);
                        CloseHandle(hFile);
                        return TRUE;
                    }
                    // Пробуем определить кодировку по содержимому
                    else
                    {
                        // Сначала пробуем UTF-8
                        int wideSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buffer, -1, NULL, 0);
                        if (wideSize > 0)
                        {
                            codePage = CP_UTF8;
                        }
                        else
                        {
                            // Если не UTF-8, пробуем системную кодировку (обычно CP1251 для русского)
                            wideSize = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
                            if (wideSize > 0)
                            {
                                codePage = CP_ACP;
                            }
                            else
                            {
                                // Последняя попытка - CP1252 (Latin-1)
                                codePage = 1252;
                            }
                        }
                    }
                    
                    // Конвертируем в Unicode
                    int wideSize = MultiByteToWideChar(codePage, 0, buffer, -1, NULL, 0);
                    WCHAR* wideBuffer = (WCHAR*)malloc(wideSize * sizeof(WCHAR));
                    if (wideBuffer)
                    {
                        MultiByteToWideChar(codePage, 0, buffer, -1, wideBuffer, wideSize);
                        
                        // Устанавливаем текст в EDIT-контрол
                        SetWindowTextW(hEditControl, wideBuffer);
                        
                        free(wideBuffer);
                    }
                }
                free(buffer);
            }
        }
        CloseHandle(hFile);
        return TRUE;
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

    // Определяем кодировку для сохранения на основе расширения файла
    UINT saveCodePage = CP_UTF8; // По умолчанию UTF-8
    
    // Получаем расширение файла
    WCHAR* fileExt = wcsrchr(currentFileName, L'.');
    if (fileExt)
    {
        // Для текстовых и кодовых файлов используем UTF-8
        if (_wcsicmp(fileExt, L".txt") == 0 ||
            _wcsicmp(fileExt, L".c") == 0 ||
            _wcsicmp(fileExt, L".cpp") == 0 ||
            _wcsicmp(fileExt, L".h") == 0 ||
            _wcsicmp(fileExt, L".hpp") == 0 ||
            _wcsicmp(fileExt, L".cs") == 0 ||
            _wcsicmp(fileExt, L".java") == 0 ||
            _wcsicmp(fileExt, L".js") == 0 ||
            _wcsicmp(fileExt, L".html") == 0 ||
            _wcsicmp(fileExt, L".css") == 0 ||
            _wcsicmp(fileExt, L".xml") == 0 ||
            _wcsicmp(fileExt, L".json") == 0 ||
            _wcsicmp(fileExt, L".py") == 0 ||
            _wcsicmp(fileExt, L".php") == 0 ||
            _wcsicmp(fileExt, L".rb") == 0 ||
            _wcsicmp(fileExt, L".pl") == 0 ||
            _wcsicmp(fileExt, L".sh") == 0)
        {
            saveCodePage = CP_UTF8;
        }
        // Для других файлов используем системную кодировку
        else
        {
            saveCodePage = CP_ACP;
        }
    }
    
    // Конвертируем в выбранную кодировку
    int bufferSize = WideCharToMultiByte(saveCodePage, 0, textBuffer, -1, NULL, 0, NULL, NULL);
    CHAR* saveBuffer = (CHAR*)malloc(bufferSize);
    if (!saveBuffer)
    {
        free(textBuffer);
        MessageBoxW(hWnd, L"Недостаточно памяти", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    WideCharToMultiByte(saveCodePage, 0, textBuffer, -1, saveBuffer, bufferSize, NULL, NULL);

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
        if (WriteFile(hFile, saveBuffer, (DWORD)strlen(saveBuffer), &bytesWritten, NULL))
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
    free(saveBuffer);
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

    // Загружаем фильтры из ресурсов
    WCHAR filterBuffer[MAX_LOADSTRING * 2];
    LoadFileDialogFilters(filterBuffer, MAX_LOADSTRING * 2);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filterBuffer;
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = titleBuffer;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = L""; // Без расширения по умолчанию, пользователь сам выберет

    if (GetSaveFileName(&ofn))
    {
        wcscpy_s(currentFileName, MAX_PATH, szFile);
        hasFileName = TRUE;
        
        // Сохраняем состояние "файл открыт" в реестре
        if (g_pRegistryManager)
        {
            g_pRegistryManager->SaveLastFileState(TRUE);
        }
        
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
    WCHAR title[128];
    
    // Формируем сообщение в зависимости от того, есть ли имя файла
    if (hasFileName)
    {
        const WCHAR* fileName = wcsrchr(currentFileName, L'\\');
        if (fileName)
            fileName = fileName + 1; // Пропускаем символ '\'
        else
            fileName = currentFileName;
            
        swprintf_s(message, 512, L"Файл \"%s\" был изменен.\n\nСохранить изменения?", fileName);
        wcscpy_s(title, 128, L"Сохранение файла");
    }
    else
    {
        wcscpy_s(message, 512, L"Документ был изменен.\n\nСохранить изменения?");
        wcscpy_s(title, 128, L"Сохранение документа");
    }

    int result = MessageBoxW(hWnd, message, title, 
                            MB_YESNOCANCEL | MB_ICONQUESTION);

    switch (result)
    {
    case IDYES:
        // Пытаемся сохранить файл
        if (hasFileName)
        {
            return SaveTextFile(hWnd);
        }
        else
        {
            // Если файл не был сохранен ранее, показываем диалог "Сохранить как"
            return SaveTextFileAs(hWnd);
        }
    case IDNO:
        return TRUE; // Продолжаем без сохранения
    case IDCANCEL:
    default:
        return FALSE; // Отменяем операцию
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

// Загрузка фильтров файлов из ресурсов
void LoadFileDialogFilters(WCHAR* filterBuffer, int bufferSize)
{
    CHAR textFilterAnsi[MAX_LOADSTRING];
    CHAR codeFilterAnsi[MAX_LOADSTRING];
    CHAR allFilterAnsi[MAX_LOADSTRING];
    LoadStringA(hInst, IDS_TEXT_FILES_FILTER, textFilterAnsi, MAX_LOADSTRING);
    LoadStringA(hInst, IDS_CODE_FILES_FILTER, codeFilterAnsi, MAX_LOADSTRING);
    LoadStringA(hInst, IDS_ALL_FILES_FILTER, allFilterAnsi, MAX_LOADSTRING);
    
    WCHAR textFilterWide[MAX_LOADSTRING];
    WCHAR codeFilterWide[MAX_LOADSTRING];
    WCHAR allFilterWide[MAX_LOADSTRING];
    MultiByteToWideChar(CP_ACP, 0, textFilterAnsi, -1, textFilterWide, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, codeFilterAnsi, -1, codeFilterWide, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, allFilterAnsi, -1, allFilterWide, MAX_LOADSTRING);
    
    // Простой фильтр для отображения всех файлов
    // Формат: "Описание\0маска\0\0" - заканчивается двойным нулем
    swprintf_s(filterBuffer, bufferSize, 
        L"\0*.*\0\0");
}

// Загрузка настроек из реестра
void LoadSettingsFromRegistry()
{
    if (!g_pRegistryManager)
        return;

    // Загружаем настройки шрифта
    g_pRegistryManager->LoadFontSettings(g_currentFont);
    
    // Загружаем цвета
    g_pRegistryManager->LoadTextColor(g_textColor);
    g_pRegistryManager->LoadBackgroundColor(g_backgroundColor);
    
    // Загружаем состояние файла (открыт/новый)
    BOOL savedFileState = FALSE;
    g_pRegistryManager->LoadLastFileState(savedFileState);
    
    // Загружаем последний открытый файл только если состояние указывает на открытый файл
    if (savedFileState)
    {
        std::wstring lastFile;
        if (g_pRegistryManager->LoadLastFile(lastFile) && !lastFile.empty())
        {
            wcscpy_s(currentFileName, MAX_PATH, lastFile.c_str());
            hasFileName = TRUE;
            
            // Загружаем содержимое файла в EditControl
            if (!LoadFileContent(currentFileName))
            {
                // Если файл не удалось загрузить, сбрасываем состояние
                hasFileName = FALSE;
                currentFileName[0] = L'\0';
                SetFileModified(FALSE);
            }
            else
            {
                // Файл успешно загружен, сбрасываем флаг изменения
                SetFileModified(FALSE);
            }
        }
        else
        {
            // Если состояние говорит о открытом файле, но файл не найден, сбрасываем состояние
            hasFileName = FALSE;
            currentFileName[0] = L'\0';
        }
    }
    else
    {
        // Состояние "новый файл" - очищаем поле
        hasFileName = FALSE;
        currentFileName[0] = L'\0';
    }
    
    // Применяем загруженные настройки шрифта и цветов
    ApplyFontSettings();
    ApplyColorSettings();
}

// Сохранение настроек в реестр
void SaveSettingsToRegistry()
{
    if (!g_pRegistryManager)
        return;

    // Сохраняем настройки шрифта
    g_pRegistryManager->SaveFontSettings(g_currentFont);
    
    // Сохраняем цвета
    g_pRegistryManager->SaveTextColor(g_textColor);
    g_pRegistryManager->SaveBackgroundColor(g_backgroundColor);
    
    // Сохраняем состояние файла (открыт/новый)
    g_pRegistryManager->SaveLastFileState(hasFileName);
    
    // Сохраняем последний открытый файл только если файл открыт
    if (hasFileName)
    {
        g_pRegistryManager->SaveLastFile(currentFileName);
    }
}

// Применение настроек шрифта
void ApplyFontSettings()
{
    if (!hEditControl)
        return;

    // Удаляем старый шрифт
    if (g_hCurrentFont)
    {
        DeleteObject(g_hCurrentFont);
    }

    // Создаем новый шрифт
    g_hCurrentFont = CreateFontIndirectW(&g_currentFont);
    if (g_hCurrentFont)
    {
        SendMessage(hEditControl, WM_SETFONT, (WPARAM)g_hCurrentFont, TRUE);
    }
}

// Применение настроек цветов
void ApplyColorSettings()
{
    if (!hEditControl)
        return;

    // Устанавливаем цвет фона через WM_CTLCOLOREDIT
    // Для этого нужно перерисовать окно
    InvalidateRect(hEditControl, NULL, TRUE);
}

// Диалог выбора шрифта
BOOL ShowFontDialog()
{
    if (!hEditControl)
        return FALSE;

    CHOOSEFONTW cf = { 0 };
    cf.lStructSize = sizeof(CHOOSEFONTW);
    cf.hwndOwner = hMainWnd;
    cf.lpLogFont = &g_currentFont;
    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS;
    cf.rgbColors = g_textColor;

    if (ChooseFontW(&cf))
    {
        g_textColor = cf.rgbColors;
        return TRUE;
    }

    return FALSE;
}

// Диалог выбора цвета
BOOL ShowColorDialog(BOOL isTextColor)
{
    CHOOSECOLORW cc = { 0 };
    static COLORREF customColors[16] = { 0 };

    cc.lStructSize = sizeof(CHOOSECOLORW);
    cc.hwndOwner = hMainWnd;
    cc.lpCustColors = customColors;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;

    if (isTextColor)
    {
        cc.rgbResult = g_textColor;
    }
    else
    {
        cc.rgbResult = g_backgroundColor;
    }

    if (ChooseColorW(&cc))
    {
        if (isTextColor)
        {
            g_textColor = cc.rgbResult;
        }
        else
        {
            g_backgroundColor = cc.rgbResult;
        }
        return TRUE;
    }

    return FALSE;
}

