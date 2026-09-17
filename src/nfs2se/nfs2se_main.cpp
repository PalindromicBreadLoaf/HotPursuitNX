#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <nfs2se.h>
#include <cstdlib>
#include <string>

#ifndef __SWITCH__
static std::string getExeDirectory(const char* argv0)
{
    std::string path(argv0);
    for (auto& c : path)
        if (c == '\\') c = '/';
    auto pos = path.rfind('/');
    if (pos != std::string::npos)
        return path.substr(0, pos + 1);
    return "./";
}
#endif

int main(int argc, char* argv[])
{
#ifdef __SWITCH__
    (void)argc;
    setenv("NVK_I_WANT_A_BROKEN_VULKAN_DRIVER", "1", 1);
    win32::File::setDataDirectory("sdmc:/switch/nfs2se/");
    win32::File::setCdDirectory("sdmc:/switch/nfs2se/");
#else
    if (argc >= 3)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[2]);
    }
    else if (argc == 2)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[1]);
    }
    else
    {
        std::string exeDir = getExeDirectory(argv[0]);
        win32::File::setDataDirectory(exeDir.c_str());
        win32::File::setCdDirectory(exeDir.c_str());
    }
#endif
    SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK);
    {
        nfs2se::Application app(argv[0]);
        app.execute();
    }
    SDL_Quit();
    return 0;
}
