#pragma once

#include "framework.h"
#include "FileManager.h"
#include "EditControlManager.h"
#include "DarkScreenManager.h"
#include "WindowManager.h"
#include <memory>

/**
 * @brief Главный класс приложения
 * 
 * Координирует работу всех модулей приложения:
 * - WindowManager - управление окнами
 * - FileManager - работа с файлами
 * - EditControlManager - управление текстовым редактором
 * - DarkScreenManager - управление темным экраном
 */
class Application
{
public:
    /**
     * @brief Конструктор
     * @param hInstance Дескриптор экземпляра приложения
     */
    Application(HINSTANCE hInstance);

    /**
     * @brief Деструктор
     */
    ~Application();

    /**
     * @brief Инициализировать приложение
     * @return TRUE если инициализация успешна, FALSE в противном случае
     */
    BOOL initialize();

    /**
     * @brief Запустить главный цикл приложения
     * @param nCmdShow Параметр показа окна
     * @return Код завершения приложения
     */
    int run(int nCmdShow);

    /**
     * @brief Обработать команду меню
     * @param commandId ID команды
     */
    void handleMenuCommand(int commandId);

    /**
     * @brief Обработать изменение размера окна
     * @param hWnd Дескриптор окна
     */
    void handleWindowResize(HWND hWnd);

    /**
     * @brief Обработать изменение текста
     */
    void handleTextChange();

    /**
     * @brief Обработать активность пользователя
     * @param hWnd Дескриптор окна
     */
    void handleUserActivity(HWND hWnd);

    /**
     * @brief Обработать таймер
     * @param hWnd Дескриптор окна
     * @param timerId ID таймера
     */
    void handleTimer(HWND hWnd, UINT_PTR timerId);

    /**
     * @brief Обработать закрытие окна
     * @param hWnd Дескриптор окна
     * @return TRUE если окно можно закрыть, FALSE в противном случае
     */
    BOOL handleWindowClose(HWND hWnd);

    /**
     * @brief Обновить заголовок окна
     */
    void updateWindowTitle();

    /**
     * @brief Получить дескриптор главного окна
     * @return Дескриптор главного окна
     */
    HWND getMainWindow() const;

private:
    HINSTANCE m_hInstance;                    ///< Дескриптор экземпляра приложения
    
    // Модули приложения
    std::unique_ptr<WindowManager> m_windowManager;           ///< Менеджер окон
    std::unique_ptr<FileManager> m_fileManager;               ///< Менеджер файлов
    std::unique_ptr<EditControlManager> m_editControlManager; ///< Менеджер текстового редактора
    std::unique_ptr<DarkScreenManager> m_darkScreenManager;   ///< Менеджер темного экрана

    /**
     * @brief Настроить обработчики событий
     */
    void setupEventHandlers();

    /**
     * @brief Инициализировать локализацию
     */
    void initializeLocalization();

    /**
     * @brief Инициализировать Common Controls
     */
    void initializeCommonControls();

    /**
     * @brief Обработать команду открытия файла
     */
    void handleOpenFile();

    /**
     * @brief Обработать команду сохранения файла
     */
    void handleSaveFile();

    /**
     * @brief Обработать команду вырезания текста
     */
    void handleCutText();

    /**
     * @brief Обработать команду копирования текста
     */
    void handleCopyText();

    /**
     * @brief Обработать команду вставки текста
     */
    void handlePasteText();

    /**
     * @brief Обработать команду "О программе"
     */
    void handleAbout();

    /**
     * @brief Обработать команду выхода
     */
    void handleExit();
};
