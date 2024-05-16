// ✔ Compile with: gcc main.c bitmap.c -o C-To-Do-List.exe resource.o -mwindows -lgdi32 -luser32
// Compile resource with: windres resource.rc resource.o
// And: gcc -o myapp main.c resource.o -lgdi32
// ✖ Previous compilation flags:  gcc main.c bitmap.c -o C-To-Do-List.exe -mwindows
// Reference https://learn.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program

#include "resource.h"
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <ctype.h> // Include ctype.h for isspace
#include <time.h> // Add this line at the beginning of main.c

// bitmap.h
#ifndef BITMAP_H // Prevent multiple inclusions
#define BITMAP_H
#include <windows.h> // Include Windows.h here or in each C file as needed
HBITMAP CreateCheckboxBitmap(HDC hdc, BOOL checked);
#endif // BITMAP_H

#define MAX_TODO_LENGTH 42
#define MAX_TODOS 9999
#define IDC_ADD_BUTTON 101
#define IDC_DELETE_BUTTON 102
#define IDC_INPUT_EDIT 103 // Define IDC_INPUT_EDIT
#define ID_TOGGLE_BUTTON 1 // New ID for the settings button
// #define IDC_SETTINGS_APPLY_BUTTON 105
// #define ID_TOGGLE_BUTTON 1




char newTodoText[MAX_TODO_LENGTH] = {0};
int newTodoTextLength = 0;
int selectedTodoIndex = -1; // No todo selected initially
// These are the new starting positions for the "X" button and checkbox relative to each todo item
int deleteButtonOffset = 540; // Offset for the delete "X" button from startX
int checkboxOffset = 500; // Offset for the checkbox from startX

typedef struct {
    char text1[MAX_TODO_LENGTH];
    char text2[MAX_TODO_LENGTH]; // New field for the second text
    BOOL completed;
    // char timestamp[20]; // Enough to hold timestamps in "YYYY-MM-DD HH:MM:SS" format
} TodoItem;



TodoItem todos[MAX_TODOS];
int numTodos = 0; // Initial number of todos
HWND hInputEdit; // Declare a handle for the first input field
HWND hSecondInputEdit; // Declare a handle for the second input field
void ApplyTheme(HWND hwnd, BOOL darkTheme);
void SaveDarkModeSetting(BOOL darkMode);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void SaveDarkModeSetting(BOOL darkMode) {
    FILE *file = fopen("config.txt", "w");
    if (file != NULL) {
        fprintf(file, "DarkMode=%d", darkMode);
        fclose(file);
    }
}

BOOL LoadDarkModeSetting() {
    FILE *file = fopen("config.txt", "r");
    if (file != NULL) {
        int darkMode = 0;
        fscanf(file, "DarkMode=%d", &darkMode);
        fclose(file);
        return darkMode != 0;
    }
    return FALSE; // Default to light mode if no config file exists
}

void LoadTodos() {
    FILE* file = fopen("todos.txt", "r");
    if (file != NULL) {
        char line[2 * MAX_TODO_LENGTH + 10]; // Adjust buffer size to handle two texts, a delimiter, and the completion status
        while (fgets(line, sizeof(line), file) != NULL && numTodos < MAX_TODOS) {
            char *token = strtok(line, "|");
            if (token != NULL) {
                strncpy(todos[numTodos].text1, token, MAX_TODO_LENGTH);
                token = strtok(NULL, "|");
                if (token != NULL) {
                    strncpy(todos[numTodos].text2, token, MAX_TODO_LENGTH);
                    token = strtok(NULL, "\n");
                    // Set default completion status to FALSE if not specified
                    todos[numTodos].completed = (token != NULL) ? atoi(token) : FALSE;
                    numTodos++;
                }
            }
        }
        fclose(file);
    }
}




