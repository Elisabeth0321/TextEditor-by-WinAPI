#include "Application.h"
#include "Resource.h"
#include <commctrl.h>
#include <locale.h>

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
{
}

Application::~Application()
{
}

BOOL Application::initialize()
{
    // Инициализируем локализацию
    initializeLocalization();
    
    // Инициализируем Common Controls
    initializeCommonControls();

    // Создаем модули приложения
    m_windowManager = std::make_unique<WindowManager>(m_hInstance);
    m_fileManager = std::make_unique<FileManager>(m_hInstance);
    m_editControlManager = std::make_unique<EditControlManager>(m_hInstance);
    m_darkScreenManager = std::make_unique<DarkScreenManager>(m_hInstance);

    // Регистрируем класс окна
    if (!m_windowManager->registerWindowClass())
    {
        return FALSE;
    }

    // Настраиваем обработчики событий
    setupEventHandlers();

    return TRUE;
}

int Application::run(int nCmdShow)
{
    // Создаем главное окно
    if (!m_windowManager->createMainWindow(nCmdShow))
    {
        return FALSE;
    }

    // Создаем EDIT-контрол
    if (!m_editControlManager->createEditControl(getMainWindow()))
    {
        return FALSE;
    }

    // Устанавливаем таймер для отслеживания неактивности
    if (m_darkScreenManager)
    {
        m_darkScreenManager->setIdleTimer(getMainWindow(), 5000);
    }

    // Загружаем таблицу акселераторов
    HACCEL hAccelTable = LoadAccelerators(m_hInstance, MAKEINTRESOURCE(IDC_TEXTEDITOR));
    MSG msg;

    // Главный цикл сообщений
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

void Application::handleMenuCommand(int commandId)
{
    switch (commandId)
    {
    case IDM_FILE_OPEN:
        handleOpenFile();
        break;
    case IDM_FILE_SAVE:
        handleSaveFile();
        break;
    case IDM_EDIT_CUT:
        handleCutText();
        break;
    case IDM_EDIT_COPY:
        handleCopyText();
        break;
    case IDM_EDIT_PASTE:
        handlePasteText();
        break;
    case IDM_ABOUT:
        handleAbout();
        break;
    case IDM_EXIT:
        handleExit();
        break;
    default:
        break;
    }
}

void Application::handleWindowResize(HWND hWnd)
{
    if (m_editControlManager)
    {
        m_editControlManager->resizeEditControl(hWnd);
    }
}

void Application::handleTextChange()
{
    if (m_fileManager)
    {
        m_fileManager->setFileModified(TRUE);
        updateWindowTitle();
    }
}

void Application::handleUserActivity(HWND hWnd)
{
    if (m_darkScreenManager)
    {
        m_darkScreenManager->handleUserActivity(hWnd);
    }
}

void Application::handleTimer(HWND hWnd, UINT_PTR timerId)
{
    if (m_darkScreenManager)
    {
        m_darkScreenManager->handleTimer(hWnd, timerId);
    }
}

BOOL Application::handleWindowClose(HWND hWnd)
{
    if (m_fileManager && m_fileManager->isFileModified())
    {
        if (!m_fileManager->promptSaveChanges(hWnd))
        {
            return FALSE; // Отменяем закрытие
        }
    }
    return TRUE; // Разрешаем закрытие
}

void Application::updateWindowTitle()
{
    if (m_windowManager && m_fileManager)
    {
        std::wstring fileName = m_fileManager->getShortFileName();
        BOOL isModified = m_fileManager->isFileModified();
        m_windowManager->updateWindowTitle(fileName, isModified);
    }
}

HWND Application::getMainWindow() const
{
    if (m_windowManager)
    {
        return m_windowManager->getMainWindow();
    }
    return NULL;
}

void Application::setupEventHandlers()
{
    if (!m_windowManager)
    {
        return;
    }

    // Устанавливаем обработчик команд меню
    m_windowManager->setCommandHandler([this](int commandId) {
        handleMenuCommand(commandId);
    });

    // Устанавливаем обработчик изменения размера окна
    m_windowManager->setResizeHandler([this](HWND hWnd) {
        handleWindowResize(hWnd);
    });

    // Устанавливаем обработчик изменения текста
    m_windowManager->setTextChangeHandler([this]() {
        handleTextChange();
    });

    // Устанавливаем обработчик сообщений от EDIT-контрола
    m_windowManager->setEditControlHandler([this](WPARAM wParam, LPARAM lParam) -> BOOL {
        if (m_editControlManager && m_editControlManager->isEditControlMessage(wParam, lParam))
        {
            handleTextChange();
            return TRUE; // Сообщение обработано
        }
        return FALSE; // Сообщение не обработано
    });

    // Устанавливаем обработчик таймера
    m_windowManager->setTimerHandler([this](HWND hWnd, UINT_PTR timerId) {
        handleTimer(hWnd, timerId);
    });

    // Устанавливаем обработчик активности пользователя
    m_windowManager->setUserActivityHandler([this](HWND hWnd) {
        handleUserActivity(hWnd);
    });

    // Устанавливаем обработчик закрытия окна
    m_windowManager->setCloseHandler([this](HWND hWnd) -> BOOL {
        return handleWindowClose(hWnd);
    });

    // Устанавливаем обработчик темного экрана
    m_windowManager->setDarkScreenHandler([this](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (m_darkScreenManager)
        {
            return m_darkScreenManager->handleDarkScreenMessage(hWnd, message, wParam, lParam);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    });
}

void Application::initializeLocalization()
{
    // Устанавливаем русскую локаль для правильного отображения кириллицы
    setlocale(LC_ALL, "Russian_Russia.1251");
    SetConsoleCP(65001); // UTF-8
    SetConsoleOutputCP(65001); // UTF-8
    
    // Устанавливаем системную локаль для правильного отображения Unicode
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), SORT_DEFAULT));
    
    // Устанавливаем процесс в режим DPI-aware для правильного отображения текста
    SetProcessDPIAware();
}

