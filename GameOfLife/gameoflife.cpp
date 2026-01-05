#include <stdlib.h>
#include <stdio.h>
#include "../includes/ConsoleGameFramework.cpp"
#include <cstdlib>

class 
GameOfLife: public Engine{

    public:

        Sprite tile;
        Sprite selecter;
        uint32_t cells, active = 0;
        bool *board[2]; bool alive = 1, dead = 0, selectionGoingOn = 1;
        float density = 0.30f; // Percentage alive
        float timeSetter = 0.0f, steptime = 1/(float)20; // 5 generations per second
        int boardW;
        int boardH;
        CHAR_INFO *infoScreen;
        std::vector<CHAR_INFO*> layers;
        std::vector<std::pair<COORD, int>> pos_size;
        uint32_t generation = 1, population = 0;


        ~GameOfLife(){
            delete[] board[0];
            delete[] board[1];
        }

        // Supports in setting up seeds
        inline void SetCell(int x, int y)
        {
            if (x < 0 || x >= boardW || y < 0 || y >= boardH) return;
            board[active][y * boardW + x] = alive;
        }

//////////////////// SEEDS ///////////////////////////////////////////////////////
        void regularSeed(){
            for(int i = 0; i < cells; i++) {
                board[0][i] = (rand()/(float)RAND_MAX) < density; // every cell has a chance of begin alive which is equivalent to density
            }
        }

        void AddMiddleWeightSpaceship(int StartX, int StartY)
        {
            int p[][2] = {
                {1,0},{2,0},{3,0},{4,0},{5,0},
                {0,1},{5,1},
                {5,2},
                {0,3},{4,3}
            };

            for (auto &c : p)
                SetCell(StartX + c[0], StartY + c[1]);
        }

        void AddPulsar(int StartX, int StartY)
        {
            int p[][2] = {
                {2,0},{3,0},{4,0},{8,0},{9,0},{10,0},
                {0,2},{5,2},{7,2},{12,2},
                {0,3},{5,3},{7,3},{12,3},
                {0,4},{5,4},{7,4},{12,4},
                {2,5},{3,5},{4,5},{8,5},{9,5},{10,5},

                {2,7},{3,7},{4,7},{8,7},{9,7},{10,7},
                {0,8},{5,8},{7,8},{12,8},
                {0,9},{5,9},{7,9},{12,9},
                {0,10},{5,10},{7,10},{12,10},
                {2,12},{3,12},{4,12},{8,12},{9,12},{10,12}
            };

            for (auto &c : p)
                SetCell(StartX + c[0], StartY + c[1]);
        }

        void R_pentomino(int startX, int startY){
            const int R_pentomino[][2] = {
                {1,0},{2,0},
                {0,1},{1,1},
                {1,2}
            };

            const uint32_t structSize = sizeof(R_pentomino)/sizeof(R_pentomino[0]);
            for (auto &c : R_pentomino)
                SetCell(startX + c[0], startY + c[1]);

        }

        void AddGosperGliderGun(int startX, int startY)
        {

            // Relative positions of live cells in Gosper Glider Gun
            const int gun[][2] = {
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

            for (auto &c : gun)
                SetCell(startX + c[0], startY + c[1]);
        }

        void AddBreeder(int StartX, int StartY)
        {
            // Gun
            AddGosperGliderGun(StartX, StartY);

            // Reflector blocks
            int blocks[][2] = {
                {40,10},{41,10},{40,11},{41,11},
                {40,20},{41,20},{40,21},{41,21}
            };

            for (auto &b : blocks)
                SetCell(StartX + b[0], StartY + b[1]);
        }

        void acorn(int startX, int startY){
            const int acorn[][2] = {
                {1,0},
                {3,1},
                {0,2},{1,2},{4,2},{5,2},{6,2}
            };

            for (auto &c : acorn)
                SetCell(startX + c[0], startY + c[1]);
        }

/////////////////////////////// END //////////////////////////////////////////////

        bool Drawboard(){
            for(int y = 0; y < boardH; y++){
                for(int x  = 0; x < boardW; x++){
                    tile.point.x = x * 2; tile.point.y = y;

                    int index = y * (boardW) + x;
                    if(board[active][index]) {DrawSprite(tile);}
                }
            }
            tile.point.x = 0.0f; tile.point.y = 0.0f;
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
                        int nx = (x + direction[d][0] + (boardW)) % (boardW);
                        int ny = (y + direction[d][1] + boardH) % boardH;

                        // Not wrapping cells die outside the border
                        // int nx = x + direction[d][0];
                        // if(nx < 0 || nx >= (boardW)) continue;
                        // int ny = y + direction[d][1]; 
                        // if(ny < 0 || ny >= boardH) continue;

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

            srand(time(0)); // for generating varying random number each time this programm

            tile = LoadSpriteFromFile("C:/Users/DELL/Documents/repos/consoleGame/GameOfLife/tile.txt");
            selecter = LoadSpriteFromFile("C:/Users/DELL/Documents/repos/consoleGame/GameOfLife/selecter.txt");
            

            std::memset(board[0], dead, cells * sizeof(bool)); // writes every cells false or dead
            std::memset(board[1], dead, cells * sizeof(bool)); // writes every cells false or dead

            // load initial state
            // regularSeed();
            // R_pentomino(boardW/2,boardH/2);
            // acorn(boardW/2, boardH/2);
            // AddPulsar(30, 10);
            // AddBreeder(10, 10);
            // AddGosperGliderGun(40,10);
            // AddLightShapeship(0,10);
            // AddMiddleWeightSpaceship(0,1);
            return true;
        }

        void updatelife(float &elapsedt){
            if(timeSetter >= steptime){ //constrols the flow of generation per second
                updateBoard();
                active = (active + 1) % 2; // swaping between active and inactive boards
                timeSetter -= steptime;
                generation ++;
            }

            timeSetter += elapsedt;

        }

        void updateSelecter(float &elapsedt){
            DrawSprite(selecter);
        }

        bool update(float elapsedt) override{
            // displays current state
            static float moveTimer = 0.0f;
            const float moveInterval = 0.10f; // 10 moves/sec
            std::wstring gen_no = L"Generation " + std::to_wstring(generation) + L" Population " + std::to_wstring(population);
            std::wstring info = L"tile x " + std::to_wstring(tile.point.x) + L" y " + std::to_wstring(tile.point.y) + L" selecter x " 
            + std::to_wstring(selecter.point.x) + L" y " + std::to_wstring(selecter.point.y);
            DrawString({0,(float) boardH - 2}, info);
            DrawString({0,(float)(boardH - 1)}, gen_no);

            if(selectionGoingOn) {
                if(keys[KEY_ENTER].held) selectionGoingOn = 0;
                // WASD buttons to move selector
                if(keys['W'].held) selecter.point.y -= 10.0f * elapsedt;
                if(keys['S'].held) selecter.point.y += 10.0f * elapsedt;
                //move sideways
                moveTimer += elapsedt;
                if (moveTimer >= moveInterval) {
                    if (keys['A'].held) selecter.point.x -= 2;
                    if (keys['D'].held) selecter.point.x += 2;
                    moveTimer = 0.0f;
                }
                
                selecter.point.x = std::clamp(selecter.point.x, 0.0f, (float)secScreenWidth - 2);
                selecter.point.y = std::clamp(selecter.point.y, 0.0f, (float)(boardH - 1));


                //Selection of cell
                if(keys[KEY_SPACE].pressed){
                    int32_t x = (int32_t)(selecter.point.x / 2);
                    int32_t y = (int32_t)selecter.point.y;

                    if (x >= 0 && x < boardW && y >= 0 && y < boardH) {
                        board[active][y * boardW + x] ^= 1;
                    }

                    tile.point.x = x; tile.point.y = y;
                    DrawSprite(tile);
                }

                updateSelecter(elapsedt);
            }
            else {
                updatelife(elapsedt);
            }
            Drawboard();

            return true;
        }

        bool render(){
            // Compose(layers, pos_size);
            Compose();
            writePrimaryScreenBuffer();
            return true;
        }
};

int init(){
    GameOfLife game;
    game.refreshRate = 60;
    game.keepBorder = 1;
    game.steptime = 1/(float)15;
    game.run(8,16,140,40);
    return 0;
}

int
WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    init();
    return EXIT_SUCCESS;
}