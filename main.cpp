#include <iostream>
#include <windows.h>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#define ENABLE_ANSII 1
//#undef _WIN32
#ifdef _WIN32
void clr(){
    std::cout<<"\033[H"<<std::flush;
    system("cls");
}
#else
void clr(){
    std::cout<<"\033[H"<<std::flush;
    system("clear");
}
#endif

//lol

class Piece{
    public:
    int x,y;
    int team=0;
    std::string sym="$";
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
    virtual std::vector<std::pair<int,int>> possibleMoves() = 0;
    void draw(std::string map[8][8]){
        map[y][x].clear();
        if (ENABLE_ANSII)
            if (team==0)
                map[y][x]+="\033[30m";
            else if (team==1)
                map[y][x]+="\033[37m";
        map[y][x]+=sym;
        if (ENABLE_ANSII)
            map[y][x]+="\033[0m";
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
    std::vector<std::pair<int,int>> possibleMoves() override {
        std::vector<std::pair<int,int>> moves;
        for(int dx=-1; dx<=1; dx++){
            for(int dy=-1; dy<=1; dy++){
                if(dx==0 && dy==0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if(nx>=0 && nx<8 && ny>=0 && ny<8){
                    bool occupiedByOwn = false;
                    for(auto p : all_pieces){
                        if(p->x == nx && p->y == ny && p->team == team){
                            occupiedByOwn = true;
                            break;
                        }
                    }
                    if(!occupiedByOwn){
                        moves.push_back({nx, ny});
                    }
                }
            }
        }
        return moves;
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
    std::vector<std::pair<int,int>> possibleMoves() override {
        std::vector<std::pair<int,int>> moves;
        // Horizontal
        for(int dir : {-1, 1}){
            int nx = x + dir;
            while(nx >= 0 && nx < 8){
                bool occupied = false;
                int occ_team = -1;
                for(auto p : all_pieces){
                    if(p->x == nx && p->y == y){
                        occupied = true;
                        occ_team = p->team;
                        break;
                    }
                }
                if(!occupied){
                    moves.push_back({nx, y});
                } else {
                    if(occ_team != team){
                        moves.push_back({nx, y});
                    }
                    break;
                }
                nx += dir;
            }
        }
        // Vertical
        for(int dir : {-1, 1}){
            int ny = y + dir;
            while(ny >= 0 && ny < 8){
                bool occupied = false;
                int occ_team = -1;
                for(auto p : all_pieces){
                    if(p->x == x && p->y == ny){
                        occupied = true;
                        occ_team = p->team;
                        break;
                    }
                }
                if(!occupied){
                    moves.push_back({x, ny});
                } else {
                    if(occ_team != team){
                        moves.push_back({x, ny});
                    }
                    break;
                }
                ny += dir;
            }
        }
        return moves;
    }
};

bool isPressed(int key){
    return GetAsyncKeyState(key) & 0x0001;
}

void setCursorVisible(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(hConsole, &info);

    info.bVisible = visible;

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
        king->team=team;

        pieces.push_back(king);

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
    bool isStalemated(){
        if(isChecked()) return false;

        auto removeFrom = [](std::vector<Piece*>& vec, Piece* target){
            auto it = std::find(vec.begin(), vec.end(), target);
            if(it != vec.end()) vec.erase(it);
        };

        auto addTo = [](std::vector<Piece*>& vec, Piece* target){
            vec.push_back(target);
        };

        for(auto p : pieces){
            auto moves = p->possibleMoves();

            for(auto [nx, ny] : moves){
                int oldX = p->x;
                int oldY = p->y;

                Piece* captured = nullptr;

                for(auto opp : opponent->pieces){
                    if(opp->x == nx && opp->y == ny){
                        captured = opp;
                        break;
                    }
                }

                if(captured){
                    removeFrom(all_pieces, captured);
                    removeFrom(opponent->pieces, captured);
                }

                p->x = nx;
                p->y = ny;

                bool stillChecked = isChecked();

                p->x = oldX;
                p->y = oldY;

                if(captured){
                    addTo(all_pieces, captured);
                    addTo(opponent->pieces, captured);
                }

                if(!stillChecked){
                    return false;
                }
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
    std::string map[8][8];
    std::vector<Piece*> pieces;
    Player p(pieces,nullptr,1);
    p.king->sym='K';
    p.king->x=6;
    Player pp(pieces,&p,0);
    pp.king->sym='K';

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
    clr();
    init();
    setCursorVisible(false);
    while (true){
        if (p.isMated()){
            clr();
            return 0;
        }
        else if (pp.isMated()){
            clr();
            return 0;
        }
        else if (p.isStalemated() || pp.isStalemated()){
            clr();
            return 0;
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
        if (isPressed('Q')){
            std::cout<<"quit";
            return 0;
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
                int code;
                if (WhiteTurn){
                    code=p.DoMove(sx,sy,X,Y);
                }
                else{
                    code=pp.DoMove(sx,sy,X,Y);
                }
                if(!code)
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
        if (ENABLE_ANSII)
            map[Y][X] = "\033[41m" + std::string(1, cursor) + "\033[0m";
        else
            map[Y][X]=std::string(1, cursor);
        std::cout<<"\033[H"<<std::flush;
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                std::cout<<map[i][j]<<" ";
            }
            std::cout<<std::endl;
        }
        if (isPressed(VK_ESCAPE)){
            break;
        }
        Sleep(10);
    }
    return 0;
}
