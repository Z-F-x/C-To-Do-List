// Compile with gcc main.c bitmap.c -o C-To-Do-List.exe -mwindows

#include "../include/bitmap.h"  // Include the header to match declaration and definition

HBITMAP CreateCheckboxBitmap(HDC hdc, BOOL checked) {
    // Create a compatible DC for drawing the bitmap
    HDC hMemDC = CreateCompatibleDC(hdc);
    
    // Create the bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 20, 20); // Assuming a 20x20 checkbox
    SelectObject(hMemDC, hBitmap);

    // Draw the outer rectangle (checkbox border)
    Rectangle(hMemDC, 0, 0, 20, 20);

if (checked) {
    // If checked, draw a checkmark that visually overflows the checkbox borders
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(34, 177, 76)); // Use a green pen for the checkmark
    SelectObject(hMemDC, hPen);

    // Adjust coordinates to overflow the 20x20 checkbox borders
    // The starting and ending points are chosen to ensure the lines extend beyond the checkbox
    MoveToEx(hMemDC, 0, 12, NULL); // Start inside the left boundary but close to it
    LineTo(hMemDC, 7, 17); // Draw towards the bottom-right corner, touching the edge or slightly beyond
    MoveToEx(hMemDC, 7, 17, NULL);
    LineTo(hMemDC, 20, 0); // Extend to the right and above, well outside the 20x20 boundary

    DeleteObject(hPen);
}


    // Cleanup
    SelectObject(hMemDC, NULL); // Always good practice to deselect objects
    DeleteDC(hMemDC);

    return hBitmap;
}
