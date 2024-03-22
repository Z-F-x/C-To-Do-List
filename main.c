// Compile with gcc main.c -o C-To-Do-List.exe -mwindows
// Reference https://learn.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <ctype.h> // Include ctype.h for isspace

#define MAX_TODO_LENGTH 100
#define MAX_TODOS 10
#define IDC_ADD_BUTTON 101
#define IDC_DELETE_BUTTON 102
#define IDC_INPUT_EDIT 103 // Define IDC_INPUT_EDIT

char newTodoText[MAX_TODO_LENGTH] = {0};
int newTodoTextLength = 0;
int selectedTodoIndex = -1; // No todo selected initially
// These are the new starting positions for the "X" button and checkbox relative to each todo item
int deleteButtonOffset = 540; // Offset for the delete "X" button from startX
int checkboxOffset = 500; // Offset for the checkbox from startX

typedef struct {
    char text[MAX_TODO_LENGTH];
    BOOL completed;
} TodoItem;

TodoItem todos[MAX_TODOS];
int numTodos = 0; // Initial number of todos
HWND hInputEdit;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LoadTodos() {
    FILE* file = fopen("todos.txt", "r");
    if (file != NULL) {
        char line[MAX_TODO_LENGTH];
        while (fgets(line, MAX_TODO_LENGTH, file) != NULL && numTodos < MAX_TODOS) {
            // Remove newline character read by fgets
            line[strcspn(line, "\n")] = 0;

            // Check if the line is not empty (ignoring whitespace only lines)
            char* trimmedLine = line;
            while (isspace((unsigned char)*trimmedLine)) trimmedLine++;

            if (*trimmedLine != '\0') { // Line is not empty
                strncpy(todos[numTodos].text, line, MAX_TODO_LENGTH);
                todos[numTodos].completed = FALSE;
                numTodos++;
            }
        }
        fclose(file);
    }
}

