#include <stdlib.h>
#include <stdio.h>
#include "../includes/ConsoleGameFramework.cpp"

class 
Dummy: public Engine{

    Sprite tile;

    bool load() override{
        tile = LoadSpriteFromFile("GameOfLife/tile.txt");
        return true;
    }

    bool update(float elapsedt) override{
        DrawSprite(tile);
        return true;
    }

    bool render(){
        Compose();
        writePrimaryScreenBuffer();
        return true;
    }
};

int 
WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

    Dummy game;
    game.refreshRate = 60;
    game.run(8,16);
    return EXIT_SUCCESS;
}