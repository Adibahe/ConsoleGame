#include <stdlib.h>
#include <stdio.h>
#include "../includes/ConsoleGameFramework.cpp"
#include <cstdlib>

class 
Dummy: public Engine{

    public:

        Sprite tile;
        uint32_t cells, active = 0;
        bool *board[2]; bool alive = 1, dead = 0;
        float density = 0.25f; // Percentage alive
        float timeSetter = 0.0f, steptime = 1/(float)6.5; // 5 generations per second
        int boardW;
        int boardH;


        ~Dummy(){
            delete[] board[0];
            delete[] board[1];
        }

        bool Drawboard(){
            for(int y = 0; y < boardH; y++){
                for(int x  = 0; x < boardW; x++){
                    tile.point.x = x * 2; tile.point.y = y;

                    int index = y * (boardW) + x;
                    if(board[active][index]) {DrawSprite(tile);}
                }
            }
            return true;
        }

        bool updateBoard(){
            int32_t direction[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1},{0,1}, {1,-1}, {1,0}, {1,1}};
            uint32_t neighbourAlive = 0;
            bool temp;
            uint32_t inactive = (active + 1) % 2;

            for(int y = 0; y < boardH; y++){
                for(int x  = 0; x < boardW; x++){
                    neighbourAlive = 0; // resetting for every cell
                    for (int d = 0; d < 8; d++) {
                        // wrapping around the world
                        // int nx = (x + direction[d][0] + (boardW)) % (boardW);
                        // int ny = (y + direction[d][1] + boardH) % boardH;

                        // Not wrapping cells die outside the border
                        int nx = x + direction[d][0];
                        if(nx < 0 || nx >= (boardW)) continue;
                        int ny = y + direction[d][1]; 
                        if(ny < 0 || ny >= boardH) continue;

                        if(board[active][ny * (boardW) + nx]) neighbourAlive ++;
                    }

                    temp = board[active][y * (boardW) + x];
                    if( temp && (neighbourAlive < 2 || neighbourAlive > 3) ) temp = dead;
                    else {
                        if(neighbourAlive == 3) temp = alive;
                    }

                    board[inactive][y * (boardW) + x] = temp;
                }
            }            
            return true;
        }

        bool load() override{
            boardW = secScreenWidth/2;
            boardH = secScreenHeight;

            cells = (boardW) * boardH;
            board[0] = new bool[cells];
            board[1] = new bool[cells]; 

            srand(time(0)); // for generating varying random number each time this programm

            tile = LoadSpriteFromFile("GameOfLife/tile.txt");
            std::memset(board[0], dead, cells * sizeof(bool)); // writes every cells false or dead
            std::memset(board[1], dead, cells * sizeof(bool)); // writes every cells false or dead

            // // load initial state
            for(int i = 0; i < cells; i++) {
                board[0][i] = (rand()/(float)RAND_MAX) < density; // every cell has a chance of begin alive which is equivalent to density
            }

            return true;
        }

        bool update(float elapsedt) override{
            // displays current state
            Drawboard();

            if(timeSetter >= steptime){ //constrols the flow of generation per second
                updateBoard();
                active = (active + 1) % 2; // swaping between active and inactive boards
                timeSetter -= steptime;
            }

            timeSetter += elapsedt;
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
    game.keepBorder = 1;
    game.run(8,16,140,40);
    return EXIT_SUCCESS;
}