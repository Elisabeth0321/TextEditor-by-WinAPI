#pragma once

#include "framework.h"
#include <string>

/**
 * @brief Менеджер для управления текстовым редактором
 * 
 * Отвечает за создание, управление и взаимодействие с EDIT-контролом.
 * Предоставляет методы для работы с текстом, шрифтами и размерами.
 */
class EditControlManager
{
public:
    /**
     * @brief Конструктор
     * @param hInstance Дескриптор экземпляра приложения
     */
    EditControlManager(HINSTANCE hInstance);

    /**
     * @brief Деструктор
     */
    ~EditControlManager();

    /**
     * @brief Создать многострочный EDIT-контрол
     * @param hParent Дескриптор родительского окна
     * @return TRUE если контрол успешно создан, FALSE в противном случае
     */
    BOOL createEditControl(HWND hParent);

    /**
     * @brief Изменить размер EDIT-контрола
     * @param hParent Дескриптор родительского окна
     */
    void resizeEditControl(HWND hParent);

    /**
     * @brief Получить дескриптор EDIT-контрола
     * @return Дескриптор контрола
     */
    HWND getEditControl() const;

    /**
     * @brief Установить текст в контрол
     * @param text Текст для установки
     */
    void setText(const std::wstring& text);

    /**
     * @brief Получить текст из контрола
     * @return Текст из контрола
     */
    std::wstring getText() const;

    /**
     * @brief Вырезать выделенный текст
     */
    void cutText();

    /**
     * @brief Копировать выделенный текст
     */
    void copyText();

    /**
     * @brief Вставить текст из буфера обмена
     */
    void pasteText();

    /**
     * @brief Установить фокус на контрол
     */
    void setFocus();

    /**
     * @brief Установить шрифт для контрола
     * @param fontName Имя шрифта
     * @param fontSize Размер шрифта
     * @param isBold Жирный шрифт
     * @param isItalic Курсивный шрифт
     */
    void setFont(const std::wstring& fontName, int fontSize, BOOL isBold = FALSE, BOOL isItalic = FALSE);

    /**
     * @brief Получить длину текста в контроле
     * @return Длина текста
     */
    int getTextLength() const;

    /**
     * @brief Проверить, является ли сообщение от EDIT-контрола
     * @param wParam Параметр wParam сообщения
     * @param lParam Параметр lParam сообщения
     * @return TRUE если сообщение от EDIT-контрола
     */
    BOOL isEditControlMessage(WPARAM wParam, LPARAM lParam) const;

    /**
     * @brief Обработать изменение текста в контроле
     * @param callback Функция обратного вызова для уведомления об изменениях
     */
    void onTextChanged(std::function<void()> callback);

    /**
     * @brief Вызвать callback при изменении текста
     */
    void triggerTextChanged();

private:
    HINSTANCE m_hInstance;                    ///< Дескриптор экземпляра приложения
    HWND m_hEditControl;                      ///< Дескриптор EDIT-контрола
    HFONT m_hFont;                           ///< Дескриптор шрифта
    std::function<void()> m_textChangedCallback; ///< Функция обратного вызова для изменений

    /**
     * @brief Создать шрифт по умолчанию
     * @return Дескриптор созданного шрифта
     */
    HFONT createDefaultFont();

    /**
     * @brief Удалить текущий шрифт
     */
    void deleteCurrentFont();
};
