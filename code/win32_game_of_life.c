/*
	NOTE(Douglas): The project structure is very simple: one translation unit only.
	Once one's include "simulation_platform.h" and "simulation.c" in the platform
	layer specific code, everything is good to go.
*/

#include "simulation_platform.h"
#include "simulation.c"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h> /* timeBeginPeriod() */
#include <strsafe.h> /* StringCbPrintfA | NOTE: Replace this with stb_sprintf.h or with something better */

typedef struct
{
	s32 width;
	s32 height;
	s32 bytes_per_pixel;
	s32 line_stride;
	HDC bitmap_dc;
	HBITMAP bitmap_handle;
	void *pixels;
} win32_backbuffer;



global_variable b32 global_running;
global_variable WINDOWPLACEMENT global_window_placement;
global_variable win32_backbuffer global_backbuffer;



internal void
win32_resize_backbuffer(s32 new_width,
                        s32 new_height)
{
	BITMAPINFO bitmap_info;

	if(!global_backbuffer.bitmap_dc)
	{
		global_backbuffer.bitmap_dc = CreateCompatibleDC(0);
	}

	global_backbuffer.width = new_width;
	global_backbuffer.height = new_height;
	global_backbuffer.bytes_per_pixel = 4; /* ARGB */
	global_backbuffer.line_stride = global_backbuffer.width * global_backbuffer.bytes_per_pixel;

	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = new_width;

	bitmap_info.bmiHeader.biHeight = -new_height;
	/*
	 * NOTE: "biHeight" with a positive number are bottom-top drawing. For a top-bottom
	 * drawing, use the NEGATIVE value of the height.
	 */

	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	
	global_backbuffer.bitmap_handle = CreateDIBSection(global_backbuffer.bitmap_dc,
	                                                   &bitmap_info,
	                                                   DIB_RGB_COLORS,
	                                                   &global_backbuffer.pixels,
	                                                   0, 0);

	assert(global_backbuffer.bitmap_handle);
}

internal LRESULT 
win32_window_messages_callback(HWND window,
                               UINT msg,
                               WPARAM wparam,
                               LPARAM lparam)
{
	LRESULT result = 0;

	switch(msg)
	{
		case WM_SETCURSOR:
		{
			SetCursor(0);
		} break;

		case WM_CLOSE:
		{
			global_running = FALSE;
		} break;

		case WM_DESTROY:
		{
			/* TODO: Diagnostic (main window destroyed) */
			/* TODO: Check if it was intended */

			PostQuitMessage(0);
		} break;

		default:
		{
			result = DefWindowProcA(window, msg, wparam, lparam);
		}
	}

	return result;
}

