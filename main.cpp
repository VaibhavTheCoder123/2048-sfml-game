#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>

using Board = std::array<std::array<int,4>,4>;

std::random_device rd;
std::mt19937 gen(rd());

int bestScore = 0;

void loadBestScore()
{
    std::ifstream file("bestscore.txt");
    if(file) file >> bestScore;
}

void saveBestScore()
{
    std::ofstream file("bestscore.txt");
    file << bestScore;
}

void spawnTile(Board& board)
{
    std::vector<std::pair<int,int>> empty;

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(board[i][j]==0)
                empty.push_back({i,j});

    if(empty.empty()) return;

    std::uniform_int_distribution<> dist(0, empty.size()-1);
    auto pos = empty[dist(gen)];

    std::uniform_int_distribution<> val(0,9);
    board[pos.first][pos.second] = (val(gen)==0)?4:2;
}

bool moveLeft(Board& board,int& score)
{
    bool moved=false;

    for(int i=0;i<4;i++)
    {
        for(int j=1;j<4;j++)
        {
            if(board[i][j]==0) continue;

            int col=j;

            while(col>0 && board[i][col-1]==0)
            {
                board[i][col-1]=board[i][col];
                board[i][col]=0;
                col--;
                moved=true;
            }

            if(col>0 && board[i][col-1]==board[i][col])
            {
                board[i][col-1]*=2;
                score+=board[i][col-1];
                board[i][col]=0;
                moved=true;
            }
        }
    }

    return moved;
}

void rotate(Board& board)
{
    Board temp=board;

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            board[i][j]=temp[3-j][i];
}

bool move(Board& board,int dir,int& score)
{
    bool moved;

    for(int i=0;i<dir;i++)
        rotate(board);

    moved=moveLeft(board,score);

    for(int i=0;i<(4-dir)%4;i++)
        rotate(board);

    return moved;
}

bool canMove(Board board)
{
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

// 🎨 Improved Color Palette
sf::Color tileColor(int v)
{
    switch(v)
    {
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

// Text color for contrast
sf::Color textColor(int v)
{
    if(v <= 4) return sf::Color(119,110,101); // dark text
    return sf::Color::White; // light text
}

void resetGame(Board& board,int& score)
{
    board={0};
    score=0;
    spawnTile(board);
    spawnTile(board);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({600,700}),"2048");

    sf::Font font;
    if(!font.openFromFile("assets/fonts/fonts/arial.ttf"))
    {
        std::cout<<"Font loading failed\n";
        return -1;
    }

    loadBestScore();

    Board board={0};
    int score=0;

    bool startScreen=true;
    bool gameOver=false;

    resetGame(board,score);

    float cell=120;
    float pad=10;

    while(window.isOpen())
    {
        while(auto event=window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
                window.close();

            if(event->is<sf::Event::KeyPressed>())
            {
                auto key=event->getIf<sf::Event::KeyPressed>();

                if(startScreen)
                {
                    startScreen=false;
                    continue;
                }

                if(key->code==sf::Keyboard::Key::R)
                {
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

                if(moved)
                {
                    spawnTile(board);

                    if(score>bestScore)
                    {
                        bestScore=score;
                        saveBestScore();
                    }

                    if(!canMove(board))
                        gameOver=true;
                }
            }
        }

        // 🎨 Background
        window.clear(sf::Color(250,248,239));

        if(startScreen)
        {
            sf::Text start(font,"2048\nPress Any Key",50);
            start.setFillColor(sf::Color(50,50,50));
            start.setPosition({120,300});
            window.draw(start);
            window.display();
            continue;
        }

        // Board background
        sf::RectangleShape boardBg({550,550});
        boardBg.setFillColor(sf::Color(187,173,160));
        boardBg.setPosition({10,120});
        window.draw(boardBg);

        for(int i=0;i<4;i++)
        {
            for(int j=0;j<4;j++)
            {
                sf::RectangleShape rect({cell,cell});
                rect.setPosition({pad+j*(cell+pad),pad+i*(cell+pad)+120});
                rect.setFillColor(tileColor(board[i][j]));

                window.draw(rect);

                if(board[i][j]!=0)
                {
                    sf::Text text(font,std::to_string(board[i][j]),40);
                    text.setFillColor(textColor(board[i][j]));
                    text.setPosition({pad+j*(cell+pad)+40,pad+i*(cell+pad)+150});
                    window.draw(text);
                }
            }
        }

        sf::Text scoreText(font,"Score: "+std::to_string(score),35);
        scoreText.setFillColor(sf::Color(50,50,50));
        scoreText.setPosition({20,20});
        window.draw(scoreText);

        sf::Text bestText(font,"Best: "+std::to_string(bestScore),35);
        bestText.setFillColor(sf::Color(50,50,50));
        bestText.setPosition({350,20});
        window.draw(bestText);

        if(gameOver)
        {
            sf::Text over(font,"GAME OVER\nPress R",40);
            over.setFillColor(sf::Color::Red);
            over.setPosition({150,320});
            window.draw(over);
        }

        window.display();
    }
}