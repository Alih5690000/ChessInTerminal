#include <iostream>
#include <windows.h>
#include <vector>
#include <utility>
#include <cmath>

//lol

class Piece{
    public:
    int x,y;
    int team=0;
    char sym='$';
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
        map[y][x]=sym;
    }
};

class King:public Piece{
    public:
    King(int x, int y, std::vector<Piece*>& p):Piece(x,y,p){
        sym='K';
    }
    bool islegal(int dx, int dy) override{
        int X = std::abs(dx - x);
        int Y = std::abs(dy - y);
        return X <= 1 && Y <= 1 && (X + Y > 0);
    }
};

class Rook: public Piece{
    public:
    Rook(int x, int y, std::vector<Piece*>& p):Piece(x,y,p){
        sym='R';
    }
    bool islegal(int dx, int dy) override{
        if (dx == x && dy == y) return false;
        if (dx != x && dy != y) return false;

        int stepX = (dx > x) ? 1 : (dx < x) ? -1 : 0;
        int stepY = (dy > y) ? 1 : (dy < y) ? -1 : 0;
        int cx = x + stepX;
        int cy = y + stepY;

        while (cx != dx || cy != dy) {
            for (auto p : all_pieces) {
                if (p != this && p->x == cx && p->y == cy)
                    return false;
            }
            cx += stepX;
            cy += stepY;
        }

        return true;
    }
};

bool isPressed(int key){
    return GetAsyncKeyState(key) & 0x0001;
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
    Player(std::vector<Piece*>& ap,Player* o,int team):all_pieces(ap),opponent(o){
        king=new King(0,0,all_pieces);

        pieces.push_back([team,&ap,this](){
            king->team=team;
            return king;
        }());

        for (auto i:pieces){
            all_pieces.push_back(i);
        }
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
                int oldX=p->x;
                int oldY=p->y;
                p->move(dx,dy);
                if (isChecked()){
                    p->y=oldY;
                    p->x=oldX;
                    return -1;
                }
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
    Player p(pieces,nullptr,1);
    p.king->sym='W';
    p.king->x=6;
    Player pp(pieces,&p,0);
    pp.king->sym='B';

    Rook* r=new Rook(5,5,pieces);
    r->team=0;
    pp.pieces.push_back(r);
    pieces.push_back(r);
    p.opponent=&pp;
    
    int X=0,Y=0;
    char cursor='#';
    bool dragging=false;
    bool WhiteTurn=true;
    int sx,sy;
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
                        sx=X;
                        sy=Y;
                        cursor='@';
                        break;
                    }
                }
            else{
                if (WhiteTurn){
                    p.DoMove(sx,sy,X,Y);
                }
                else{
                    pp.DoMove(sx,sy,X,Y);
                }
                WhiteTurn=!WhiteTurn;
                dragging=false;
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
    }
    return 0;
}
