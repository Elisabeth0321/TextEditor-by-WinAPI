#include "WindowManager.h"
#include "Resource.h"
#include <functional>

WindowManager::WindowManager(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_hMainWnd(NULL)
{
    loadStringsFromResources();
}

WindowManager::~WindowManager()
{
}

BOOL WindowManager::registerWindowClass()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = mainWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_TEXTEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TEXTEDITOR);
    wcex.lpszClassName = m_windowClass.c_str();
    wcex.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_TEXTEDITOR));

    return RegisterClassExW(&wcex) != 0;
}

BOOL WindowManager::createMainWindow(int nCmdShow)
{
    // Создаем главное окно с помощью CreateWindowEx
    m_hMainWnd = CreateWindowExW(
        0,                          // dwExStyle
        m_windowClass.c_str(),      // lpClassName
        m_appTitle.c_str(),         // lpWindowName
        WS_OVERLAPPEDWINDOW,        // dwStyle
        CW_USEDEFAULT,              // x
        0,                          // y
        CW_USEDEFAULT,              // nWidth
        0,                          // nHeight
        nullptr,                    // hWndParent
        nullptr,                    // hMenu
        m_hInstance,                // hInstance
        this                        // lpParam - передаем указатель на объект
    );

    if (!m_hMainWnd)
    {
        return FALSE;
    }
    
    ShowWindow(m_hMainWnd, nCmdShow);
    UpdateWindow(m_hMainWnd);

    return TRUE;
}

HWND WindowManager::getMainWindow() const
{
    return m_hMainWnd;
}

void WindowManager::updateWindowTitle(const std::wstring& fileName, BOOL isModified)
{
    if (!m_hMainWnd)
    {
        return;
    }

    std::wstring title;
    
    if (!fileName.empty())
    {
        if (isModified)
        {
            title = fileName + L"* - " + m_appTitle;
        }
        else
        {
            title = fileName + L" - " + m_appTitle;
        }
    }
    else
    {
        // Если файл не открыт, показываем только название программы
        title = m_appTitle;
    }

    SetWindowTextW(m_hMainWnd, title.c_str());
}

void WindowManager::showAboutDialog(HWND hWnd)
{
    DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, aboutDialogProc);
}

LRESULT WindowManager::handleMainWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Сохраняем указатель на объект в данных окна
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
        
        // Вызываем обработчик изменения размера для инициализации
        if (m_resizeHandler)
        {
            m_resizeHandler(hWnd);
        }
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        
        // Обрабатываем команды меню
        if (m_commandHandler)
        {
            m_commandHandler(wmId);
        }
        
        // Обработка уведомлений от EDIT-контрола
        if (m_editControlHandler)
        {
            if (m_editControlHandler(wParam, lParam))
            {
                return 0; // Сообщение обработано
            }
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
        if (m_timerHandler)
        {
            m_timerHandler(hWnd, wParam);
        }
        break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
        if (m_userActivityHandler)
        {
            m_userActivityHandler(hWnd);
        }
        break;
    case WM_SIZE:
        if (m_resizeHandler)
        {
            m_resizeHandler(hWnd);
        }
        break;
    case WM_CLOSE:
        if (m_closeHandler)
        {
            if (!m_closeHandler(hWnd))
            {
                return 0; // Отменяем закрытие
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void WindowManager::setCommandHandler(std::function<void(int)> commandHandler)
{
    m_commandHandler = commandHandler;
}

void WindowManager::setResizeHandler(std::function<void(HWND)> resizeHandler)
{
    m_resizeHandler = resizeHandler;
}

void WindowManager::setTextChangeHandler(std::function<void()> textChangeHandler)
{
    m_textChangeHandler = textChangeHandler;
}

void WindowManager::setEditControlHandler(std::function<BOOL(WPARAM, LPARAM)> editControlHandler)
{
    m_editControlHandler = editControlHandler;
}

void WindowManager::setTimerHandler(std::function<void(HWND, UINT_PTR)> timerHandler)
{
    m_timerHandler = timerHandler;
}

void WindowManager::setUserActivityHandler(std::function<void(HWND)> userActivityHandler)
{
    m_userActivityHandler = userActivityHandler;
}

void WindowManager::setCloseHandler(std::function<BOOL(HWND)> closeHandler)
{
    m_closeHandler = closeHandler;
}

void WindowManager::setDarkScreenHandler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> darkScreenHandler)
{
    m_darkScreenHandler = darkScreenHandler;
}

LRESULT WindowManager::handleDarkScreenMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (m_darkScreenHandler)
    {
        return m_darkScreenHandler(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

RECT WindowManager::getClientRect() const
{
    RECT rect = { 0 };
    if (m_hMainWnd)
    {
        ::GetClientRect(m_hMainWnd, &rect);
    }
    return rect;
}

void WindowManager::loadStringsFromResources()
{
    CHAR titleAnsi[MAX_LOADSTRING];
    WCHAR titleWide[MAX_LOADSTRING];
    
    // Загружаем заголовок приложения
    LoadStringA(m_hInstance, IDS_APP_TITLE, titleAnsi, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, titleAnsi, -1, titleWide, MAX_LOADSTRING);
    m_appTitle = std::wstring(titleWide);
    
    // Загружаем имя класса окна
    LoadStringW(m_hInstance, IDC_TEXTEDITOR, titleWide, MAX_LOADSTRING);
    m_windowClass = std::wstring(titleWide);
}

void WindowManager::centerDialog(HWND hDlg)
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

LRESULT CALLBACK WindowManager::mainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WindowManager* instance = getInstanceFromWindow(hWnd);
    if (instance)
    {
        // Проверяем, является ли это сообщение от темного экрана
        if (instance->m_darkScreenHandler)
        {
            LRESULT result = instance->m_darkScreenHandler(hWnd, message, wParam, lParam);
            if (result != 0) // Если сообщение было обработано темным экраном
            {
                return result;
            }
        }
        
        return instance->handleMainWindowMessage(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK WindowManager::aboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Центрируем диалог
        RECT dialogRect;
        GetWindowRect(hDlg, &dialogRect);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int dialogWidth = dialogRect.right - dialogRect.left;
        int dialogHeight = dialogRect.bottom - dialogRect.top;
        int x = (screenWidth - dialogWidth) / 2;
        int y = (screenHeight - dialogHeight) / 2;

        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        return (INT_PTR)TRUE;
    }

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

WindowManager* WindowManager::getInstanceFromWindow(HWND hWnd)
{
    return (WindowManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}
