#pragma once

#include "resource.h"
#include "framework.h"

#include "Global.h"
#include "winsock.h"
#include "Player.h"


int game(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow,
    DWORD address,
    SOCKET p_sock);

DWORD WINAPI ProcessClient(LPVOID arg);

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HBITMAP DrawBackground(HWND, int, int, int, int, HDC, HDC, HBITMAP);
HBITMAP DrawSkill(HWND, int, int, int, int, HDC, HDC, HBITMAP);

HBITMAP DrawCharater(HWND, Player, HDC, HDC, HBITMAP);
void Run(HWND);


Bullet_Arr arr_to_struct(Bullet* arr);
