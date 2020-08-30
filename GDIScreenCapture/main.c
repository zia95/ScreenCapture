#include<stdio.h>
#include<stdbool.h>
#include<time.h>
#include<Windows.h>

bool save_bitmap(LPCSTR lpFilename, HDC hdc, HBITMAP hBitmap)
{
	FILE* fp = NULL;
	LPVOID pBuf = NULL;
	BITMAPINFO bmpInfo;
	BITMAPFILEHEADER bmpFileHeader;


	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	ZeroMemory(&bmpFileHeader, sizeof(BITMAPFILEHEADER));


	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hdc, hBitmap, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);

	if (bmpInfo.bmiHeader.biSizeImage <= 0)
		bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth * abs(bmpInfo.bmiHeader.biHeight) * (bmpInfo.bmiHeader.biBitCount + 7) / 8;
	
	// alloc size for our bmp file.
	pBuf = malloc(bmpInfo.bmiHeader.biSizeImage);

	if (pBuf)
	{
		//read bmp data
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		GetDIBits(hdc, hBitmap, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);

		//write data to bmp file
		fp = fopen(lpFilename, "wb");

		if (fp)
		{
			//write bytes...
			bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpInfo.bmiHeader.biSizeImage;
			bmpFileHeader.bfType = 'MB';
			bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

			fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
			fwrite(&bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, fp);
			fwrite(pBuf, bmpInfo.bmiHeader.biSizeImage, 1, fp);


			// cleanup.....

			free(pBuf);
			fclose(fp);

			return true;
		}

		perror("Unable to Create Bitmap File");
		free(pBuf);
		return false;
	}

	perror("Unable to Allocate Bitmap Memory");
	return false;
}

bool take_screenshot(LPCSTR lpFileName, HWND hWnd, int	nWidth, int	nHeight)
{
	bool result = false;
	HDC hWndDC = GetDC(hWnd);

	HDC	hBmpFileDC = CreateCompatibleDC(hWndDC);
	HBITMAP	hBmpFileBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBmpFileDC, hBmpFileBitmap);
	result = BitBlt(hBmpFileDC, 0, 0, nWidth, nHeight, hWndDC, 0, 0, SRCCOPY | CAPTUREBLT);
	SelectObject(hBmpFileDC, hOldBitmap);
	
	result = lpFileName ? save_bitmap(lpFileName, hWndDC, hBmpFileBitmap) : true;
	
	DeleteDC(hWndDC);
	DeleteDC(hBmpFileDC);
	ReleaseDC(hWnd, hWndDC);
	DeleteObject(hBmpFileBitmap);

	return result;
}
bool take_screenshot_desktop(LPCSTR lpFileName)
{
	HWND hDesktopWnd = GetDesktopWindow();

	int	nWidth = GetSystemMetrics(SM_CXSCREEN);
	int	nHeight = GetSystemMetrics(SM_CYSCREEN);

	return take_screenshot(lpFileName, hDesktopWnd, nWidth, nHeight);
}
bool take_screenshot_window_rect_by_name(LPCSTR lpFileName, LPCSTR lpWindowName)
{
	HWND hWnd = FindWindowA(NULL, lpWindowName);

	if (hWnd != NULL)
	{
		RECT rect = {0};
		GetWindowRect(hWnd, &rect);
		
		return take_screenshot(lpFileName, hWnd, rect.right - rect.left, rect.bottom - rect.top);
	}
	return false;
}

void run_screenshot_test()
{
	clock_t current_ticks, delta_ticks, max_fps = 0;

	

	while (true)
	{
		current_ticks = clock();

		take_screenshot_desktop(NULL);
		//take_screenshot_desktop("D:\\test.bmp");

		delta_ticks = clock() - current_ticks;
		delta_ticks = (delta_ticks > 0) ? CLOCKS_PER_SEC / delta_ticks : delta_ticks;//calc fps

		max_fps = delta_ticks > max_fps ? delta_ticks : max_fps;

		printf("MAX_FPS: %d || FPS: %d\n", max_fps, delta_ticks);
	}
}
int main(void)
{
	run_screenshot_test();

	if (take_screenshot_window_rect_by_name("D:\\test.bmp", "<WINDOW NAME>"))
	{
		printf("bmp screenshot is saved....");
		return 0;
	}
	return 1;
}