#include "RegistryManager.h"
#include <iostream>

RegistryManager::RegistryManager() : hKey(nullptr)
{
}

RegistryManager::~RegistryManager()
{
    CloseRegistryKey();
}

BOOL RegistryManager::OpenRegistryKey()
{
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_KEY_PATH,
        0,
        KEY_READ | KEY_WRITE,
        &hKey
    );

    if (result != ERROR_SUCCESS)
    {
        // Если ключ не существует, создаем его
        return CreateRegistryKey();
    }

    return TRUE;
}

BOOL RegistryManager::CreateRegistryKey()
{
    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        REGISTRY_KEY_PATH,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        nullptr,
        &hKey,
        nullptr
    );

    return (result == ERROR_SUCCESS);
}

void RegistryManager::CloseRegistryKey()
{
    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = nullptr;
    }
}

BOOL RegistryManager::SaveFontSettings(const LOGFONTW& logFont)
{
    if (!OpenRegistryKey())
        return FALSE;

    BOOL success = TRUE;

    // Сохраняем имя шрифта
    DWORD dataSize = static_cast<DWORD>((wcslen(logFont.lfFaceName) + 1) * sizeof(WCHAR));
    if (RegSetValueExW(hKey, FONT_NAME_KEY, 0, REG_SZ, 
                      (const BYTE*)logFont.lfFaceName, dataSize) != ERROR_SUCCESS)
    {
        success = FALSE;
    }

    // Сохраняем размер шрифта
    DWORD fontSize = static_cast<DWORD>(logFont.lfHeight);
    if (RegSetValueExW(hKey, FONT_SIZE_KEY, 0, REG_DWORD, 
                      (const BYTE*)&fontSize, sizeof(DWORD)) != ERROR_SUCCESS)
    {
        success = FALSE;
    }

    // Сохраняем стиль шрифта (жирный, курсив и т.д.)
    DWORD fontStyle = 0;
    if (logFont.lfWeight >= FW_BOLD)
        fontStyle |= 0x01; // Жирный
    if (logFont.lfItalic)
        fontStyle |= 0x02; // Курсив
    if (logFont.lfUnderline)
        fontStyle |= 0x04; // Подчеркнутый
    if (logFont.lfStrikeOut)
        fontStyle |= 0x08; // Зачеркнутый

    if (RegSetValueExW(hKey, FONT_STYLE_KEY, 0, REG_DWORD, 
                      (const BYTE*)&fontStyle, sizeof(DWORD)) != ERROR_SUCCESS)
    {
        success = FALSE;
    }

    return success;
}

BOOL RegistryManager::LoadFontSettings(LOGFONTW& logFont)
{
    if (!OpenRegistryKey())
        return FALSE;

    BOOL success = TRUE;

    // Загружаем имя шрифта
    WCHAR fontName[LF_FACESIZE] = { 0 };
    DWORD dataSize = sizeof(fontName);
    if (RegQueryValueExW(hKey, FONT_NAME_KEY, nullptr, nullptr, 
                        (BYTE*)fontName, &dataSize) == ERROR_SUCCESS)
    {
        wcscpy_s(logFont.lfFaceName, LF_FACESIZE, fontName);
    }
    else
    {
        // Значение по умолчанию
        wcscpy_s(logFont.lfFaceName, LF_FACESIZE, L"Consolas");
        success = FALSE;
    }

    // Загружаем размер шрифта
    DWORD fontSize = 16; // Значение по умолчанию
    dataSize = sizeof(DWORD);
    if (RegQueryValueExW(hKey, FONT_SIZE_KEY, nullptr, nullptr, 
                        (BYTE*)&fontSize, &dataSize) == ERROR_SUCCESS)
    {
        logFont.lfHeight = static_cast<LONG>(fontSize);
    }
    else
    {
        logFont.lfHeight = 16;
        success = FALSE;
    }

    // Загружаем стиль шрифта
    DWORD fontStyle = 0;
    dataSize = sizeof(DWORD);
    if (RegQueryValueExW(hKey, FONT_STYLE_KEY, nullptr, nullptr, 
                        (BYTE*)&fontStyle, &dataSize) == ERROR_SUCCESS)
    {
        logFont.lfWeight = (fontStyle & 0x01) ? FW_BOLD : FW_NORMAL;
        logFont.lfItalic = (fontStyle & 0x02) ? TRUE : FALSE;
        logFont.lfUnderline = (fontStyle & 0x04) ? TRUE : FALSE;
        logFont.lfStrikeOut = (fontStyle & 0x08) ? TRUE : FALSE;
    }
    else
    {
        logFont.lfWeight = FW_NORMAL;
        logFont.lfItalic = FALSE;
        logFont.lfUnderline = FALSE;
        logFont.lfStrikeOut = FALSE;
        success = FALSE;
    }

    // Устанавливаем остальные параметры по умолчанию
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality = DEFAULT_QUALITY;
    logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    return success;
}

