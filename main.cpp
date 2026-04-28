#include <iostream>
#include <windows.h>

class Piece{
    int x,y;
    virtual bool islegal(){return true;}
    int move(int x,int y){
        if(islegal()){
            this->x = x;
            this->y = y;
            return 0;
        }
        else{
            return -1;
        }
    }
    char getname(){
        return '0';
    }
};

bool isPressed(int key){
    return GetAsyncKeyState(key) & 0x8000;
}

void setCursorVisible(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(hConsole, &info);

    info.bVisible = visible;   // true = show, false = hide

    SetConsoleCursorInfo(hConsole, &info);
}

void init(){
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

int main(){
    char map[8][8];
    int X=0,Y=0;
    init();
    setCursorVisible(false);
    while (true){
        std::cout<<"\033[H"<<std::flush;
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                std::cout<<map[i][j]<<" ";
            }
            std::cout<<std::endl;
        }
        if (isPressed(VK_UP) && Y>0){
            Y--;
        }
        if (isPressed(VK_DOWN) && Y<7){
            Y++;
        }
        if (isPressed(VK_LEFT) && X>0){
            X--;
        }
        if (isPressed(VK_RIGHT) && X<7){
            X++;
        }
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                map[i][j]='.';
            }
        }
        map[Y][X]='#';
        if (isPressed(VK_ESCAPE)){
            break;
        }
        Sleep(100);
    }
    return 0;
}