void AddTodo(HWND hwnd, const char* text) {
    if (numTodos < MAX_TODOS) {
        strncpy(todos[numTodos].text, text, MAX_TODO_LENGTH);
        todos[numTodos].text[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination
        todos[numTodos].completed = FALSE;
        numTodos++;
        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
        
        // Append new todo to a file
        FILE* file = fopen("todos.txt", "a"); // Open the file in append mode
        if (file != NULL) {
            fprintf(file, "%s\n", text); // Write the todo text followed by a newline
            fclose(file); // Close the file
        } else {
            MessageBox(hwnd, "Failed to open file for writing.", "Error", MB_OK);
        }
    }
}

void DeleteTodo(HWND hwnd, int index) {
    if (index >= 0 && index < numTodos) {
        // Shift all items down one position from the deleted item's index
        for (int i = index; i < numTodos - 1; ++i) {
            strcpy(todos[i].text, todos[i + 1].text);
            todos[i].completed = todos[i + 1].completed;
        }
        numTodos--;

        // Now, update the file to reflect the new state of the todos array
        FILE* file = fopen("todos.txt", "w"); // Open in write mode, which clears existing content
        if (file != NULL) {
            for (int i = 0; i < numTodos; ++i) {
                fprintf(file, "%s\n", todos[i].text);
            }
            fclose(file);
        } else {
            MessageBox(hwnd, "Failed to open file for updating.", "Error", MB_OK);
        }

        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
    }
}

void ToggleCompletion(HWND hwnd, int index) {
    if (index >= 0 && index < numTodos) {
        todos[index].completed = !todos[index].completed;
        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LoadTodos();
        if (numTodos == 0) {
    }
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyWindowClass";
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    HWND hWnd = CreateWindowEx(0, "MyWindowClass", "Windowed Todo List App", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 600, 800, NULL, NULL, hInstance, NULL);
    if (!hWnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
   // Create an Add Button
    HWND hAddButton = CreateWindowEx(
        0, "BUTTON", "Add Todo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        470, 10, 100, 30, hWnd, (HMENU)IDC_ADD_BUTTON, GetModuleHandle(NULL), NULL);

    // Create a Delete Button
    // HWND hDeleteButton = CreateWindowEx(
    //     0, "BUTTON", "[X]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
    //     500, 80, 100, 30, hWnd, (HMENU)IDC_DELETE_BUTTON, GetModuleHandle(NULL), NULL);

    // Adjust the input edit field to be at the top
    hInputEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        30, 10, 430, 30, hWnd, (HMENU)IDC_INPUT_EDIT, GetModuleHandle(NULL), NULL);
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return (int)msg.wParam;
    }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case IDC_ADD_BUTTON: {
                char buffer[MAX_TODO_LENGTH];
                GetWindowText(hInputEdit, buffer, sizeof(buffer));

                if (selectedTodoIndex >= 0 && selectedTodoIndex < numTodos) {
                    // Update existing todo item's text
                    strncpy(todos[selectedTodoIndex].text, buffer, MAX_TODO_LENGTH);
                    todos[selectedTodoIndex].text[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination
                } else {
                    // Add new todo item
                    AddTodo(hwnd, buffer);
                }

                SetWindowText(hInputEdit, ""); // Clear the input box after updating/adding
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
        case WM_KEYDOWN:
            if (wParam == VK_RETURN && newTodoTextLength > 0) { // Finalize todo
                AddTodo(hwnd, newTodoText);
                memset(newTodoText, 0, sizeof(newTodoText)); // Reset text
                newTodoTextLength = 0; // Reset length
                InvalidateRect(hwnd, NULL, TRUE); // Redraw window with updated text
            } else if (wParam == VK_UP || wParam == VK_DOWN) {
        // Navigate up or down in the todo list
        int prevSelectedIndex = selectedTodoIndex;
        if (wParam == VK_UP) selectedTodoIndex = max(0, selectedTodoIndex - 1);
        if (wParam == VK_DOWN) selectedTodoIndex = min(numTodos - 1, selectedTodoIndex + 1);

        // Load selected item's text into the input box for editing
        if (selectedTodoIndex != prevSelectedIndex && selectedTodoIndex >= 0 && selectedTodoIndex < numTodos) {
            SetWindowText(hInputEdit, todos[selectedTodoIndex].text);
        }

        InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
            switch (wParam) {
                case 'N': // Add a new todo
                    AddTodo(hwnd, "New Todo");
                    break;
                case 'D': // Delete the first todo
                    DeleteTodo(hwnd, 0);
                    break;
                case 'T': // Toggle completion of the first todo
                    ToggleCompletion(hwnd, 0);
                    break;
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            int startX = 10; // Declare and initialize startX here
            int startY = 100; // Declare and initialize startY here

            SetTextColor(hdc, RGB(0, 0, 0)); // Black color
            SetBkMode(hdc, TRANSPARENT); // Transparent background for text

            // Create the regular and strikethrough fonts
            LOGFONT lf = {0};
            lf.lfHeight = 20; // Adjust the font size as necessary
            lf.lfWeight = FW_NORMAL;
            // Regular font
            lf.lfStrikeOut = FALSE;
            HFONT hFontRegular = CreateFontIndirect(&lf);
            // Strikethrough font
            lf.lfStrikeOut = TRUE;
            HFONT hFontStrikethrough = CreateFontIndirect(&lf);

            for (int i = 0; i < numTodos; ++i) {
                int itemY = startY + (i * 30); // Calculate Y position of each item

                // Draw checkbox
                Rectangle(hdc, startX + checkboxOffset, itemY, startX + checkboxOffset + 20, itemY + 20);
                if (todos[i].completed) {
                    HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 0));
                    FillRect(hdc, &(RECT){startX + checkboxOffset + 3, itemY + 3, startX + checkboxOffset + 17, itemY + 17}, hBrush);
                    DeleteObject(hBrush);
                }
 
                // Select appropriate font
                if (todos[i].completed) {
                    SelectObject(hdc, hFontStrikethrough);
                } else {
                    SelectObject(hdc, hFontRegular);
                }

                // Draw the todo item's text
                char displayText[MAX_TODO_LENGTH];
                snprintf(displayText, sizeof(displayText), "%d. %s", i + 1, todos[i].text);
                TextOut(hdc, startX, itemY, displayText, strlen(displayText));
                
                // Revert to the regular font for subsequent text, like the "X" button
                SelectObject(hdc, hFontRegular);
                TextOut(hdc, startX + deleteButtonOffset, itemY, "X", 1);

            }
            // Clean up: Delete the fonts after use
            DeleteObject(hFontRegular);
            DeleteObject(hFontStrikethrough);
            
            EndPaint(hwnd, &ps);
            break;
        }



        //                 // Draw checkbox (toggle completion)
        //         Rectangle(hdc, startX + 420, startY + i*430, startX + 440, startY + i*30 + 20);
        //         if (todos[i].completed) {
        //             // Draw a green checkmark or filled rectangle for simplicity
        //             HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 0));
        //             FillRect(hdc, &(RECT){startX + 420, startY + i*430 + 2, startX + 440, startY + i*30 + 18}, hBrush);
        //             DeleteObject(hBrush);
        //         }
        //                 // Draw "X" button (delete item)
        //    TextOut(hdc, startX + 450, startY + i*450, "X", 1);

           
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
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
