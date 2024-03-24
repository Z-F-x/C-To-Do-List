// Create a bitmap for the "X" button
HBITMAP hBitmapX = CreateCompatibleBitmap(hdc, 20, 20); // Adjust size as needed
HDC hDCBitmap = CreateCompatibleDC(hdc);
SelectObject(hDCBitmap, hBitmapX);

// Fill the bitmap with the background color
HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
RECT rect = {0, 0, 20, 20}; // Adjust size as needed
FillRect(hDCBitmap, &rect, hBrush);
DeleteObject(hBrush);

// Draw the "X" shape on the bitmap
HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // Adjust color and thickness as needed
SelectObject(hDCBitmap, hPen);
MoveToEx(hDCBitmap, 0, 0, NULL);
LineTo(hDCBitmap, 20, 20);
MoveToEx(hDCBitmap, 0, 20, NULL);
LineTo(hDCBitmap, 20, 0);
DeleteObject(hPen);

// Use BitBlt to copy the bitmap onto the window
BitBlt(hdc, startX + deleteButtonOffset, itemY, 20, 20, hDCBitmap, 0, 0, SRCCOPY);

// Clean up
DeleteDC(hDCBitmap);
DeleteObject(hBitmapX);
