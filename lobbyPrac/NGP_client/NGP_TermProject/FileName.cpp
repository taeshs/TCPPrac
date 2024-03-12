
#include <iostream>
#include "NGP_TermProject.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    AllocConsole();
    freopen("CONOUT$", "wt", stdout);
    freopen("CONIN$", "r", stdin);



    std::cout << "hi hi : " << std::endl;
    game(hInstance,
        hPrevInstance,
        lpCmdLine,
         nCmdShow);
}