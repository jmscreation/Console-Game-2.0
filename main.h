#ifndef __MAIN_H__
#define __MAIN_H__

#include <string>
#include <vector>
#include <chrono>
#include "windows.h"

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;

class Clock {
    timepoint start;
public:
    Clock();
    virtual ~Clock();
    double getSeconds();
    double getMilliseconds();
    inline void restart() { start = std::chrono::high_resolution_clock::now(); }
};

class Draw {
    HANDLE console;
    CONSOLE_FONT_INFO font;
    COORD fontSize;
    short width, height;
    char backChar;
public:
    Draw(const char backChar=' ');
    virtual ~Draw();

    inline COORD getScreenSize() { return (COORD){width, height}; }

    void drawText(short x, short y, const char* str);
    void drawChar(short x, short y, char c);
    void drawRemove(short x, short y);
    bool setSize(short x, short y);
    void setWindowResizeable(bool allowed);
    void drawClear();

};

class Input {
    static std::vector<int> keys;
public:
    Input();
    virtual ~Input();

    int getDirection();
    bool getSpace();
    void waitAnykey(float seconds=1.);
};


class Object {
protected:
    Draw* screen;
    short x, y;
    char myChar;
public:
    Object(short x, short y, char myChar, Draw* screen);
    virtual ~Object();

    inline short X() { return x; }
    inline short Y() { return y; }
    inline char getChar() { return myChar; }
    inline void setChar(char c) { myChar = c; screen->drawChar(x, y, myChar); }

    void updatePos(short _x, short _y);
};

class Wall : public Object {
    static std::vector<Wall*> walls;
    size_t index;
public:
    Wall(short x, short y, char myChar, Draw* screen);
    virtual ~Wall();

    static Wall* checkCollision(short x, short y);
    static inline const std::vector<Wall*>& getWalls() { return walls; }
};


class Game {
    Draw* screen;
    Input* input;

    Object* user;
    int score, unlocks;
    bool unlocked, gameOver;

    Clock timer, keyTimer;
public:
    Game(short width, short height);
    virtual ~Game();

    bool update();
    void spawnScore();
};

#endif // __MAIN_H__
