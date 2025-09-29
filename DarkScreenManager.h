#pragma once

#include "framework.h"

/**
 * @brief Менеджер для управления темным экраном
 * 
 * Отвечает за показ/скрытие темного экрана с анимированным спрайтом
 * при неактивности пользователя.
 */
class DarkScreenManager
{
public:
    /**
     * @brief Конструктор
     * @param hInstance Дескриптор экземпляра приложения
     */
    DarkScreenManager(HINSTANCE hInstance);

    /**
     * @brief Деструктор
     */
    ~DarkScreenManager();

    /**
     * @brief Показать темный экран
     * @param hParent Дескриптор родительского окна
     */
    void showDarkScreen(HWND hParent);

    /**
     * @brief Скрыть темный экран
     */
    void hideDarkScreen();

    /**
     * @brief Проверить, активен ли темный экран
     * @return TRUE если темный экран активен, FALSE в противном случае
     */
    BOOL isDarkScreenActive() const;

    /**
     * @brief Обработать сообщения темного экрана
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    LRESULT handleDarkScreenMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


    /**
     * @brief Обработать активность пользователя
     * @param hMainWnd Дескриптор главного окна
     */
    void handleUserActivity(HWND hMainWnd);

    /**
     * @brief Установить таймер неактивности
     * @param hWnd Дескриптор окна
     * @param timeout Время неактивности в миллисекундах
     */
    void setIdleTimer(HWND hWnd, UINT timeout);

    /**
     * @brief Остановить таймер неактивности
     * @param hWnd Дескриптор окна
     */
    void killIdleTimer(HWND hWnd);

    /**
     * @brief Обработать таймер
     * @param hWnd Дескриптор окна
     * @param timerId ID таймера
     */
    void handleTimer(HWND hWnd, UINT_PTR timerId);

    /**
     * @brief Проверить, является ли клавиша клавишей управления спрайтом
     * @param vkCode Код виртуальной клавиши
     * @return TRUE если клавиша управляет спрайтом, FALSE в противном случае
     */
    BOOL isSpriteControlKey(WPARAM vkCode) const;

    /**
     * @brief Вернуть фокус в режим редактирования
     * @param hMainWnd Дескриптор главного окна
     * @param vkCode Код виртуальной клавиши для передачи в EditControl
     */
    void returnToEditMode(HWND hMainWnd, WPARAM vkCode = 0);

private:
    HINSTANCE m_hInstance;                    ///< Дескриптор экземпляра приложения
    HWND m_hDarkScreen;                       ///< Дескриптор темного экрана
    BOOL m_isDarkScreenActive;                ///< Флаг активности темного экрана
    POINT m_spritePos;                        ///< Позиция спрайта
    POINT m_spriteVelocity;                   ///< Скорость движения спрайта
    RECT m_clientRect;                        ///< Размеры клиентской области
    
    // Состояние клавиш управления
    BOOL m_keyW;                              ///< Состояние клавиши W
    BOOL m_keyA;                              ///< Состояние клавиши A
    BOOL m_keyS;                              ///< Состояние клавиши S
    BOOL m_keyD;                              ///< Состояние клавиши D
    BOOL m_keyUp;                             ///< Состояние клавиши стрелка вверх
    BOOL m_keyDown;                           ///< Состояние клавиши стрелка вниз
    BOOL m_keyLeft;                           ///< Состояние клавиши стрелка влево
    BOOL m_keyRight;                          ///< Состояние клавиши стрелка вправо

    static const UINT TIMER_IDLE = 1;         ///< ID таймера неактивности
    static const UINT TIMER_ANIMATION = 2;    ///< ID таймера анимации
    static const UINT IDLE_TIMEOUT = 5000;    ///< Время неактивности по умолчанию
    static const UINT ANIMATION_INTERVAL = 50; ///< Интервал анимации
    static const int SPRITE_SPEED = 5;        ///< Скорость движения спрайта

    /**
     * @brief Обновить позицию спрайта
     */
    void updateSprite();

    /**
     * @brief Процедура окна для темного экрана
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    static LRESULT CALLBACK darkScreenProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Получить указатель на объект DarkScreenManager из окна
     * @param hWnd Дескриптор окна
     * @return Указатель на DarkScreenManager или NULL
     */
    static DarkScreenManager* getInstanceFromWindow(HWND hWnd);
};
