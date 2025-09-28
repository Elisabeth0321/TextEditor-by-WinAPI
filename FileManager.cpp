#include "FileManager.h"
#include "Resource.h"
#include <commdlg.h>

FileManager::FileManager(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_isFileModified(FALSE)
    , m_hasFileName(FALSE)
{
}

FileManager::~FileManager()
{
}

BOOL FileManager::openTextFile(HWND hWnd)
{
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = { 0 };
    
    std::wstring title = loadStringFromResources(IDS_OPEN_FILE_TITLE);
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Все файлы\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn))
    {
        HANDLE hFile = CreateFileW(
            szFile,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE)
            {
                // Выделяем память для содержимого файла
                CHAR* buffer = (CHAR*)malloc(fileSize + 1);
                if (buffer)
                {
                    DWORD bytesRead;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL))
                    {
                        buffer[bytesRead] = '\0';
                        
                        // Конвертируем в Unicode
                        int wideSize = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
                        WCHAR* wideBuffer = (WCHAR*)malloc(wideSize * sizeof(WCHAR));
                        if (wideBuffer)
                        {
                            MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wideBuffer, wideSize);
                            
                            // Сохраняем содержимое файла
                            m_fileContent = std::wstring(wideBuffer);
                            
                            // Сохраняем имя файла
                            m_currentFileName = std::wstring(szFile);
                            m_hasFileName = TRUE;
                            m_isFileModified = FALSE;
                            
                            free(wideBuffer);
                        }
                    }
                    free(buffer);
                }
            }
            CloseHandle(hFile);
            return TRUE;
        }
        else
        {
            MessageBoxW(hWnd, L"Не удалось открыть файл", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
    return FALSE;
}

BOOL FileManager::saveTextFile(HWND hWnd)
{
    if (!m_hasFileName)
    {
        return saveTextFileAs(hWnd);
    }

    // Конвертируем в UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_fileContent.c_str(), -1, NULL, 0, NULL, NULL);
    CHAR* utf8Buffer = (CHAR*)malloc(utf8Size);
    if (!utf8Buffer)
    {
        MessageBoxW(hWnd, L"Недостаточно памяти", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    WideCharToMultiByte(CP_UTF8, 0, m_fileContent.c_str(), -1, utf8Buffer, utf8Size, NULL, NULL);

    // Создаем/перезаписываем файл
    HANDLE hFile = CreateFileW(
        m_currentFileName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    BOOL success = FALSE;
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        if (WriteFile(hFile, utf8Buffer, (DWORD)strlen(utf8Buffer), &bytesWritten, NULL))
        {
            m_isFileModified = FALSE;
            success = TRUE;
        }
        else
        {
            MessageBoxW(hWnd, L"Ошибка при записи файла", L"Ошибка", MB_OK | MB_ICONERROR);
        }
        CloseHandle(hFile);
    }
    else
    {
        MessageBoxW(hWnd, L"Не удалось создать файл", L"Ошибка", MB_OK | MB_ICONERROR);
    }

    free(utf8Buffer);
    return success;
}

BOOL FileManager::saveTextFileAs(HWND hWnd)
{
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = { 0 };

    // Если есть текущее имя файла, используем его
    if (m_hasFileName)
    {
        wcscpy_s(szFile, MAX_PATH, m_currentFileName.c_str());
    }

    std::wstring title = loadStringFromResources(IDS_SAVE_FILE_TITLE);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Все файлы\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = L""; // Без расширения по умолчанию, пользователь сам выберет

    if (GetSaveFileName(&ofn))
    {
        m_currentFileName = std::wstring(szFile);
        m_hasFileName = TRUE;
        return saveTextFile(hWnd);
    }
    return FALSE;
}

BOOL FileManager::isFileModified() const
{
    return m_isFileModified;
}

void FileManager::setFileModified(BOOL modified)
{
    m_isFileModified = modified;
}

BOOL FileManager::hasFileName() const
{
    return m_hasFileName;
}

std::wstring FileManager::getCurrentFileName() const
{
    return m_currentFileName;
}

std::wstring FileManager::getShortFileName() const
{
    if (!m_hasFileName)
    {
        return L"";
    }

    size_t pos = m_currentFileName.find_last_of(L"\\");
    if (pos != std::wstring::npos)
    {
        return m_currentFileName.substr(pos + 1);
    }
    return m_currentFileName;
}

BOOL FileManager::promptSaveChanges(HWND hWnd)
{
    if (!m_isFileModified)
    {
        return TRUE; // Нет изменений, продолжаем
    }

    WCHAR message[512];
    if (m_hasFileName)
    {
        std::wstring shortName = getShortFileName();
        swprintf_s(message, 512, L"Файл \"%s\" был изменен.\n\nСохранить изменения?", 
                  shortName.c_str());
    }
    else
    {
        wcscpy_s(message, 512, L"Документ был изменен.\n\nСохранить изменения?");
    }

    int result = MessageBoxW(hWnd, message, L"Сохранение", 
                            MB_YESNOCANCEL | MB_ICONQUESTION);

    switch (result)
    {
    case IDYES:
        return saveTextFile(hWnd);
    case IDNO:
        return TRUE;
    case IDCANCEL:
    default:
        return FALSE;
    }
}

void FileManager::setFileContent(const std::wstring& content)
{
    m_fileContent = content;
}

std::wstring FileManager::getFileContent() const
{
    return m_fileContent;
}

void FileManager::centerDialog(HWND hDlg)
{
    RECT dialogRect;
    GetWindowRect(hDlg, &dialogRect);

    // Получаем размеры экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Вычисляем позицию для центрирования
    int dialogWidth = dialogRect.right - dialogRect.left;
    int dialogHeight = dialogRect.bottom - dialogRect.top;
    int x = (screenWidth - dialogWidth) / 2;
    int y = (screenHeight - dialogHeight) / 2;

    // Устанавливаем новую позицию
    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

UINT_PTR CALLBACK FileManager::fileDialogHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // Получаем указатель на объект FileManager из lParam
        // В данном случае используем статический метод для центрирования
        RECT dialogRect;
        GetWindowRect(hDlg, &dialogRect);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int dialogWidth = dialogRect.right - dialogRect.left;
        int dialogHeight = dialogRect.bottom - dialogRect.top;
        int x = (screenWidth - dialogWidth) / 2;
        int y = (screenHeight - dialogHeight) / 2;

        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        break;
    }
    return 0;
}

std::wstring FileManager::loadStringFromResources(UINT resourceId)
{
    const int MAX_LOADSTRING = 100;
    CHAR titleAnsi[MAX_LOADSTRING];
    WCHAR titleWide[MAX_LOADSTRING];
    
    LoadStringA(m_hInstance, resourceId, titleAnsi, MAX_LOADSTRING);
    MultiByteToWideChar(CP_ACP, 0, titleAnsi, -1, titleWide, MAX_LOADSTRING);
    
    return std::wstring(titleWide);
}