void AddTodo(HWND hwnd) {
    if (numTodos < MAX_TODOS) {
        char buffer1[MAX_TODO_LENGTH];
        char buffer2[MAX_TODO_LENGTH];
        GetWindowText(hInputEdit, buffer1, sizeof(buffer1)); // Get text from the first input
        GetWindowText(hSecondInputEdit, buffer2, sizeof(buffer2)); // Get text from the second input

        strncpy(todos[numTodos].text1, buffer1, MAX_TODO_LENGTH - 1);
        todos[numTodos].text1[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination for first text
        strncpy(todos[numTodos].text2, buffer2, MAX_TODO_LENGTH - 1);
        todos[numTodos].text2[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination for second text
        todos[numTodos].completed = FALSE;

        numTodos++;
        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
        
        // Update file storage logic to handle two texts
        FILE* file = fopen("todos.txt", "a");
        if (file != NULL) {
            fprintf(file, "%s|%s\n", buffer1, buffer2); // Save both texts separated by a delimiter
            fclose(file);
        } else {
            MessageBox(hwnd, "Failed to open To-do file for writing.", "Error", MB_OK);
        }
    }
}


void DeleteTodo(HWND hwnd, int index) {
    if (index >= 0 && index < numTodos) {
        for (int i = index; i < numTodos - 1; ++i) {
            strcpy(todos[i].text1, todos[i + 1].text1);
            strcpy(todos[i].text2, todos[i + 1].text2);
            todos[i].completed = todos[i + 1].completed;
        }
        numTodos--;

        FILE* file = fopen("todos.txt", "w");
        if (file != NULL) {
            for (int i = 0; i < numTodos; ++i) {
                fprintf(file, "%s|%s|%d\n", todos[i].text1, todos[i].text2, todos[i].completed);
            }
            fclose(file);
        }
    }
}

void ToggleCompletion(HWND hwnd, int index) {
    if (index >= 0 && index < numTodos) {
        todos[index].completed = !todos[index].completed;
        InvalidateRect(hwnd, NULL, TRUE);

        FILE* file = fopen("todos.txt", "w");
        if (file != NULL) {
            for (int i = 0; i < numTodos; ++i) {
                fprintf(file, "%s|%s|%d\n", todos[i].text1, todos[i].text2, todos[i].completed);
            }
            fclose(file);
        }
    }
}

// void ApplyTheme(HWND hwnd, BOOL darkTheme) {
//     HBRUSH hBrush = CreateSolidBrush(darkTheme ? RGB(45, 45, 48) : RGB(255, 255, 255));
//     SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
//     InvalidateRect(hwnd, NULL, TRUE);  // Forces the window to be repainted
//     UpdateWindow(hwnd);  // Immediately updates the client area
// }

// void ApplyTheme(HWND hwnd, BOOL darkTheme) {
//     HBRUSH hBrush = CreateSolidBrush(darkTheme ? RGB(45, 45, 48) : RGB(255, 255, 255));
//     SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
//     InvalidateRect(hwnd, NULL, TRUE); // Forces the window to be repainted
//     UpdateWindow(hwnd); // Immediately updates the client area
// }

void ApplyTheme(HWND hwnd, BOOL darkTheme) {
    // Update the background brush for the entire window
    HBRUSH hBrush = CreateSolidBrush(darkTheme ? RGB(45, 45, 48) : RGB(255, 255, 255));
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

    // Force redraw of the window and all controls
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

    // Iterate through all child windows to set their themes
    HWND hChild = GetWindow(hwnd, GW_CHILD);
    while (hChild) {
        SendMessage(hChild, WM_THEMECHANGED, (WPARAM)darkTheme, 0);
        hChild = GetWindow(hChild, GW_HWNDNEXT);
    }
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    BOOL darkTheme = LoadDarkModeSetting();
    LoadTodos();
        if (numTodos == 0) {
    }
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Set the icon
    wc.style         = 0;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName  = NULL;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    HWND hWnd = CreateWindowEx(0, "MyWindowClass", "C-To-Do", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 600, 800, NULL, NULL, hInstance, NULL);
    if (!hWnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int totalWidth = 600; // This is your window's width
    int firstInputWidth = totalWidth * 1 / 5;
    int secondInputWidth = totalWidth * 3 / 5;
    int buttonWidth = totalWidth * 1 / 5;


   // Create an Add Button
    HWND hAddButton = CreateWindowEx(
    0,                // No extended styles 
    "BUTTON",   ""      // Predefined class; Unicode assumed 
    "Add",            // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles, including BS_OWNERDRAW
    10 + firstInputWidth + secondInputWidth, // x position 
    20,                                      // y position 
    buttonWidth - 30,                        // Button width
    30,                                      // Button height
    hWnd,                                    // Parent window
    (HMENU)IDC_ADD_BUTTON,                   // The control's ID for command messages
    (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
    NULL);         // Pointer not needed.

    // Create an input field with WS_TABSTOP
    hInputEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Extended window styles
        "EDIT",           // Predefined class
        "",               // No default text
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, // Styles including WS_TABSTOP
        10,               // x position
        20,               // y position
        firstInputWidth - 20, // width
        30,               // height
        hWnd,             // parent window
        (HMENU)IDC_INPUT_EDIT, // control ID
        GetModuleHandle(NULL),
        NULL);

    // Set a character limit for the input field
    SendMessage(hInputEdit, EM_SETLIMITTEXT, (WPARAM)10, 0); // Limit to 8 characters


    // Similarly, for the second input field
    hSecondInputEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, // Styles including WS_TABSTOP
        10 + firstInputWidth, 20, secondInputWidth - 20, 30,
        hWnd,
        (HMENU)(IDC_INPUT_EDIT + 1), // You might need to define a different ID for the second input
        GetModuleHandle(NULL),
        NULL);



    HWND hSettingsButton = CreateWindowEx(
        0, "BUTTON", "Dark Mode", 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, // Notice the BS_OWNERDRAW style
        10, 730, 80, 30,
        hWnd, (HMENU)ID_TOGGLE_BUTTON, hInstance, NULL);

    // CreateWindow("BUTTON", "Toggle Theme",
    //     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
    // //     10, 730, 80, 30, 
    //     hwnd,
    //     (HMENU)ID_TOGGLE_BUTTON,
    //     hInstance,
    //     NULL);

    // Set a character limit for the input field
    SendMessage(hSecondInputEdit, EM_SETLIMITTEXT, (WPARAM)40, 0); // Limit to 8 characters
    // Note: You do not need to cast GetWindowLongPtr(hWnd, GWLP_HINSTANCE) to (HINSTANCE) for GetModuleHandle(NULL) is appropriate here.

        ShowWindow(hWnd, nCmdShow);
        SetFocus(hInputEdit);
        UpdateWindow(hWnd);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (!IsDialogMessage(hWnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return (int)msg.wParam;
    }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int focusIndex = 1; // This variable will keep its value between calls
    HWND hFocus;
    static BOOL darkTheme = FALSE;  // This should be static to maintain state or a global variable
    static BOOL initialized = FALSE;
    static HBRUSH hbrBkgnd = NULL; // Declare brush handle to manage theme color
    static HBRUSH hbrBkgndBtn = NULL;  // Declare this at the top of WindowProc
    if (!initialized) {
        darkTheme = LoadDarkModeSetting();  // Load once when the window is created
        ApplyTheme(hwnd, darkTheme);        // Apply initial theme
        initialized = TRUE;
    }
    switch (uMsg) {
        // case WM_CTLCOLORBTN: {
        //     HDC hdcBtn = (HDC) wParam;
        //     HWND hwndBtn = (HWND) lParam;
        //     SetBkMode(hdcBtn, TRANSPARENT);
        //     if (!hbrBkgndBtn || darkTheme) {
        //         // Recreate the brush if theme has changed or not yet created
        //         if (hbrBkgndBtn) {
        //             DeleteObject(hbrBkgndBtn); // Clean up old brush
        //         }
        //         hbrBkgndBtn = CreateSolidBrush(darkTheme ? RGB(45, 45, 48) : RGB(240, 240, 240));
        //     }
        //     if (darkTheme) {
        //                 ApplyTheme(hwnd, darkTheme);  // Ensure ApplyTheme properly adjusts all UI elements


        //             return (INT_PTR)hbrBkgndBtn;
        //         } else {
        //             SetTextColor(hdcBtn, RGB(0, 0, 0));       // Black text
        //             SetBkColor(hdcBtn, RGB(240, 240, 240));   // Lighter background
        //             return (INT_PTR)hbrBkgndBtn;
        //         }
        // }
case WM_DRAWITEM:
{
    LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
    if (pDIS->CtlType == ODT_BUTTON) {
        WCHAR buttonText[10];
        GetWindowTextW(pDIS->hwndItem, buttonText, sizeof(buttonText) / sizeof(buttonText[0]));

        HBRUSH hBrush = CreateSolidBrush(darkTheme ? RGB(45, 45, 48) : RGB(240, 240, 240));
        HPEN hPen = CreatePen(PS_SOLID, 1, darkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SelectObject(pDIS->hDC, hBrush);
        SelectObject(pDIS->hDC, hPen);

        Rectangle(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right, pDIS->rcItem.bottom);

        SetTextColor(pDIS->hDC, darkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkMode(pDIS->hDC, TRANSPARENT);
        DrawTextW(pDIS->hDC, buttonText, -1, &pDIS->rcItem, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(hBrush);
        DeleteObject(hPen);
    }
    return TRUE;
}



        case WM_CTLCOLORBTN:

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC) wParam;
            HWND hwndControl = (HWND) lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (darkTheme) {
                SetTextColor(hdc, RGB(255, 255, 255)); // White text
                SetBkColor(hdc, RGB(45, 45, 48));      // Dark background
            } else {
                SetTextColor(hdc, RGB(0, 0, 0));       // Black text
                SetBkColor(hdc, RGB(255, 255, 255));   // Light background
            }
                  }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            HWND hwndEdit = (HWND)lParam;
            if (darkTheme) {
                SetTextColor(hdcEdit, RGB(0, 0, 0)); // Black text
                SetBkColor(hdcEdit,  RGB(255, 255, 255)); // White background
                if (!hbrBkgnd) {
                    hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255)); // White background
                }
                return (INT_PTR)hbrBkgnd;
            } else {
                SetTextColor(hdcEdit, RGB(0, 0, 0)); // Black text
                SetBkColor(hdcEdit, RGB(255, 255, 255)); // White background
                if (hbrBkgnd) {
                    DeleteObject(hbrBkgnd);
                    hbrBkgnd = NULL;
                }
                return (INT_PTR)GetStockObject(WHITE_BRUSH);
            }
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
        case ID_TOGGLE_BUTTON:{
            darkTheme = !darkTheme;
            SaveDarkModeSetting(darkTheme);
            HWND hChild = GetWindow(hwnd, GW_CHILD);
            while (hChild) {
                char className[256];
                GetClassName(hChild, className, sizeof(className));
                if (strcmp(className, "Button") == 0) {
                    InvalidateRect(hChild, NULL, TRUE);
                }
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            }
            case IDC_ADD_BUTTON: {
                char buffer[MAX_TODO_LENGTH];
                GetWindowText(hInputEdit, buffer, sizeof(buffer));

            // Inside the WM_COMMAND case for IDC_ADD_BUTTON:
            if (selectedTodoIndex >= 0 && selectedTodoIndex < numTodos) {
                char buffer1[MAX_TODO_LENGTH];
                char buffer2[MAX_TODO_LENGTH];
                GetWindowText(hInputEdit, buffer1, sizeof(buffer1));
                GetWindowText(hSecondInputEdit, buffer2, sizeof(buffer2));
                strncpy(todos[selectedTodoIndex].text1, buffer1, MAX_TODO_LENGTH);
                strncpy(todos[selectedTodoIndex].text2, buffer2, MAX_TODO_LENGTH);
                todos[selectedTodoIndex].text1[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination for text1
                todos[selectedTodoIndex].text2[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination for text2
            } else {
                AddTodo(hwnd);
            }


                SetWindowText(hInputEdit, ""); // Clear the input box after updating/adding
                SetWindowText(hSecondInputEdit, ""); // Clear the input box after updating/adding
                selectedTodoIndex = -1; // Reset selection
                InvalidateRect(hwnd, NULL, TRUE); // Redraw window
                break;
            }
                case IDC_DELETE_BUTTON:
                    // Logic for deleting the currently selected todo
                    if (selectedTodoIndex >= 0 && selectedTodoIndex < numTodos) {
                        DeleteTodo(hwnd, selectedTodoIndex);
                        selectedTodoIndex = -1; // Reset selection
                        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
                    }
                    break;
            }
            break; // Ensure we break after handling WM_COMMAND
        }
                case WM_KEYDOWN: {
                int totalControls = 2; // Start with two for the input fields.
            switch (wParam) {
                case VK_TAB: {
                    BOOL shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
                    if (!shiftPressed) {
                        // Forward cycle through controls
                        focusIndex = (focusIndex + 1) % totalControls;
                    } else {
                        // Reverse cycle through controls
                        focusIndex = (focusIndex - 1 + totalControls) % totalControls;
                    }

                    // Depending on focusIndex, set the focus to the respective control
                    if (focusIndex == 0) {
                        SetFocus(hInputEdit);
                    } else if (focusIndex == 1) {
                        SetFocus(hSecondInputEdit);
                    } else {
                        // Logic for focusing todo items and their elements
                        // This example doesn't provide direct implementation details
                        // for focusing on custom elements like todo items or checkboxes.
                        // You might manage this with a custom focus handling mechanism.
                    }

                    return 0; // Handled the message
                }
                case VK_RETURN: {
                    hFocus = GetFocus();
                    if (hFocus == hInputEdit) {
                        SetFocus(hSecondInputEdit);
                    } else if (hFocus == hSecondInputEdit) {
                        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_ADD_BUTTON, BN_CLICKED), 0);
                    } else {
                        // Logic for Enter on todo items, similar to above,
                        // needs specific implementation for your app structure.
                    }

                    return 0; // Handled the message
                }
                // Additional VK_UP and VK_DOWN handling can be implemented similarly
            }
            break;
        }

        case WM_PAINT: {
            // If secondInputWidth isn't globally defined, ensure it's calculated within WM_PAINT or passed appropriately.
            // Assuming a totalWidth of 600, as in WinMain:





            int secondInputWidth = 100;
            // PAINTSTRUCT ps;
            //     HDC hdc = BeginPaint(hwnd, &ps);
            //     SetBkMode(hdc, TRANSPARENT);
            //     SetTextColor(hdc, darkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0));
            //     TextOut(hdc, 10, 50, "Sample Text", 11);
            
            // FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            // HBITMAP CreateCheckboxBitmap(HDC hdc, BOOL checked);

            PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                // Setting the background mode and text color based on the theme
                SetBkMode(hdc, TRANSPARENT);
                COLORREF textColor = darkTheme ? RGB(255, 255, 255) : RGB(0, 0, 0);
                SetTextColor(hdc, textColor);



                // Set background brush depending on the theme
                HBRUSH hBrush = darkTheme ? CreateSolidBrush(RGB(30, 30, 30)) : CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &ps.rcPaint, hBrush);
                DeleteObject(hBrush); // Clean up the brush

                // Text output adjusted for theme
     

               HBITMAP CreateCheckboxBitmap(HDC hdc, BOOL checked);





            int startX = 10; // Declare and initialize startX here
            int startY = 100; // Declare and initialize startY here



            // Create the regular and strikethrough fonts
            LOGFONT lf = {0};
            lf.lfHeight = 20; // Adjust the font size as necessary
            lf.lfWeight = FW_NORMAL;
            strcpy(lf.lfFaceName, TEXT("Cascadia Code"));
            // Regular font
            lf.lfStrikeOut = FALSE;
            lf.lfUnderline = FALSE;
            HFONT hFontRegular = CreateFontIndirect(&lf);
            // Strikethrough font
            lf.lfStrikeOut = TRUE;
            HFONT hFontStrikethrough = CreateFontIndirect(&lf);
            // Regular underlined font
            lf.lfStrikeOut = FALSE;
            lf.lfUnderline = TRUE;
            HFONT hFontUnderlined = CreateFontIndirect(&lf);
            // Strikethrough underlined font
            lf.lfStrikeOut = TRUE;
            HFONT hFontStrikethroughUnderlined = CreateFontIndirect(&lf);

   for (int i = 0; i < numTodos; ++i) {
        int itemY = startY + (i * 30); // Calculate Y position for each item


        // Create and draw the checkbox bitmap for each todo item
        HBITMAP hCheckboxBitmap = CreateCheckboxBitmap(hdc, todos[i].completed);
        HDC hMemDC = CreateCompatibleDC(hdc);
        SelectObject(hMemDC, hCheckboxBitmap);
        BitBlt(hdc, startX + checkboxOffset, itemY, 20, 20, hMemDC, 0, 0, SRCCOPY);
        DeleteDC(hMemDC);
        DeleteObject(hCheckboxBitmap);

        // Select the appropriate font based on completion and selection
        HFONT selectedFont = hFontRegular; // Default to regular font
        if (i == selectedTodoIndex) {   
            selectedFont = todos[i].completed ? hFontStrikethroughUnderlined : hFontUnderlined;
        } else if (todos[i].completed) {
            selectedFont = hFontStrikethrough;
        }
        SelectObject(hdc, selectedFont);

         // Set text color based on completion status for the item text
        // COLORREF textColor = todos[i].completed ? RGB(128, 128, 128) : RGB(0, 0, 0);
        SetTextColor(hdc, todos[i].completed ? RGB(128, 128, 128) : textColor);


        // Construct and draw the todo item's text, including a timestamp if present
        // char displayText[MAX_TODO_LENGTH + 30]; // Adjust size for timestamp 
        // snprintf(displayText, sizeof(displayText), "%s %s", todos[i].timestamp, todos[i].text); // Assume timestamp exists
        // TextOut(hdc, startX + 25, itemY, displayText, strlen(displayText)); // Adjust text position for checkbox

        // Draw first text
        TextOut(hdc, startX, itemY, todos[i].text1, strlen(todos[i].text1));

        // Draw second text in a second column
        TextOut(hdc, startX + 25 + secondInputWidth, itemY, todos[i].text2, strlen(todos[i].text2));
        
        // Set font and color for the "X" button
        SelectObject(hdc, hFontRegular); // Use regular font for the "X" button
        SetTextColor(hdc, darkTheme ? RGB(255, 85, 85) : RGB(255, 0, 0)); // Red in both themes, but can be adjusted


        // Draw "X" button for deletion
        TextOutW(hdc, startX + deleteButtonOffset, itemY, L"\u2715", 1); // Using Unicode for rendering the symbol
    }

    // Clean up: Delete the fonts after use
    DeleteObject(hFontRegular);
    DeleteObject(hFontStrikethrough);
    DeleteObject(hFontUnderlined);
    DeleteObject(hFontStrikethroughUnderlined);


    EndPaint(hwnd, &ps);
    break;
}

      
    case WM_CHAR: {
    switch (wParam) {
        case 'N': // Add a new todo
            AddTodo(hwnd);
            InvalidateRect(hwnd, NULL, TRUE); // Redraw window to reflect changes
            break;
        case 'D': // Delete the first todo
            if (numTodos > 0) { // Ensure there's at least one todo to delete
                DeleteTodo(hwnd, 0);
                InvalidateRect(hwnd, NULL, TRUE); // Redraw window to reflect changes
            }
            break;
        case 'T': // Toggle completion of the first todo
            if (numTodos > 0) { // Ensure there's at least one todo to toggle
                ToggleCompletion(hwnd, 0);
                InvalidateRect(hwnd, NULL, TRUE); // Redraw window to reflect changes
            }
            break;
    }
    break;
}
           
case WM_LBUTTONDOWN: {
    int xPos = LOWORD(lParam);
    int yPos = HIWORD(lParam);
    int startX = 10; // Match with your drawing logic in WM_PAINT
    int startY = 100; // Match with your drawing logic in WM_PAINT
   //   int yPos = HIWORD(lParam) - startY;

        for (int i = 0; i < numTodos; i++) {
                int itemY = startY + (i * 30);
                // Checkbox click area
                if (xPos >= startX + checkboxOffset && xPos <= startX + checkboxOffset + 20 && yPos >= itemY && yPos <= itemY + 20) {
                    ToggleCompletion(hwnd, i);
                }
                // "X" button click area
                if (xPos >= startX + deleteButtonOffset && xPos <= startX + deleteButtonOffset + 20 && yPos >= itemY && yPos <= itemY + 20) {
                    if (MessageBox(hwnd, "Are you sure you want to delete this item?", "Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        DeleteTodo(hwnd, i);
                    }
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

        case WM_DESTROY:
           if (hbrBkgnd) {
                DeleteObject(hbrBkgnd); // Clean up the brush when destroying the window
            }
           if (hbrBkgndBtn) {
                DeleteObject(hbrBkgndBtn);
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

