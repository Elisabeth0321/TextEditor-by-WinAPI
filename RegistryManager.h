#pragma once

#include <windows.h>
#include <string>

/**
 * @brief Класс для управления настройками приложения в реестре Windows
 */
class RegistryManager
{
private:
    static constexpr LPCWSTR REGISTRY_KEY_PATH = L"SOFTWARE\\TextEditor\\Settings";
    static constexpr LPCWSTR FONT_NAME_KEY = L"FontName";
    static constexpr LPCWSTR FONT_SIZE_KEY = L"FontSize";
    static constexpr LPCWSTR FONT_STYLE_KEY = L"FontStyle";
    static constexpr LPCWSTR TEXT_COLOR_KEY = L"TextColor";
    static constexpr LPCWSTR BACKGROUND_COLOR_KEY = L"BackgroundColor";
    static constexpr LPCWSTR LAST_FILE_KEY = L"LastFile";
    static constexpr LPCWSTR LAST_FILE_STATE_KEY = L"LastFileState";

    HKEY hKey;

public:
    RegistryManager();
    ~RegistryManager();

    // Методы для работы с шрифтом
    BOOL SaveFontSettings(const LOGFONTW& logFont);
    BOOL LoadFontSettings(LOGFONTW& logFont);

    // Методы для работы с цветами
    BOOL SaveTextColor(COLORREF color);
    BOOL LoadTextColor(COLORREF& color);
    BOOL SaveBackgroundColor(COLORREF color);
    BOOL LoadBackgroundColor(COLORREF& color);

    // Методы для работы с последним файлом
    BOOL SaveLastFile(const std::wstring& filePath);
    BOOL LoadLastFile(std::wstring& filePath);
    
    // Методы для работы с состоянием файла (открыт/новый)
    BOOL SaveLastFileState(BOOL hasFile);
    BOOL LoadLastFileState(BOOL& hasFile);

    // Общие методы
    BOOL OpenRegistryKey();
    void CloseRegistryKey();
    BOOL CreateRegistryKey();
};
