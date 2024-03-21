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

typedef struct {
    char text[MAX_TODO_LENGTH];
    BOOL completed;
} TodoItem;

TodoItem todos[MAX_TODOS];
int numTodos = 3; // Initial number of todos

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND hInputEdit;

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

// void AddTodo(HWND hwnd, const char* text) {
//     if (numTodos < MAX_TODOS) {
//         strncpy(todos[numTodos].text, text, MAX_TODO_LENGTH);
//         todos[numTodos].text[MAX_TODO_LENGTH - 1] = '\0'; // Ensure null termination
//         todos[numTodos].completed = FALSE;
//         numTodos++;
//         InvalidateRect(hwnd, NULL, TRUE); // Redraw window
//     }
// }

void DeleteTodo(HWND hwnd, int index) {
    if (index >= 0 && index < numTodos) {
        for (int i = index; i < numTodos - 1; ++i) {
            strcpy(todos[i].text, todos[i + 1].text);
            todos[i].completed = todos[i + 1].completed;
        }
        numTodos--;
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
        // Add default todo items here, similar to what you have done previously
        strcpy(todos[0].text, "1. Complete assignment");
        todos[0].completed = FALSE;
        strcpy(todos[1].text, "2. Buy groceries");
        todos[1].completed = FALSE;
        strcpy(todos[2].text, "3. Call mom");
        todos[2].completed = FALSE;
        numTodos = 3;
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
                               CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);


    if (!hWnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }


   // Create an Add Button
    HWND hAddButton = CreateWindowEx(
        0, "BUTTON", "Add Todo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        500, 40, 100, 30, hWnd, (HMENU)IDC_ADD_BUTTON, GetModuleHandle(NULL), NULL);

    // Create a Delete Button
    HWND hDeleteButton = CreateWindowEx(
        0, "BUTTON", "[X]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        500, 80, 100, 30, hWnd, (HMENU)IDC_DELETE_BUTTON, GetModuleHandle(NULL), NULL);

    // Adjust the input edit field to be at the top
    hInputEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 10, 480, 20, hWnd, (HMENU)IDC_INPUT_EDIT, GetModuleHandle(NULL), NULL);




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
            SetTextColor(hdc, RGB(0, 0, 0)); // Black color
            SetBkMode(hdc, TRANSPARENT); // Transparent background for text

            // Start drawing todos below the input field and buttons
            int startY = 100; // Adjust as needed to fit your layout

            for (int i = 0; i < numTodos; ++i) {
                if (i == selectedTodoIndex) {
                    // Set a different background color for the selected item
                    SetBkMode(hdc, OPAQUE);
                    SetBkColor(hdc, RGB(220, 220, 220)); // Light gray background for selection
                }


        if (i == selectedTodoIndex) {
            SetBkColor(hdc, RGB(220, 220, 220)); // Light gray background for the selected item
        } else {
            SetBkColor(hdc, RGB(255, 255, 255)); // White background for non-selected items
        }
        char displayText[MAX_TODO_LENGTH + 4]; // Extra space for numbering
        snprintf(displayText, sizeof(displayText), "%d. %s", i + 1, todos[i].text);
        TextOut(hdc, 10, startY + (i * 20), displayText, strlen(displayText));
    }
                    
            EndPaint(hwnd, &ps);
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
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//     // Initialize initial todo list with examples
//     strcpy(todos[0].text, "1. Complete assignment");
//     todos[0].completed = FALSE;
//     strcpy(todos[1].text, "2. Buy groceries");
//     todos[1].completed = FALSE;
//     strcpy(todos[2].text, "3. Call mom");
//     todos[2].completed = FALSE;

//     // Register window class
//     WNDCLASS wc = {0};
//     wc.lpfnWndProc = WindowProc;
//     wc.hInstance = hInstance;
//     wc.lpszClassName = "MyWindowClass";
//     RegisterClass(&wc);

//     // Create the window
//     HWND hWnd = CreateWindowEx(
//         0,
//         "MyWindowClass",
//         "Windowed Todo List App",
//         WS_OVERLAPPEDWINDOW,
//         CW_USEDEFAULT, CW_USEDEFAULT,
//         640, 480,
//         NULL,
//         NULL,
//         hInstance,
//         NULL
//     );

//   // Show the window
//     ShowWindow(hWnd, nCmdShow);
//     UpdateWindow(hWnd);

//     // Message loop
//     MSG msg;
//     while (GetMessage(&msg, NULL, 0, 0) > 0) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     return (int) msg.wParam;

// }

// LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//     switch (uMsg) {
//         case WM_KEYDOWN:
//             switch (wParam) {
//                 case 'N': // Add a new todo
//                     AddTodo(hwnd, "New Todo");
//                     break;
//                 case 'D': // Delete the first todo
//                     DeleteTodo(hwnd, 0);
//                     break;
//                 case 'T': // Toggle completion of the first todo
//                     ToggleCompletion(hwnd, 0);
//                     break;
//             }
//             break;
//         // Existing case for WM_PAINT and WM_DESTROY goes here
//         case WM_PAINT: {
//             // Existing painting logic goes here
//         }
//         case WM_DESTROY:
//             PostQuitMessage(0);
//             return 0;
//         default:
//             return DefWindowProc(hwnd, uMsg, wParam, lParam);
//     }
// }

