#include <iostream>
#include <windows.h>
#include <vector>

class Piece{
    public:
    int x,y;
    std::vector<Piece*>& all_pieces;
    void take(Piece* piece){
        for (auto i=all_pieces.begin();i!=all_pieces.end();i++){
            if (*i==piece){
                all_pieces.erase(i);
                break;
            }
        }
    }
    Piece(int x,int y, std::vector<Piece*>& all_pieces):x(x),y(y),all_pieces(all_pieces){}
    virtual ~Piece()=default;
    virtual bool islegal(){return true;}
    int move(int x,int y){
        if(islegal()){
            for (auto i:all_pieces){
                if (i->x==x && i->y==y){
                    take(i);
                    break;
                }
            }
            this->x = x;
            this->y = y;
            return 0;
        }
        else{
            return -1;
        }
    }
    void draw(char map[8][8]){
        map[y][x]='$';
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
    std::vector<Piece*> pieces;
    {
        pieces.push_back(new Piece{0,0,pieces});
        pieces.push_back(new Piece{7,7,pieces});
    }
    int X=0,Y=0;
    char cursor='#';
    bool dragging=false;
    Piece* selectedPiece=nullptr;
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
        if (isPressed(VK_RETURN)){
            if (!dragging)
                for (auto i:pieces){
                    if (X==i->x && Y==i->y){
                        dragging=true;
                        selectedPiece=i;
                        cursor='@';
                        break;
                    }
                }
            else{
                selectedPiece->move(X,Y);
                dragging=false;
                selectedPiece=nullptr;
                cursor='#';
            }
        }
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                map[i][j]='.';
            }
        }
        for (auto i:pieces){
            i->draw(map);
        }
        map[Y][X]=cursor;
        if (isPressed(VK_ESCAPE)){
            break;
        }
        Sleep(100);
    }
    return 0;
}
