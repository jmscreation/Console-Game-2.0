#include "main.h"

using namespace std;

#define GAME_SIZE_W     50
#define GAME_SIZE_H     25

#define BACK_CHAR       ' '
#define PLAYER_CHAR     '0'
#define PLAYER_SCHAR    '@'
#define BORDER_CHAR     'X'
#define DWALL_CHAR      'x'
#define POINT_CHAR      '$'

Clock::Clock(){
    restart();
}
Clock::~Clock(){}

double Clock::getSeconds() {
    return getMilliseconds() / 1000.0;
}

double Clock::getMilliseconds() {
    return double(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count()) / 1000.0;
}



Draw::Draw(const char backChar): backChar(backChar) {
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(console, &info);
    GetCurrentConsoleFont(console, FALSE, &font); // get font info

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo); // no blinking cursor

    DWORD dwMode;
    GetConsoleMode(console, &dwMode);
    dwMode |= ENABLE_EXTENDED_FLAGS;
    dwMode |= DISABLE_NEWLINE_AUTO_RETURN; // no new line at end
    dwMode &= ~ENABLE_QUICK_EDIT_MODE; // disable editing mode
    SetConsoleMode(console, dwMode);

    width = info.dwSize.X; // update console buffer size
    height = info.dwSize.Y;
    fontSize = font.dwFontSize; // update console font size
}


Draw::~Draw() {}

bool Draw::setSize(short x, short y) {

    COORD mxsize = GetLargestConsoleWindowSize(console),
          mnsize = { short(GetSystemMetrics(SM_CXMIN) / fontSize.X),
                     short(GetSystemMetrics(SM_CYMIN) / fontSize.Y) };
    x = max(min(x, short(mxsize.X-1)), short(mnsize.X+1));
    y = max(min(y, short(mxsize.Y-1)), short(mnsize.Y+1));

    COORD size = {x, y};
    SMALL_RECT win = { 0, 0, short(size.X-1), short(size.Y-1) };
    if(x < width && y < height) SetConsoleWindowInfo(console, TRUE, (const SMALL_RECT*) &win);
    SetConsoleScreenBufferSize(console, size);
    if(x >= width || y >= height) SetConsoleWindowInfo(console, TRUE, (const SMALL_RECT*) &win);

    if(GetLastError() == 0){
        width = x;
        height = y;
        return true;
    } else {
        return false;
    }
}

void Draw::drawClear() {
    DWORD out;
    FillConsoleOutputCharacter(console, BACK_CHAR, width * height, {0,0}, &out);
}

void Draw::setWindowResizeable(bool allowed) {
    HWND consoleWindow = GetConsoleWindow();
    LONG flags = GetWindowLong(consoleWindow, GWL_STYLE);
    if(allowed){
        flags |= WS_MAXIMIZEBOX;
        flags |= WS_SIZEBOX;
    } else {
         flags &= ~WS_MAXIMIZEBOX;
         flags &= ~WS_SIZEBOX;
    }
    SetWindowLong(consoleWindow, GWL_STYLE, flags);
}

void Draw::drawChar(short x, short y, char c) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, (const char*)&c, 1, {x, y}, &out);
}

void Draw::drawRemove(short x, short y) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, (const char*)&backChar, 1, {x, y}, &out);
}

void Draw::drawText(short x, short y, const char* str) {
    if(x >= width || x < 0 || y >= height || y < 0) return;
    DWORD out;
    WriteConsoleOutputCharacter(console, str, strlen(str), {x, y}, &out);
}


Input::Input() {}
Input::~Input() {}

int Input::getDirection() {
    if(GetAsyncKeyState(VK_RIGHT) & 0x8000)  return 1;
    if(GetAsyncKeyState(VK_UP) & 0x8000)     return 2;
    if(GetAsyncKeyState(VK_LEFT) & 0x8000)   return 3;
    if(GetAsyncKeyState(VK_DOWN) & 0x8000)   return 4;
    return 0;
}

bool Input::getSpace() {
    return GetAsyncKeyState(VK_SPACE) & 0x8000;
}

void Input::waitAnykey(float seconds) {
    Sleep(DWORD(1000 * seconds));
    do {
        for(int i=0x00; i < 0xFF; ++i) if(GetAsyncKeyState(i) & 0x8000) return;
    } while(1);
}



Object::Object(short x, short y, char myChar, Draw* screen): screen(screen), x(x), y(y), myChar(myChar) {
    screen->drawChar(x, y, myChar);
}
Object::~Object() {
    screen->drawRemove(x, y);
}

void Object::updatePos(short _x, short _y) {
    screen->drawRemove(x, y);
    x = _x;
    y = _y;
    screen->drawChar(x, y, myChar);
}


vector<Wall*> Wall::walls;

Wall::Wall(short x, short y, char myChar, Draw* screen) : Object(x, y, myChar, screen) {
    index = walls.size();
    walls.push_back(this);
}

Wall::~Wall() {
    walls[index] = nullptr;
}

Wall* Wall::checkCollision(short x, short y){
    for(Wall* wall : walls){
        if(wall == nullptr) continue;
        if(x == wall->x && y == wall->y) return wall;
    }
    return nullptr;
}

