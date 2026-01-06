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
        bool *board[2]; bool alive = 1, dead = 0, selectionGoingOn = 0, wrap = 0, anySeedChoosen = 0;
        float density = 0.30f; // Percentage alive
        float timeSetter = 0.0f, steptime = 1/(float)20; // 5 generations per second
        int boardW;
        int boardH;
        CHAR_INFO *infoScreen;
        std::vector<CHAR_INFO*> layers;
        std::vector<std::pair<COORD, int>> pos_size;
        uint32_t generation = 1, population = 0;

        // Pattern flags
        bool seed_regular   = false;
        bool seed_rpent     = false;
        bool seed_acorn     = false;
        bool seed_pulsar    = false;
        bool seed_breeder   = false;
        bool seed_gosper    = false;
        bool seed_lwss      = false;
        bool seed_mwss      = false;



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

        bool isSameBoard() { // This funciton identifies stagnation and prevents the update
            for (int i = 0; i < boardW * boardH; i++) {
                if (board[active][i] != board[(active + 1) % 2][i])
                    return false;
            }
            return true;
        }


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
                    int nx, ny;
                    for (int d = 0; d < 8; d++) {
                        // wrapping around the world
                        if(wrap){
                            nx = (x + direction[d][0] + (boardW)) % (boardW);
                            ny = (y + direction[d][1] + boardH) % boardH;
                        }
                        // Not wrapping cells die outside the border
                        else{
                            nx = x + direction[d][0];
                            if(nx < 0 || nx >= (boardW)) continue;
                            ny = y + direction[d][1]; 
                            if(ny < 0 || ny >= boardH) continue;
                        }
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

            if (seed_regular || !anySeedChoosen) regularSeed();
            if (seed_rpent) R_pentomino(boardW / 2, boardH / 2);
            if (seed_acorn) acorn(boardW / 2, boardH / 2);
            if (seed_pulsar) AddPulsar(30, 10);
            if (seed_breeder) AddBreeder(10, 10);
            if (seed_gosper) AddGosperGliderGun(40, 10);
            if (seed_lwss) AddMiddleWeightSpaceship(0, 10);
            if (seed_mwss) AddMiddleWeightSpaceship(0, 1);
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

                if(board[active] != board[(active + 1) % 2])
                    generation ++;
            }

            timeSetter += elapsedt;

        }

        // logic for selecter
        void updateSelecter(float &elapsedt){
                static float moveTimer = 0.0f;
                const float moveInterval = 0.10f; // 10 moves/sec
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
                }
            DrawSprite(selecter);
        }

        bool update(float elapsedt) override{
            // displays current state
            Drawboard();

            if(selectionGoingOn) {
                std::wstring info = L"tile x " + std::to_wstring(tile.point.x) + L" y " + std::to_wstring(tile.point.y) + L" selecter x " 
                + std::to_wstring(selecter.point.x) + L" y " + std::to_wstring(selecter.point.y);
                std::wstring controlinfo = L"Press Enter to start simulation, SPACE to select/de-select a cell";

                DrawString({0,(float) boardH - 2}, info);
                DrawString({0,(float) boardH - 1}, controlinfo); 
                updateSelecter(elapsedt);
            }
            else {
                std::wstring gen_no = L"Generation " + std::to_wstring(generation) + L" Population " + std::to_wstring(population);
                DrawString({0,(float)(boardH - 1)}, gen_no);
                if(!isSameBoard())
                    updatelife(elapsedt);
            }

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
    game.keepBorder = 0;
    game.steptime = 1/(float)15;
    return 0;
}

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int
)
{
    uint32_t W = 140, H = 40;
    int argc;
    bool seedRequested = false;

    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    GameOfLife game;
    game.refreshRate = 60;
    game.keepBorder = 0;
    game.steptime = 1/(float)15;
    
    for (int i = 1; i < argc; i++)
    {
        if (!wcscmp(argv[i], L"-fps")) game.refreshRate = _wtoi(argv[++i]);
        else if(!wcscmp(argv[i], L"-w")){if(!(_wtoi(argv[++i]) % 2)) W = _wtoi(argv[i]);} 
        else if(!wcscmp(argv[i], L"-h")) H = _wtoi(argv[++i]);
        else if(!wcscmp(argv[i], L"-borders") || !wcscmp(argv[i], L"-b")) game.keepBorder = 1;
        else if(!wcscmp(argv[i], L"-genRate") || !wcscmp(argv[i], L"-gr")) game.steptime = _wtof(argv[++i]);
        else if(!wcscmp(argv[i], L"-wrap") || !wcscmp(argv[i], L"-w")) game.wrap = 1;
        else if (!wcscmp(argv[i], L"-draw") || !wcscmp(argv[i], L"-d")) { if (!seedRequested) game.selectionGoingOn = 1; game.anySeedChoosen = 1;}
        else if (!wcscmp(argv[i], L"-seed"))
        {
            if (i + 1 >= argc) continue;

            seedRequested = true;
            game.selectionGoingOn = 0;

            ++i;
            if      (!wcscmp(argv[i], L"regular"))  game.seed_regular = true;
            else if (!wcscmp(argv[i], L"rpent"))    game.seed_rpent   = true;
            else if (!wcscmp(argv[i], L"acorn"))    game.seed_acorn   = true;
            else if (!wcscmp(argv[i], L"pulsar"))   game.seed_pulsar  = true;
            else if (!wcscmp(argv[i], L"breeder"))  game.seed_breeder = true;
            else if (!wcscmp(argv[i], L"gosper"))   game.seed_gosper  = true;
            else if (!wcscmp(argv[i], L"lwss"))     game.seed_lwss    = true;
            else if (!wcscmp(argv[i], L"mwss"))     game.seed_mwss    = true;
            game.anySeedChoosen = 1;
        }
        else if(!wcscmp(argv[i], L"-help")){
            wprintf(L"=== Game of Life - Help ===\n\n");
            wprintf(L"Usage: GameOfLife.exe [options]\n\n");
            wprintf(L"Options:\n");
            wprintf(L"  -w <number>    Set the board width (must be even, default: 140)\n");
            wprintf(L"  -h <number>    Set the board height (default: 40)\n");
            wprintf(L"  -gr <number>   Set the simulation speed in steps/sec (default: 10)\n");
            wprintf(L"  -fps <number>  Set the refresh rate\n");
            wprintf(L"  -help,         Display this help section\n\n");
            wprintf(L"  -wrap              Enable wrap-around edges (toroidal board)\n");
            wprintf(L"  -borders, -b       Keep borders visible\n\n");
            wprintf(L"Initial state / seed selection:\n");
            wprintf(L"  -draw, -d          Enable interactive seed drawing mode\n");
            wprintf(L"  -seed <type>       Choose a preset seed:\n");
            wprintf(L"                     regular  - Regular pattern\n");
            wprintf(L"                     rpent    - R-pentomino\n");
            wprintf(L"                     acorn    - Acorn pattern\n");
            wprintf(L"                     pulsar   - Pulsar oscillator\n");
            wprintf(L"                     breeder  - Breeder pattern\n");
            wprintf(L"                     gosper   - Gosper glider gun\n");
            wprintf(L"                     lwss     - Lightweight spaceship\n");
            wprintf(L"                     mwss     - Middleweight spaceship\n\n");
            wprintf(L"Controls during simulation:\n");
            wprintf(L"  W / A / S / D  Move the selector cursor\n");
            wprintf(L"  SPACE          Toggle the cell under selector\n");
            wprintf(L"  ENTER          Finish selection and start simulation\n\n");
            wprintf(L"Example:\n");
            wprintf(L"  GameOfLife.exe -w 120 -h 50 -s 15\n");
            return EXIT_SUCCESS;
        }
    }
    game.run(8,16,W,H);
    LocalFree(argv);
    return EXIT_SUCCESS;
}
