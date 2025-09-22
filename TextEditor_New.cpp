// TextEditor_New.cpp : Defines the entry point for the application.
// -*- coding: utf-8 -*-

#include "framework.h"
#include "Application.h"

/**
 * @brief Точка входа в приложение
 * @param hInstance Дескриптор экземпляра приложения
 * @param hPrevInstance Дескриптор предыдущего экземпляра (не используется)
 * @param lpCmdLine Командная строка
 * @param nCmdShow Параметр показа окна
 * @return Код завершения приложения
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    try
    {
        // Создаем экземпляр приложения
        Application app(hInstance);

        // Инициализируем приложение
        if (!app.initialize())
        {
            MessageBoxW(NULL, L"Ошибка инициализации приложения", L"Ошибка", MB_OK | MB_ICONERROR);
            return -1;
        }

        // Запускаем главный цикл приложения
        return app.run(nCmdShow);
    }
    catch (const std::exception& e)
    {
        // Обработка исключений C++
        std::string errorMsg = "Критическая ошибка: ";
        errorMsg += e.what();
        
        // Конвертируем в Unicode для MessageBox
        int wideSize = MultiByteToWideChar(CP_UTF8, 0, errorMsg.c_str(), -1, NULL, 0);
        WCHAR* wideBuffer = new WCHAR[wideSize];
        MultiByteToWideChar(CP_UTF8, 0, errorMsg.c_str(), -1, wideBuffer, wideSize);
        
        MessageBoxW(NULL, wideBuffer, L"Критическая ошибка", MB_OK | MB_ICONERROR);
        delete[] wideBuffer;
        
        return -1;
    }
    catch (...)
    {
        // Обработка неизвестных исключений
        MessageBoxW(NULL, L"Произошла неизвестная ошибка", L"Критическая ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }
}
