#include "DarkScreenManager.h"

DarkScreenManager::DarkScreenManager(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_hDarkScreen(NULL)
    , m_isDarkScreenActive(FALSE)
    , m_keyW(FALSE)
    , m_keyA(FALSE)
    , m_keyS(FALSE)
    , m_keyD(FALSE)
    , m_keyUp(FALSE)
    , m_keyDown(FALSE)
    , m_keyLeft(FALSE)
    , m_keyRight(FALSE)
{
    m_spritePos.x = 100;
    m_spritePos.y = 100;
    m_spriteVelocity.x = 0;  // Начинаем с нулевой скорости
    m_spriteVelocity.y = 0;
}

DarkScreenManager::~DarkScreenManager()
{
    hideDarkScreen();
}

void DarkScreenManager::showDarkScreen(HWND hParent)
{
    if (m_isDarkScreenActive) 
    {
        return;
    }

    // Получаем размеры родительского окна
    RECT parentRect;
    GetWindowRect(hParent, &parentRect);

    // Создаем темное окно
    m_hDarkScreen = CreateWindowExW(
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
        m_hInstance,
        NULL
    );

    if (m_hDarkScreen)
    {
        // Сохраняем указатель на объект в данных окна
        SetWindowLongPtr(m_hDarkScreen, GWLP_USERDATA, (LONG_PTR)this);
        SetWindowLongPtr(m_hDarkScreen, GWLP_WNDPROC, (LONG_PTR)darkScreenProc);
        SetTimer(m_hDarkScreen, TIMER_ANIMATION, ANIMATION_INTERVAL, NULL);
        m_isDarkScreenActive = TRUE;
        ShowCursor(FALSE);
    }
}

void DarkScreenManager::hideDarkScreen()
{
    if (!m_isDarkScreenActive) 
    {
        return;
    }

    if (m_hDarkScreen)
    {
        KillTimer(m_hDarkScreen, TIMER_ANIMATION);
        DestroyWindow(m_hDarkScreen);
        m_hDarkScreen = NULL;
    }

    m_isDarkScreenActive = FALSE;
    ShowCursor(TRUE);
}

BOOL DarkScreenManager::isDarkScreenActive() const
{
    return m_isDarkScreenActive;
}

LRESULT DarkScreenManager::handleDarkScreenMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Проверяем, является ли это сообщение от темного экрана
    if (hWnd == m_hDarkScreen)
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

            Ellipse(hdc, m_spritePos.x - 10, m_spritePos.y - 10,
                m_spritePos.x + 10, m_spritePos.y + 10);

            SelectObject(hdc, oldBrush);
            DeleteObject(whiteBrush);

            EndPaint(hWnd, &ps);
        }
        break;
        case WM_TIMER:
            if (wParam == TIMER_ANIMATION)
            {
                updateSprite();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        case WM_KEYDOWN:
        {
            // Проверяем, является ли клавиша клавишей управления спрайтом
            if (isSpriteControlKey(wParam))
            {
                // Обрабатываем нажатия клавиш для управления спрайтом
                switch (wParam)
                {
                case 'W':
                case 'w':
                    m_keyW = TRUE;
                    break;
                case 'A':
                case 'a':
                    m_keyA = TRUE;
                    break;
                case 'S':
                case 's':
                    m_keyS = TRUE;
                    break;
                case 'D':
                case 'd':
                    m_keyD = TRUE;
                    break;
                case VK_UP:
                    m_keyUp = TRUE;
                    break;
                case VK_DOWN:
                    m_keyDown = TRUE;
                    break;
                case VK_LEFT:
                    m_keyLeft = TRUE;
                    break;
                case VK_RIGHT:
                    m_keyRight = TRUE;
                    break;
                }
            }
            else
            {
                // Для всех остальных клавиш возвращаемся в режим редактирования
                // Получаем дескриптор главного окна из родительского окна
                HWND hMainWnd = GetParent(hWnd);
                if (hMainWnd)
                {
                    returnToEditMode(hMainWnd, wParam);
                }
            }
        }
        break;
        case WM_KEYUP:
        {
            // Обрабатываем отпускание только клавиш управления спрайтом
            if (isSpriteControlKey(wParam))
            {
                switch (wParam)
                {
                case 'W':
                case 'w':
                    m_keyW = FALSE;
                    break;
                case 'A':
                case 'a':
                    m_keyA = FALSE;
                    break;
                case 'S':
                case 's':
                    m_keyS = FALSE;
                    break;
                case 'D':
                case 'd':
                    m_keyD = FALSE;
                    break;
                case VK_UP:
                    m_keyUp = FALSE;
                    break;
                case VK_DOWN:
                    m_keyDown = FALSE;
                    break;
                case VK_LEFT:
                    m_keyLeft = FALSE;
                    break;
                case VK_RIGHT:
                    m_keyRight = FALSE;
                    break;
                }
            }
        }
        break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            hideDarkScreen();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    
    // Если это не сообщение от темного экрана, возвращаем 0 (не обработано)
    return 0;
}

