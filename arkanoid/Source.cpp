#include <SFML/Graphics.hpp>

constexpr int windowWidth = 800 , windowHeight = 600;
constexpr float ballRadius = 15 , ballSpeed =  6 ;
constexpr float bouncerHeight = 20, bouncerWidth = 80, bouncerSpeed = 8;
constexpr float brickHeight = 20, brickWidth = 60;
constexpr int brickX = 11, brickY = 4;

struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f speed{ -ballSpeed, -ballSpeed };
    bool gameOver = 0;

    Ball(float posX, float posY) 
    {
        shape.setPosition(posX, posY);
        shape.setRadius(ballRadius);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(ballRadius,ballRadius);
    }

    void update()
    {
        shape.move(speed);

        if (left() < 0)
            speed.x = ballSpeed;
        else if (right() > windowWidth)
            speed.x = -ballSpeed;

        if (top() < 0)
            speed.y = ballSpeed;
        else if (bottom() > windowHeight)
            gameOver = 1;

    }
    float x() { return shape.getPosition().x; }
    float y() { return shape.getPosition().y; }
    float left() { return x() - shape.getRadius(); }
    float right() { return x() + shape.getRadius(); }
    float top() { return y() - shape.getRadius(); }
    float bottom() { return y() + shape.getRadius(); }
};

struct Rectangle
{
    sf::RectangleShape shape;

    float x() { return shape.getPosition().x; }
    float y() { return shape.getPosition().y; }
    float left() { return x() - shape.getSize().x/2; }
    float right() { return x() + shape.getSize().x/2; }
    float top() { return y() - shape.getSize().y/2; }
    float bottom() { return y() + shape.getSize().y/2; }
};

struct Bouncer : public Rectangle
{
    sf::Vector2f speed;

    Bouncer(float posX, float posY)
    {
        shape.setPosition(posX, posY);
        shape.setSize({ bouncerWidth, bouncerHeight });
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin(bouncerWidth/2, bouncerHeight/2);
    }

    void update()
    {
        shape.move(speed);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
        {
            speed.x = -bouncerSpeed;
        }
        else {
            speed.x = 0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && right() < windowWidth)
        {
            speed.x = bouncerSpeed;
        }
    }
};

struct Brick : public Rectangle
{
    bool doesNotExist = false;
    Brick(float posX, float posY)
    {
        shape.setPosition(posX, posY);
        shape.setSize({ brickWidth, brickHeight });
        shape.setFillColor(sf::Color::Blue);
        shape.setOrigin(brickWidth / 2, brickHeight / 2);
    }
};

template <class T1, class T2>
bool isIntersecting(T1& mA, T2& mB)
{
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
        mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Bouncer& bouncer, Ball& ball)
{
    if (!isIntersecting(bouncer,ball)) return;

    ball.speed.y = -ballSpeed;
    if (ball.x() < bouncer.x())
        ball.speed.x = -ballSpeed;
    else
        ball.speed.x = ballSpeed;
}

void testCollision(Brick& brick, Ball& ball)
{
    if (!isIntersecting(brick, ball)) return;
    brick.doesNotExist = true;

    float overlapLeft{ ball.right() - brick.left() };
    float overlapRight{ brick.right() - ball.left() };
    float overlapTop{ ball.bottom() - brick.top() };
    float overlapBottom{ brick.bottom() - ball.top() };

    bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));
    bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

    float minOverlapX{ ballFromLeft ? overlapLeft : overlapRight };
    float minOverlapY{ ballFromTop ? overlapTop : overlapBottom };

    if (abs(minOverlapX) < abs(minOverlapY))
        ball.speed.x = ballFromLeft ? -ballSpeed : ballSpeed;
    else
        ball.speed.y = ballFromTop ? -ballSpeed : ballSpeed;
}


int main()
{
    sf::RenderWindow window{ {windowWidth, windowHeight}, "game", sf::Style::Default };
    window.setFramerateLimit(60);
    sf::Event event;

    Ball ball(400, 400);
    Bouncer bouncer(400, 500);
    std::vector<Brick> bricks;
    for (int iX = 0; iX < brickX; iX++)
    {
        for (int iY = 0; iY < brickY; iY++)
        {
            bricks.emplace_back((iX + 1) * (brickWidth + 3) + 22, (iY + 2) * (brickHeight + 3));
        }
    }

    sf::Text text;
    sf::Font font;
    font.loadFromFile("arial.ttf");
    text.setFont(font);
    text.setString("Press Space to Start!");
    sf::FloatRect textRect = text.getLocalBounds();
    text.setFillColor(sf::Color::Yellow);
    text.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    text.setPosition(window.getView().getCenter());

    bool gameStarted = 0;
    while (window.isOpen()) {
        while (window.pollEvent(event))
        {
            window.clear();
            window.draw(text);
            window.display();
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Q)
                {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::Space)
                {
                    gameStarted = 1;
                }
                break;
            default:
                break;
            }
        }
        if (gameStarted)
            break;
    }

    while (gameStarted == true)
    {

        window.clear(sf::Color::Black);
        ball.update();
        bouncer.update();
        testCollision(bouncer, ball);
        for (auto& brick : bricks) testCollision(brick, ball);
        bricks.erase(remove_if(begin(bricks), end(bricks),
            [](const Brick& mBrick)
            {
                return mBrick.doesNotExist;
            }),
            end(bricks));
        if (bricks.empty())
        {
            text.setString("You win!");
            textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            text.setPosition(window.getView().getCenter());
            window.clear();
            window.draw(text);
            window.display();
            sf::sleep(sf::seconds(3));
            break;
        }
        window.draw(ball.shape);
        window.draw(bouncer.shape);
        for (auto& brick : bricks) window.draw(brick.shape);
        window.display();

        if (ball.gameOver)
        {

            text.setString("You lose!");
            textRect = text.getLocalBounds();
            text.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            text.setPosition(window.getView().getCenter());
            window.clear();
            window.draw(text);
            window.display();
            sf::sleep(sf::seconds(3));
            break;
        }

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Q)
                {
                    window.close();
                }
                break;
            default:
                break;
            }
        }
    }
    
    return 0;
}