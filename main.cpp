#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>
#include <algorithm>

using Board = std::array<std::array<int,4>,4>;

std::random_device rd;
std::mt19937 gen(rd());

int bestScore = 0;

// ================= SCORE =================
void loadBestScore() {
    std::ifstream file("bestscore.txt");
    if (file) file >> bestScore;
}

void saveBestScore() {
    std::ofstream file("bestscore.txt");
    file << bestScore;
}

// ================= TILE SPAWN =================
void spawnTile(Board& board) {
    std::vector<std::pair<int,int>> empty;

    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            if (board[i][j]==0)
                empty.push_back({i,j});

    if (empty.empty()) return;

    std::uniform_int_distribution<> dist(0, empty.size()-1);
    auto pos = empty[dist(gen)];

    std::uniform_int_distribution<> val(0,9);
    board[pos.first][pos.second] = (val(gen)==0)?4:2;
}

// ================= MOVE LEFT =================
bool moveLeft(Board& board,int& score) {
    bool moved=false;

    for(int i=0;i<4;i++) {
        for(int j=1;j<4;j++) {
            if(board[i][j]==0) continue;

            int col=j;

            while(col>0 && board[i][col-1]==0) {
                board[i][col-1]=board[i][col];
                board[i][col]=0;
                col--;
                moved=true;
            }

            if(col>0 && board[i][col-1]==board[i][col]) {
                board[i][col-1]*=2;
                score+=board[i][col-1];
                board[i][col]=0;
                moved=true;
            }
        }
    }
    return moved;
}

// ================= ROTATE =================
void rotate(Board& board) {
    Board temp=board;

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            board[i][j]=temp[3-j][i];
}

// ================= MOVE =================
bool move(Board& board,int dir,int& score) {
    for(int i=0;i<dir;i++)
        rotate(board);

    bool moved = moveLeft(board,score);

    for(int i=0;i<(4-dir)%4;i++)
        rotate(board);

    return moved;
}

// ================= GAME OVER =================
bool canMove(Board board) {
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(board[i][j]==0)
                return true;

    for(int i=0;i<4;i++)
        for(int j=0;j<3;j++)
            if(board[i][j]==board[i][j+1])
                return true;

    for(int j=0;j<4;j++)
        for(int i=0;i<3;i++)
            if(board[i][j]==board[i+1][j])
                return true;

    return false;
}

// ================= COLORS =================
sf::Color tileColor(int v) {
    switch(v) {
        case 0: return sf::Color(205,193,180);
        case 2: return sf::Color(238,228,218);
        case 4: return sf::Color(237,224,200);
        case 8: return sf::Color(242,177,121);
        case 16: return sf::Color(245,149,99);
        case 32: return sf::Color(246,124,95);
        case 64: return sf::Color(246,94,59);
        case 128: return sf::Color(237,207,114);
        case 256: return sf::Color(237,204,97);
        case 512: return sf::Color(237,200,80);
        case 1024: return sf::Color(237,197,63);
        case 2048: return sf::Color(237,194,46);
    }
    return sf::Color(60,58,50);
}

sf::Color textColor(int v) {
    return (v<=4)?sf::Color(119,110,101):sf::Color::White;
}

// ================= RESET =================
void resetGame(Board& board,int& score) {
    board={0};
    score=0;
    spawnTile(board);
    spawnTile(board);
}

