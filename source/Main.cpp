#include <irrlicht.h>

#include "CompileConfig.h"
#include "Core.h"
#include "Game.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    engine::CCore * Core = new engine::CCore();

    Core->loadCommandLineParameters(argc, argv);

    game::CGame * Game = new game::CGame(Core);

    Game->init();
    Game->run();
    Game->close();

    // Free memory
    delete Game;

    return 0;
}