/// Initialize Game Instance
Game::Game(short width, short height): screen(new Draw(BACK_CHAR)), input(new Input) {
    screen->setWindowResizeable(false);
    screen->setSize(width, height);
    screen->drawClear();

    user = new Object(5,5,PLAYER_CHAR,screen); /// player object
    score = 0;
    unlocks = 0;
    unlocked = false;
    gameOver = false;

    /// border walls
    for(short i=2; i<screen->getScreenSize().Y-1; i++){
        new Wall(0, i, BORDER_CHAR, screen);
        new Wall(screen->getScreenSize().X-1, i, BORDER_CHAR, screen);
    }
    for(short i=0; i<screen->getScreenSize().X; i++){
        new Wall(i, 1, BORDER_CHAR, screen);
        new Wall(i, screen->getScreenSize().Y-1, BORDER_CHAR, screen);
    }

    spawnScore(); // spawn first score

}
Game::~Game() {
    for(Wall* wall : Wall::getWalls()) delete wall;
    delete user;
    delete input;
    delete screen;
}

void Game::spawnScore() {
    short x, y;
    Clock timeout;
    do {
        x = rand() % screen->getScreenSize().X;
        y = 1 + rand() % (screen->getScreenSize().Y - 1); // do not spawn at Y 0
        if(timeout.getSeconds() > 1) return; // cannot spawn so just return
    } while(Wall::checkCollision(x, y) || (user->X() == x && user->Y() == y));

    new Wall(x, y, POINT_CHAR, screen);
}

///Single Game Tick
bool Game::update() {

    if(timer.getSeconds() > 0.5){ // low priority interval
        timer.restart();
        string str(string("   Score: ") + to_string(score) + " Unlocks: " + to_string(unlocks) + "   ");
        screen->drawText(screen->getScreenSize().X / 2 - str.size()/2, 0, str.data() );

        if(gameOver){
            screen->drawText(screen->getScreenSize().X / 2 - 7, 1, "---------------");
            screen->drawText(screen->getScreenSize().X / 2 - 7, 2, "|| GAME OVER ||");
            screen->drawText(screen->getScreenSize().X / 2 - 7, 3, "---------------");
            input->waitAnykey(1.5);
            return false; // final update of score
        }
    }

    if(!gameOver) do { // high priority game tick
        bool stopMovement = false;
        short x = user->X(),
              y = user->Y(),
              _x = x, _y = y;

        /// Check for player use unlock token input
        if(input->getSpace() && unlocks && !unlocked) {
            unlocks--; // use an unlock
            unlocked = true; // set unlocked
            user->setChar(PLAYER_SCHAR); // set player character
        }

        /// Get player current moving direction input
        switch(input->getDirection()){
            case 1: x += 1; break; case 2: y -= 1; break;
            case 3: x -= 1; break; case 4: y += 1; break;
            default: stopMovement = true; break;
        }

        if(stopMovement){
            keyTimer.restart(); // single step motion reset
            break;
        }

        if(keyTimer.getMilliseconds() < 300 && keyTimer.getMilliseconds() > 100) break; // allow easy single step motion

        /// Collision Event + Checking
        Wall* cwall = Wall::checkCollision(x, y);
        if(cwall != nullptr){
            switch(cwall->getChar()){
                case BORDER_CHAR:{ // collision with border
                    stopMovement = true;
                    break;
                }
                case DWALL_CHAR:{ // collision with mini wall
                    if(unlocked){ // if unlocked and ready to breakthrough
                        delete cwall; // destroy mini wall
                        unlocked = false; // reset lock state
                        user->setChar(PLAYER_CHAR); // reset player character
                    } else {
                        stopMovement = true;
                    }
                    break;
                }
                case POINT_CHAR: { // collision with point
                    spawnScore(); // spawn new point
                    score += 50; // increment score by a lot
                    unlocks++; // increase the unlock
                    delete cwall; // destroy point
                    break;
                }
            }
            if(stopMovement) break;
        }

        user->updatePos(x, y); // move player
        new Wall(_x, _y, DWALL_CHAR, screen); // generate mini wall
        score += 1; // gain score for moving

        bool stuck = true;
        {
            // check all sides for walls
            Wall* sides[4] = {
                Wall::checkCollision(x-1,y),
                Wall::checkCollision(x+1,y),
                Wall::checkCollision(x,y-1),
                Wall::checkCollision(x,y+1)};
            for(int i=0; i<4; ++i){
                if(sides[i] == nullptr){ stuck = false; break;}
                char c = sides[i]->getChar();
                if(c != BORDER_CHAR && c != DWALL_CHAR){
                    stuck = false; break;
                }
            }
        }

        /// Player is stuck and has no unlocks to use - Game Over
        if(!unlocks && stuck){
            gameOver = true;
        }

    } while(0); // just use break
    return true;
}


/// Start The Game - Program Entry Point
int main() {
    srand(time(0));
    SetConsoleTitle("Game");
    { /// Game and game loop
        Game game(GAME_SIZE_W, GAME_SIZE_H); // new game instance with given map size
        Clock tick;
        while(1){
            if(tick.getMilliseconds() > 60){
                if(!game.update()) break;
                tick.restart();
            }
        }
    }
    {   /// End title screen
        Draw screen;
        Input keyboard;
        screen.drawClear();
        string message("Thank You For Playing!");
        screen.drawText(screen.getScreenSize().X / 2 - message.size() / 2, screen.getScreenSize().Y / 2, message.data());
        keyboard.waitAnykey(2);
    }
    return 0;
}