void DarkScreenManager::handleUserActivity(HWND hMainWnd)
{
    // Сбрасываем таймер при активности пользователя
    if (m_isDarkScreenActive)
    {
        hideDarkScreen();
    }
    setIdleTimer(hMainWnd, IDLE_TIMEOUT);
}

void DarkScreenManager::setIdleTimer(HWND hWnd, UINT timeout)
{
    KillTimer(hWnd, TIMER_IDLE);
    SetTimer(hWnd, TIMER_IDLE, timeout, NULL);
}

void DarkScreenManager::killIdleTimer(HWND hWnd)
{
    KillTimer(hWnd, TIMER_IDLE);
}

void DarkScreenManager::handleTimer(HWND hWnd, UINT_PTR timerId)
{
    if (timerId == TIMER_IDLE && !m_isDarkScreenActive)
    {
        showDarkScreen(hWnd);
    }
}

void DarkScreenManager::updateSprite()
{
    if (!m_hDarkScreen)
    {
        return;
    }

    RECT rect;
    GetClientRect(m_hDarkScreen, &rect);

    // Вычисляем скорость на основе нажатых клавиш
    int velocityX = 0;
    int velocityY = 0;

    // Обработка клавиш WASD и стрелок
    if (m_keyA || m_keyLeft)
    {
        velocityX -= SPRITE_SPEED;
    }
    if (m_keyD || m_keyRight)
    {
        velocityX += SPRITE_SPEED;
    }
    if (m_keyW || m_keyUp)
    {
        velocityY -= SPRITE_SPEED;
    }
    if (m_keyS || m_keyDown)
    {
        velocityY += SPRITE_SPEED;
    }

    // Обновляем позицию спрайта
    m_spritePos.x += velocityX;
    m_spritePos.y += velocityY;

    // Ограничиваем позицию в пределах окна
    if (m_spritePos.x < 10) m_spritePos.x = 10;
    if (m_spritePos.x > rect.right - 10) m_spritePos.x = rect.right - 10;
    if (m_spritePos.y < 10) m_spritePos.y = 10;
    if (m_spritePos.y > rect.bottom - 10) m_spritePos.y = rect.bottom - 10;
}

LRESULT CALLBACK DarkScreenManager::darkScreenProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DarkScreenManager* instance = getInstanceFromWindow(hWnd);
    if (instance)
    {
        return instance->handleDarkScreenMessage(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


BOOL DarkScreenManager::isSpriteControlKey(WPARAM vkCode) const
{
    switch (vkCode)
    {
    case 'W':
    case 'w':
    case 'A':
    case 'a':
    case 'S':
    case 's':
    case 'D':
    case 'd':
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        return TRUE;
    default:
        return FALSE;
    }
}

void DarkScreenManager::returnToEditMode(HWND hMainWnd, WPARAM vkCode)
{
    // Скрываем темный экран
    hideDarkScreen();
    
    // Находим EditControl в главном окне
    HWND hEditControl = FindWindowEx(hMainWnd, NULL, L"EDIT", NULL);
    if (hEditControl)
    {
        // Устанавливаем фокус на EditControl
        SetFocus(hEditControl);
        
        // Если была нажата клавиша, передаем её в EditControl
        if (vkCode != 0)
        {
            // Отправляем сообщение о нажатии клавиши в EditControl
            SendMessage(hEditControl, WM_KEYDOWN, vkCode, 0);
        }
    }
}

DarkScreenManager* DarkScreenManager::getInstanceFromWindow(HWND hWnd)
{
    return (DarkScreenManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}