int WINAPI
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR cmd_line,
        int cmd_show)
{
	WNDCLASSEXA window_class = {0};

	window_class.cbSize = sizeof(window_class);
	window_class.style = (CS_VREDRAW | CS_HREDRAW);
	window_class.lpfnWndProc = win32_window_messages_callback;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursorA(instance, IDC_ARROW);
	window_class.lpszClassName = "Main Window Class";

	if(RegisterClassExA(&window_class))
	{
		/* monitor related variables */
		s32 monitor_vertical_resolution;
		s32 monitor_horizontal_resolution;
		s32 monitor_vertical_refresh_rate;

		/* window related variables */
		HWND window;
		char window_title[512];
		HDC window_dc;
		RECT window_dimensions_with_styles = {0};
		DWORD window_style;
		HMENU window_menu;
		#define DEFAULT_WINDOW_WIDTH (1280)
		#define DEFAULT_WINDOW_HEIGHT (720)
		s32 window_horizontal_resolution_with_styles;
		s32 window_vertical_resolution_with_styles;

		/* adjusting window initial dimensions */
		window_style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		window_menu = 0;
		window_dimensions_with_styles.right = DEFAULT_WINDOW_WIDTH;
		window_dimensions_with_styles.bottom = DEFAULT_WINDOW_HEIGHT;
		AdjustWindowRect(&window_dimensions_with_styles, window_style, (window_menu) ? (TRUE) : (FALSE));
		window_horizontal_resolution_with_styles = abs(window_dimensions_with_styles.left) + abs(window_dimensions_with_styles.right);
		window_vertical_resolution_with_styles = abs(window_dimensions_with_styles.top) + abs(window_dimensions_with_styles.bottom);

		window = CreateWindowExA(0, window_class.lpszClassName,
		                         "Main Window",
		                         window_style,
		                         0, 0,
		                         window_horizontal_resolution_with_styles,
		                         window_vertical_resolution_with_styles,
		                         0, 0, instance, 0);

		if(window)
		{
			/* window message loop related variables */
			MSG msg;

			/* timing related variables */
			#define WINDOW_TARGET_TIMER_RESOLUTION_IN_MS (1)
			TIMECAPS device_time_capabilities;
			UINT window_timer_resolution;
			LARGE_INTEGER performance_frequency;
			LARGE_INTEGER start_counter;
			LARGE_INTEGER end_counter;
			b32 limit_fps;
			f32 ms_elapsed;
			f32 ms_per_frame, desired_ms_per_frame;
			f32 frames_per_second;

			/* simulation pixel buffer for platform backbuffer */
			pixel_buffer simulation_backbuffer = {0};

			/* simulation state related variables */
			simulation_state state = {0};
			size_t simulation_grid_memory_size;
			

			/* removing resizable window capabilities */
			SetWindowLongA(window, GWL_STYLE, (GetWindowLong(window, GWL_STYLE) & ~WS_SIZEBOX) & ~WS_MAXIMIZEBOX);

			/* adjusting window initial position (centered) on monitor */
			window_dc = GetDC(window);
			monitor_vertical_resolution = GetDeviceCaps(window_dc, VERTRES);
			monitor_horizontal_resolution = GetDeviceCaps(window_dc, HORZRES);

			SetWindowPos(window, 0,
			             (monitor_horizontal_resolution - window_horizontal_resolution_with_styles) / 2,
			             (monitor_vertical_resolution - window_vertical_resolution_with_styles) / 2,
			             0, 0, (SWP_NOSIZE | SWP_NOZORDER));

			/* backbuffer setup */
			win32_resize_backbuffer(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

			/* simulation state setup */
			simulation_grid_memory_size = global_backbuffer.width * global_backbuffer.height * global_backbuffer.bytes_per_pixel;
			state.grid_output = VirtualAlloc(0, simulation_grid_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			state.cells_state = VirtualAlloc(0, simulation_grid_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			/* setting window timer resolution for events */
			if(timeGetDevCaps(&device_time_capabilities, sizeof(device_time_capabilities)) != TIMERR_NOERROR)
			{
				/* TODO: Diagnostic (device doesn't have time resolution capabilities; can't set) */
			}
			else
			{
				window_timer_resolution = min(max(device_time_capabilities.wPeriodMin, WINDOW_TARGET_TIMER_RESOLUTION_IN_MS), device_time_capabilities.wPeriodMax);
				timeBeginPeriod(window_timer_resolution);
			}

			/* timing & sleep setup */
			#define DESIRED_MONITOR_VERTICAL_REFRESH_RATE (30)
			monitor_vertical_refresh_rate = GetDeviceCaps(window_dc, VREFRESH);

			if(monitor_vertical_refresh_rate <= 1)
			{
				monitor_vertical_refresh_rate = DESIRED_MONITOR_VERTICAL_REFRESH_RATE;
			}

			desired_ms_per_frame = (1.0f / (f32)monitor_vertical_refresh_rate) * 1000;
			limit_fps = TRUE;

			QueryPerformanceFrequency(&performance_frequency);
			QueryPerformanceCounter(&start_counter);

			global_running = TRUE;
			while(global_running)
			{
				/*
				 * Window messages
				*/

				while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				/*
				 * Update
				*/

				simulation_backbuffer.width = global_backbuffer.width;
				simulation_backbuffer.height = global_backbuffer.height;
				simulation_backbuffer.bytes_per_pixel = global_backbuffer.bytes_per_pixel;
				simulation_backbuffer.line_stride = global_backbuffer.line_stride;
				simulation_backbuffer.memory = global_backbuffer.pixels;
				simulation_update_and_render(&simulation_backbuffer, &state);

				/*
				 * Draw
				*/

				SelectObject(global_backbuffer.bitmap_dc, global_backbuffer.bitmap_handle);
				BitBlt(window_dc,
				       0, 0,
				       global_backbuffer.width, global_backbuffer.height,
				       global_backbuffer.bitmap_dc,
				       0, 0, SRCCOPY);

				/*
				 * Timing
				*/

				/* Check sleep need */
				if(limit_fps)
				{
					QueryPerformanceCounter(&end_counter);
					ms_elapsed = (f32) (end_counter.QuadPart - start_counter.QuadPart) / (f32) performance_frequency.QuadPart;
					ms_elapsed *= 1000;
					
					if(ms_elapsed < desired_ms_per_frame)
					{
						DWORD ms_to_sleep = (DWORD) ((desired_ms_per_frame - ms_elapsed) - (0.001f * desired_ms_per_frame)); /* tempo que falta - ajuste de 0.01% do tempo desejado */
						
						Sleep(ms_to_sleep);

						QueryPerformanceCounter(&end_counter);
						ms_elapsed = (f32) (end_counter.QuadPart - start_counter.QuadPart) / (f32) performance_frequency.QuadPart;
						ms_elapsed *= 1000;

						while(ms_elapsed < desired_ms_per_frame)
						{
							QueryPerformanceCounter(&end_counter);
							ms_elapsed = (f32) (end_counter.QuadPart - start_counter.QuadPart) / (f32) performance_frequency.QuadPart;
							ms_elapsed *= 1000;
						}
					}
				}

				/* Update window title with frame details & update start_counter */
				QueryPerformanceCounter(&end_counter);

				/* using counters as seeds, every frame */
				set_random_number_seed((u32) end_counter.QuadPart);

				ms_per_frame = (f32) (end_counter.QuadPart - start_counter.QuadPart) / (f32) performance_frequency.QuadPart;
				ms_per_frame *= 1000;
				frames_per_second = (f32) performance_frequency.QuadPart / (f32) (end_counter.QuadPart - start_counter.QuadPart);

				StringCbPrintfA(window_title, sizeof(window_title), "Main Window | ms: %.5f | fps: %.2f", ms_per_frame, frames_per_second);
				SetWindowText(window, window_title);

				start_counter = end_counter;
			}
		}
		else
		{
			/* TODO: Diagnostic (failed when creating the main window) */
		}
	}
	else
	{
		/* TODO: Diagnostic (failed when registering the main window class) */
	}

	return 0;
}
