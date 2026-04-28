#include <iostream>
#include <windows.h>
#include <vector>
#include <utility>
#include <cmath>

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
    virtual bool islegal(int dx,int dy){return true;}
    int move(int x,int y){
        if(islegal(x,y)){
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
    [[maybe_unused]] std::vector<std::pair<int,int>> possibleMoves(){
        return {{}};
    }
    void draw(char map[8][8]){
        map[y][x]='$';
    }
};

class King:public Piece{
    public:
    King(int x, int y, std::vector<Piece*>& p):Piece(x,y,p){}
    bool islegal(int dx, int dy) override{
        int X = std::abs(dx - x);
        int Y = std::abs(dy - y);
        return X <= 1 && Y <= 1 && (X + Y > 0);
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

class Player{
    public:
    Player* opponent;
    King* king;
    std::vector<Piece*>& all_pieces;
    //my ones
    std::vector<Piece*> pieces;
    Player(std::vector<Piece*>& ap,Player* o):all_pieces(ap),opponent(o){
        king=new King(0,0,all_pieces);
    }
    bool isChecked(){
        if (!opponent || !king) return false;
        if (opponent->king && opponent->king->islegal(king->x, king->y))
            return true;
        for (auto p : opponent->pieces){
            if (p && p->islegal(king->x, king->y))
                return true;
        }
        return false;
    }
    bool isMated(){
        if (!opponent || !king) return false;
        if (!isChecked()) return false;

        auto squareAttacked = [&](int tx, int ty){
            if (opponent->king && opponent->king->islegal(tx, ty))
                return true;
            for (auto p : opponent->pieces){
                if (p && p->islegal(tx, ty))
                    return true;
            }
            return false;
        };

        for (int dy = -1; dy <= 1; ++dy){
            for (int dx = -1; dx <= 1; ++dx){
                if (dx == 0 && dy == 0) continue;
                int nx = king->x + dx;
                int ny = king->y + dy;
                if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8)
                    continue;
                if (!king->islegal(nx, ny))
                    continue;
                bool occupiedBySelf = false;
                for (auto own : pieces){
                    if (own->x == nx && own->y == ny){
                        occupiedBySelf = true;
                        break;
                    }
                }
                if (occupiedBySelf)
                    continue;
                if (!squareAttacked(nx, ny))
                    return false;
            }
        }

        return true;
    }
    int DoMove(int x, int y,int dx, int dy){
        if (x==dx  && y==dy) return -2;
        Piece* p=nullptr;
        for (auto i:pieces){
            if (i->x==x && i->y==y){
                p=i;
            }
            if (i->x==dx && i->y==dy) return -2;
        }
        if (p){
            if (p->islegal(dx,dy)){
                p->move(dx,dy);
                return 0;
            }
            else
                return -1;
        }
        return 1;
    }
};

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