// ================= MAIN =================
int main() {

    // ⭐ FULLSCREEN READY (YOU CAN CHANGE BACK IF YOU WANT)
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(),
                            "2048",
                            sf::Style::Default);
    window.setFramerateLimit(60);

    sf::Font font;
    if(!font.openFromFile("assets/fonts/fonts/arial.ttf")) {
        std::cout<<"Font loading failed\n";
        return -1;
    }

    loadBestScore();

    Board board={0};
    int score=0;

    bool startScreen=true;
    bool gameOver=false;

    resetGame(board,score);

    const float CELL = 120.f;
    const float GAP = 10.f;

    while(window.isOpen()) {

        while(auto event=window.pollEvent()) {

            if(event->is<sf::Event::Closed>())
                window.close();

            if(event->is<sf::Event::KeyPressed>()) {

                auto key=event->getIf<sf::Event::KeyPressed>();

                if(startScreen) {
                    startScreen=false;
                    continue;
                }

                if(key->code==sf::Keyboard::Key::R) {
                    resetGame(board,score);
                    gameOver=false;
                }

                if(gameOver) continue;

                bool moved=false;

                if(key->code==sf::Keyboard::Key::Left)
                    moved=move(board,0,score);

                if(key->code==sf::Keyboard::Key::Up)
                    moved=move(board,3,score);

                if(key->code==sf::Keyboard::Key::Right)
                    moved=move(board,2,score);

                if(key->code==sf::Keyboard::Key::Down)
                    moved=move(board,1,score);

                if(moved) {
                    spawnTile(board);

                    if(score>bestScore) {
                        bestScore=score;
                        saveBestScore();
                    }

                    if(!canMove(board))
                        gameOver=true;
                }
            }
        }

        window.clear(sf::Color(250,248,239));

        float w = window.getSize().x;
        float h = window.getSize().y;

        // ================= START SCREEN =================
        if(startScreen) {
            sf::Text start(font,"2048\nPress Any Key",50);
            start.setFillColor(sf::Color(50,50,50));

            sf::FloatRect b=start.getLocalBounds();

            start.setPosition({
                (w - b.size.x)/2.f,
                (h - b.size.y)/2.f
            });

            window.draw(start);
            window.display();
            continue;
        }

        // ======================================================
        // ⭐ FIXED RESPONSIVE CENTER SYSTEM (FINAL VERSION)
        // ======================================================

        float scale = std::min(w / 900.f, h / 800.f);

        float CELL_S = CELL * scale;
        float GAP_S  = GAP * scale;

        float boardSize = 4 * CELL_S + 3 * GAP_S;

        float UI_H = 90.f * scale;
        float GAP_UI = 20.f * scale;

        float totalH = UI_H + GAP_UI + boardSize;

        float originX = (w - boardSize) / 2.f;
        float originY = (h - totalH) / 2.f + UI_H;

        float uiY = (h - totalH) / 2.f;

        // ================= SCORE =================
        sf::Text scoreText(font,"Score: "+std::to_string(score),30);
        scoreText.setFillColor(sf::Color(50,50,50));
        scoreText.setPosition({originX, uiY});
        window.draw(scoreText);

        sf::Text bestText(font,"Best: "+std::to_string(bestScore),30);
        bestText.setFillColor(sf::Color(50,50,50));
        bestText.setPosition({originX + 220, uiY});
        window.draw(bestText);

        // ================= BOARD =================
        sf::RectangleShape boardBg({boardSize+20, boardSize+20});
        boardBg.setFillColor(sf::Color(187,173,160));
        boardBg.setPosition({originX-10, originY-10});
        window.draw(boardBg);

        // ================= TILES =================
        for(int i=0;i<4;i++) {
            for(int j=0;j<4;j++) {

                float x = originX + j*(CELL_S + GAP_S);
                float y = originY + i*(CELL_S + GAP_S);

                sf::RectangleShape rect({CELL_S,CELL_S});
                rect.setPosition({x,y});
                rect.setFillColor(tileColor(board[i][j]));
                window.draw(rect);

                if(board[i][j]!=0) {

                    sf::Text text(font,std::to_string(board[i][j]),40 * scale);
                    text.setFillColor(textColor(board[i][j]));

                    sf::FloatRect b = text.getLocalBounds();

                    text.setPosition({
                        x + (CELL_S/2.f) - b.size.x/2.f,
                        y + (CELL_S/2.f) - b.size.y
                    });

                    window.draw(text);
                }
            }
        }

        // ================= GAME OVER =================
        if(gameOver) {
            sf::Text over(font,"GAME OVER\nPress R",50);
            over.setFillColor(sf::Color::Red);

            sf::FloatRect b=over.getLocalBounds();

            over.setPosition({
                (w - b.size.x)/2.f,
                (h - b.size.y)/2.f
            });

            window.draw(over);
        }

        window.display();
    }

    return 0;
}