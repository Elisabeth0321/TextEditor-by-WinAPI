#pragma once

#include "framework.h"
#include <string>

/**
 * @brief Менеджер для работы с файлами
 * 
 * Отвечает за открытие, сохранение файлов и работу с различными кодировками.
 * Поддерживает UTF-8 и ANSI кодировки.
 */
class FileManager
{
public:
    /**
     * @brief Конструктор
     * @param hInstance Дескриптор экземпляра приложения
     */
    FileManager(HINSTANCE hInstance);

    /**
     * @brief Деструктор
     */
    ~FileManager();

    /**
     * @brief Открыть текстовый файл
     * @param hWnd Дескриптор родительского окна
     * @return TRUE если файл успешно открыт, FALSE в противном случае
     */
    BOOL openTextFile(HWND hWnd);

    /**
     * @brief Сохранить текстовый файл
     * @param hWnd Дескриптор родительского окна
     * @return TRUE если файл успешно сохранен, FALSE в противном случае
     */
    BOOL saveTextFile(HWND hWnd);

    /**
     * @brief Сохранить файл с выбором имени
     * @param hWnd Дескриптор родительского окна
     * @return TRUE если файл успешно сохранен, FALSE в противном случае
     */
    BOOL saveTextFileAs(HWND hWnd);

    /**
     * @brief Проверить, был ли файл изменен
     * @return TRUE если файл был изменен, FALSE в противном случае
     */
    BOOL isFileModified() const;

    /**
     * @brief Установить флаг изменения файла
     * @param modified TRUE если файл изменен, FALSE в противном случае
     */
    void setFileModified(BOOL modified);

    /**
     * @brief Проверить, есть ли открытый файл
     * @return TRUE если файл открыт, FALSE в противном случае
     */
    BOOL hasFileName() const;

    /**
     * @brief Получить имя текущего файла
     * @return Строка с именем файла
     */
    std::wstring getCurrentFileName() const;

    /**
     * @brief Получить короткое имя файла (без пути)
     * @return Строка с коротким именем файла
     */
    std::wstring getShortFileName() const;

    /**
     * @brief Запросить сохранение изменений
     * @param hWnd Дескриптор родительского окна
     * @return TRUE если можно продолжить, FALSE если операция отменена
     */
    BOOL promptSaveChanges(HWND hWnd);

    /**
     * @brief Установить содержимое файла
     * @param content Содержимое файла
     */
    void setFileContent(const std::wstring& content);

    /**
     * @brief Получить содержимое файла
     * @return Содержимое файла
     */
    std::wstring getFileContent() const;

private:
    HINSTANCE m_hInstance;                    ///< Дескриптор экземпляра приложения
    std::wstring m_currentFileName;           ///< Имя текущего файла
    std::wstring m_fileContent;               ///< Содержимое файла
    BOOL m_isFileModified;                    ///< Флаг изменения файла
    BOOL m_hasFileName;                       ///< Флаг наличия имени файла

    /**
     * @brief Центрировать диалоговое окно
     * @param hDlg Дескриптор диалогового окна
     */
    void centerDialog(HWND hDlg);

    /**
     * @brief Hook-функция для диалогов открытия/сохранения файлов
     * @param hDlg Дескриптор диалогового окна
     * @param message Сообщение
     * @param wParam Параметр wParam
     * @param lParam Параметр lParam
     * @return Результат обработки сообщения
     */
    static UINT_PTR CALLBACK fileDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Загрузить строку из ресурсов
     * @param resourceId ID ресурса
     * @return Загруженная строка
     */
    std::wstring loadStringFromResources(UINT resourceId);
};