BOOL RegistryManager::SaveTextColor(COLORREF color)
{
    if (!OpenRegistryKey())
        return FALSE;

    return (RegSetValueExW(hKey, TEXT_COLOR_KEY, 0, REG_DWORD, 
                          (const BYTE*)&color, sizeof(DWORD)) == ERROR_SUCCESS);
}

BOOL RegistryManager::LoadTextColor(COLORREF& color)
{
    if (!OpenRegistryKey())
    {
        color = RGB(0, 0, 0); // Черный по умолчанию
        return FALSE;
    }

    DWORD dataSize = sizeof(DWORD);
    if (RegQueryValueExW(hKey, TEXT_COLOR_KEY, nullptr, nullptr, 
                        (BYTE*)&color, &dataSize) == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        color = RGB(0, 0, 0); // Черный по умолчанию
        return FALSE;
    }
}

BOOL RegistryManager::SaveBackgroundColor(COLORREF color)
{
    if (!OpenRegistryKey())
        return FALSE;

    return (RegSetValueExW(hKey, BACKGROUND_COLOR_KEY, 0, REG_DWORD, 
                          (const BYTE*)&color, sizeof(DWORD)) == ERROR_SUCCESS);
}

BOOL RegistryManager::LoadBackgroundColor(COLORREF& color)
{
    if (!OpenRegistryKey())
    {
        color = RGB(255, 255, 255); // Белый по умолчанию
        return FALSE;
    }

    DWORD dataSize = sizeof(DWORD);
    if (RegQueryValueExW(hKey, BACKGROUND_COLOR_KEY, nullptr, nullptr, 
                        (BYTE*)&color, &dataSize) == ERROR_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        color = RGB(255, 255, 255); // Белый по умолчанию
        return FALSE;
    }
}

BOOL RegistryManager::SaveLastFile(const std::wstring& filePath)
{
    if (!OpenRegistryKey())
        return FALSE;

    DWORD dataSize = static_cast<DWORD>((filePath.length() + 1) * sizeof(WCHAR));
    return (RegSetValueExW(hKey, LAST_FILE_KEY, 0, REG_SZ, 
                          (const BYTE*)filePath.c_str(), dataSize) == ERROR_SUCCESS);
}

BOOL RegistryManager::LoadLastFile(std::wstring& filePath)
{
    if (!OpenRegistryKey())
    {
        filePath.clear();
        return FALSE;
    }

    WCHAR buffer[MAX_PATH] = { 0 };
    DWORD dataSize = sizeof(buffer);
    
    if (RegQueryValueExW(hKey, LAST_FILE_KEY, nullptr, nullptr, 
                        (BYTE*)buffer, &dataSize) == ERROR_SUCCESS)
    {
        filePath = buffer;
        return TRUE;
    }
    else
    {
        filePath.clear();
        return FALSE;
    }
}

BOOL RegistryManager::SaveLastFileState(BOOL hasFile)
{
    if (!OpenRegistryKey())
        return FALSE;

    DWORD fileState = hasFile ? 1 : 0;
    return (RegSetValueExW(hKey, LAST_FILE_STATE_KEY, 0, REG_DWORD, 
                          (const BYTE*)&fileState, sizeof(DWORD)) == ERROR_SUCCESS);
}

BOOL RegistryManager::LoadLastFileState(BOOL& hasFile)
{
    if (!OpenRegistryKey())
    {
        hasFile = FALSE; // По умолчанию считаем, что файл не открыт
        return FALSE;
    }

    DWORD fileState = 0;
    DWORD dataSize = sizeof(DWORD);
    
    if (RegQueryValueExW(hKey, LAST_FILE_STATE_KEY, nullptr, nullptr, 
                        (BYTE*)&fileState, &dataSize) == ERROR_SUCCESS)
    {
        hasFile = (fileState != 0);
        return TRUE;
    }
    else
    {
        hasFile = FALSE; // По умолчанию считаем, что файл не открыт
        return FALSE;
    }
}