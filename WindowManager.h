#pragma once

#include "framework.h"
#include <string>
#include <functional>

/**
 * @brief Менеджер для управления окнами и диалогами
 * 
 * Отвечает за создание главного окна, управление заголовками,
 * центрирование диалогов и обработку сообщений окна.
 */
class WindowManager
{
public:
    /**
     * @brief Конструктор
     * @param hInstance Дескриптор экземпляра приложения
     */
    WindowManager(HINSTANCE hInstance);

    /**
     * @brief Деструктор
     */
    ~WindowManager();

    /**
     * @brief Зарегистрировать класс окна
     * @return TRUE если класс успешно зарегистрирован, FALSE в противном случае
     */
    BOOL registerWindowClass();

    /**
     * @brief Создать главное окно
     * @param nCmdShow Параметр показа окна
     * @return TRUE если окно успешно создано, FALSE в противном случае
     */
    BOOL createMainWindow(int nCmdShow);

    /**
     * @brief Получить дескриптор главного окна
     * @return Дескриптор главного окна
     */
    HWND getMainWindow() const;

    /**
     * @brief Обновить заголовок окна
     * @param fileName Имя файла (может быть пустым)
     * @param isModified Флаг изменения файла
     */
    void updateWindowTitle(const std::wstring& fileName, BOOL isModified);

    /**
     * @brief Показать диалог "О программе"
     * @param hWnd Дескриптор родительского окна
     */
    void showAboutDialog(HWND hWnd);

    /**
     * @brief Обработать сообщения главного окна
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    LRESULT handleMainWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Установить обработчик команд меню
     * @param commandHandler Функция для обработки команд меню
     */
    void setCommandHandler(std::function<void(int)> commandHandler);

    /**
     * @brief Установить обработчик изменения размера окна
     * @param resizeHandler Функция для обработки изменения размера
     */
    void setResizeHandler(std::function<void(HWND)> resizeHandler);

    /**
     * @brief Установить обработчик изменения текста
     * @param textChangeHandler Функция для обработки изменения текста
     */
    void setTextChangeHandler(std::function<void()> textChangeHandler);

    /**
     * @brief Установить обработчик сообщений от EDIT-контрола
     * @param editControlHandler Функция для обработки сообщений от EDIT-контрола
     */
    void setEditControlHandler(std::function<BOOL(WPARAM, LPARAM)> editControlHandler);

    /**
     * @brief Установить обработчик таймера
     * @param timerHandler Функция для обработки таймера
     */
    void setTimerHandler(std::function<void(HWND, UINT_PTR)> timerHandler);

    /**
     * @brief Установить обработчик активности пользователя
     * @param userActivityHandler Функция для обработки активности пользователя
     */
    void setUserActivityHandler(std::function<void(HWND)> userActivityHandler);

    /**
     * @brief Установить обработчик закрытия окна
     * @param closeHandler Функция для обработки закрытия окна
     */
    void setCloseHandler(std::function<BOOL(HWND)> closeHandler);

    /**
     * @brief Установить обработчик темного экрана
     * @param darkScreenHandler Функция для обработки сообщений темного экрана
     */
    void setDarkScreenHandler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> darkScreenHandler);

    /**
     * @brief Обработать сообщения от темного экрана
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    LRESULT handleDarkScreenMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Получить размеры клиентской области
     * @return Размеры клиентской области
     */
    RECT getClientRect() const;

private:
    HINSTANCE m_hInstance;                    ///< Дескриптор экземпляра приложения
    HWND m_hMainWnd;                         ///< Дескриптор главного окна
    std::wstring m_windowTitle;              ///< Заголовок окна
    std::wstring m_appTitle;                 ///< Название приложения
    std::wstring m_windowClass;              ///< Имя класса окна

    // Обработчики событий
    std::function<void(int)> m_commandHandler;        ///< Обработчик команд меню
    std::function<void(HWND)> m_resizeHandler;        ///< Обработчик изменения размера
    std::function<void()> m_textChangeHandler;        ///< Обработчик изменения текста
    std::function<BOOL(WPARAM, LPARAM)> m_editControlHandler; ///< Обработчик сообщений от EDIT-контрола
    std::function<void(HWND, UINT_PTR)> m_timerHandler; ///< Обработчик таймера
    std::function<void(HWND)> m_userActivityHandler;  ///< Обработчик активности пользователя
    std::function<BOOL(HWND)> m_closeHandler;         ///< Обработчик закрытия окна
    std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> m_darkScreenHandler; ///< Обработчик темного экрана

    static const int MAX_LOADSTRING = 100;

    /**
     * @brief Загрузить строки из ресурсов
     */
    void loadStringsFromResources();

    /**
     * @brief Центрировать диалоговое окно
     * @param hDlg Дескриптор диалогового окна
     */
    void centerDialog(HWND hDlg);

    /**
     * @brief Процедура окна для главного окна
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    static LRESULT CALLBACK mainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Процедура диалога "О программе"
     * @param hDlg Дескриптор диалога
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    static INT_PTR CALLBACK aboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Получить указатель на объект WindowManager из окна
     * @param hWnd Дескриптор окна
     * @return Указатель на WindowManager или NULL
     */
    static WindowManager* getInstanceFromWindow(HWND hWnd);
};
