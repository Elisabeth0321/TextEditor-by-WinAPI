#include "EditControlManager.h"
#include <functional>

EditControlManager::EditControlManager(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_hEditControl(NULL)
    , m_hFont(NULL)
{
}

EditControlManager::~EditControlManager()
{
    deleteCurrentFont();
}

BOOL EditControlManager::createEditControl(HWND hParent)
{
    m_hEditControl = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hParent,
        NULL,
        m_hInstance,
        NULL
    );

    if (m_hEditControl)
    {
        // Устанавливаем шрифт по умолчанию
        m_hFont = createDefaultFont();
        SendMessage(m_hEditControl, WM_SETFONT, (WPARAM)m_hFont, TRUE);
        
        // Устанавливаем фокус на EDIT-контрол
        SetFocus(m_hEditControl);
        return TRUE;
    }
    return FALSE;
}

void EditControlManager::resizeEditControl(HWND hParent)
{
    if (m_hEditControl)
    {
        RECT rect;
        GetClientRect(hParent, &rect);
        SetWindowPos(m_hEditControl, NULL, 0, 0, 
                    rect.right, rect.bottom, 
                    SWP_NOZORDER);
    }
}

HWND EditControlManager::getEditControl() const
{
    return m_hEditControl;
}

void EditControlManager::setText(const std::wstring& text)
{
    if (m_hEditControl)
    {
        SetWindowTextW(m_hEditControl, text.c_str());
    }
}

std::wstring EditControlManager::getText() const
{
    if (!m_hEditControl)
    {
        return L"";
    }

    int textLength = GetWindowTextLengthW(m_hEditControl);
    if (textLength == 0)
    {
        return L"";
    }

    WCHAR* textBuffer = new WCHAR[textLength + 1];
    GetWindowTextW(m_hEditControl, textBuffer, textLength + 1);
    
    std::wstring result(textBuffer);
    delete[] textBuffer;
    
    return result;
}

void EditControlManager::cutText()
{
    if (m_hEditControl)
    {
        SendMessage(m_hEditControl, WM_CUT, 0, 0);
    }
}

void EditControlManager::copyText()
{
    if (m_hEditControl)
    {
        SendMessage(m_hEditControl, WM_COPY, 0, 0);
    }
}

void EditControlManager::pasteText()
{
    if (m_hEditControl)
    {
        SendMessage(m_hEditControl, WM_PASTE, 0, 0);
    }
}

void EditControlManager::setFocus()
{
    if (m_hEditControl)
    {
        SetFocus(m_hEditControl);
    }
}

void EditControlManager::setFont(const std::wstring& fontName, int fontSize, BOOL isBold, BOOL isItalic)
{
    if (!m_hEditControl)
    {
        return;
    }

    deleteCurrentFont();

    m_hFont = CreateFontW(
        fontSize, 0, 0, 0, 
        isBold ? FW_BOLD : FW_NORMAL, 
        isItalic, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        fontName.c_str()
    );

    if (m_hFont)
    {
        SendMessage(m_hEditControl, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    }
}

int EditControlManager::getTextLength() const
{
    if (!m_hEditControl)
    {
        return 0;
    }
    return GetWindowTextLengthW(m_hEditControl);
}

BOOL EditControlManager::isEditControlMessage(WPARAM wParam, LPARAM lParam) const
{
    return (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == m_hEditControl);
}

void EditControlManager::onTextChanged(std::function<void()> callback)
{
    m_textChangedCallback = callback;
}

void EditControlManager::triggerTextChanged()
{
    if (m_textChangedCallback)
    {
        m_textChangedCallback();
    }
}

HFONT EditControlManager::createDefaultFont()
{
    return CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas"
    );
}

void EditControlManager::deleteCurrentFont()
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
}
