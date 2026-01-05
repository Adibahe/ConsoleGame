#include <stdlib.h>
#include <stdio.h>
#include "../includes/ConsoleGameFramework.cpp"
#include <cstdlib>

class 
GameOfLife: public Engine{

    public:

        Sprite tile;
        uint32_t cells, active = 0;
        bool *board[2]; bool alive = 1, dead = 0;
        float density = 0.25f; // Percentage alive
        float timeSetter = 0.0f, steptime = 1/(float)20; // 5 generations per second
        int boardW;
        int boardH;
        CHAR_INFO *infoScreen;
        std::vector<CHAR_INFO*> layers;
        std::vector<std::pair<COORD, int>> pos_size;
        uint32_t generation = 0, population = 0;


        ~GameOfLife(){
            delete[] board[0];
            delete[] board[1];
        }

        void AddGosperGliderGun(int startX, int startY)
        {
            const int boardW = secScreenWidth / 2;
            const int boardH = secScreenHeight;

            // Relative positions of live cells in Gosper Glider Gun
            static const int gun[][2] = {
            {24,0},
            {22,1},{24,1},
            {12,2},{13,2},{20,2},{21,2},{34,2},{35,2},
            {11,3},{15,3},{20,3},{21,3},{34,3},{35,3},
            {0,4},{1,4},{10,4},{16,4},{20,4},{21,4},
            {0,5},{1,5},{10,5},{14,5},{16,5},{17,5},{22,5},{24,5},
            {10,6},{16,6},{24,6},
            {11,7},{15,7},
            {12,8},{13,8}
            };

            const int gunSize = sizeof(gun) / sizeof(gun[0]);

            for (int i = 0; i < gunSize; i++)
            {
                int x = startX + gun[i][0];
                int y = startY + gun[i][1];

                if (x < 0 || x >= boardW || y < 0 || y >= boardH)
                    continue;

                board[active][y * boardW + x] = alive;
            }
        }

        void regularSeed(){
            for(int i = 0; i < cells; i++) {
                board[0][i] = (rand()/(float)RAND_MAX) < density; // every cell has a chance of begin alive which is equivalent to density
            }
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
            population = 0;
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
                    population += temp ? 1 : 0;
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

            infoScreen = new CHAR_INFO[10 * 2];
            layers.push_back(infoScreen);
            COORD infolocation; 
            infolocation.X = 0; infolocation.Y = 37;
            std::pair t = {infolocation, 10 * 2};
            pos_size.push_back(t);

            srand(time(0)); // for generating varying random number each time this programm

            tile = LoadSpriteFromFile("GameOfLife/tile.txt");
            std::memset(board[0], dead, cells * sizeof(bool)); // writes every cells false or dead
            std::memset(board[1], dead, cells * sizeof(bool)); // writes every cells false or dead

            // load initial state
            regularSeed();

            return true;
        }

        bool update(float elapsedt) override{
            // displays current state
            Drawboard();
            std::wstring gen_no = L"Generation " + std::to_wstring(generation) + L" Population " + std::to_wstring(population);
            DrawString({0,39}, gen_no);

            if(timeSetter >= steptime){ //constrols the flow of generation per second
                updateBoard();
                active = (active + 1) % 2; // swaping between active and inactive boards
                timeSetter -= steptime;
                generation ++;
            }

            timeSetter += elapsedt;
            return true;
        }

        bool render(){
            // Compose(layers, pos_size);
            Compose();
            writePrimaryScreenBuffer();
            return true;
        }
};

int
WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GameOfLife game;
    game.refreshRate = 60;
    game.keepBorder = 1;
    game.steptime = 1/(float)10;
    game.run(8,16,140,40);
    return EXIT_SUCCESS;
}