void Application::initializeCommonControls()
{
    // Инициализируем Common Controls для правильной работы диалогов
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
}

void Application::handleOpenFile()
{
    if (!m_fileManager || !m_editControlManager)
    {
        return;
    }

    // Проверяем, нужно ли сохранить изменения
    if (m_fileManager->isFileModified() && !m_fileManager->promptSaveChanges(getMainWindow()))
    {
        return; // Пользователь отменил операцию
    }

    if (m_fileManager->openTextFile(getMainWindow()))
    {
        // Загружаем содержимое файла в редактор
        std::wstring content = m_fileManager->getFileContent();
        m_editControlManager->setText(content);
        updateWindowTitle();
    }
}

void Application::handleSaveFile()
{
    if (!m_fileManager || !m_editControlManager)
    {
        return;
    }

    // Сохраняем содержимое редактора в файл
    std::wstring content = m_editControlManager->getText();
    m_fileManager->setFileContent(content);

    if (m_fileManager->hasFileName())
    {
        m_fileManager->saveTextFile(getMainWindow());
    }
    else
    {
        m_fileManager->saveTextFileAs(getMainWindow());
    }
    
    updateWindowTitle();
}

void Application::handleCutText()
{
    if (m_editControlManager)
    {
        m_editControlManager->cutText();
    }
}

void Application::handleCopyText()
{
    if (m_editControlManager)
    {
        m_editControlManager->copyText();
    }
}

void Application::handlePasteText()
{
    if (m_editControlManager)
    {
        m_editControlManager->pasteText();
    }
}

void Application::handleAbout()
{
    if (m_windowManager)
    {
        m_windowManager->showAboutDialog(getMainWindow());
    }
}

void Application::handleExit()
{
    if (getMainWindow())
    {
        DestroyWindow(getMainWindow());
    }
}
