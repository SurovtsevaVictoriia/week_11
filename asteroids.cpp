#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
#include <random>
#include<String>


using namespace sf;

//width of window
const int W = 1200;
//hight of window
const int H = 800;

//dergees to radians
float DEGTORAD = 0.017453f;

int score_multiplier = 100;
//numbers 
int n_Heart    =        5;
int n_Entities =        15;
int n_Rock_small =      2;

//velocities
int   v_Bullet =        6;
float v_Player_rot =    0.2;
float v_Player_accel =  0.9;
int   v_Player_max =    15;
//angles
int a_Player_rot = 3;

//returns mersenne()
int f_rand() {

    std::random_device rd;
    std::mt19937 mersenne(rd());

    //return mersenne();
    return rand();
}


//starts an animation
class Animation
{
public:
    float Frame;
    float speed;

    //sprite
    Sprite sprite;
    //vecyor of frame numbers
    std::vector<IntRect> frames;

    Animation() {}

    //constructor(texture, x, y, w, h, count, speed)
    Animation(Texture& t, int x, int y, int w, int h, int count, float Speed)
    {
        Frame = 0;
        speed = Speed;

        for (int i = 0;i < count;i++)
            frames.push_back(IntRect(x + i * w, y, w, h));

        sprite.setTexture(t);
        sprite.setOrigin(w / 2, h / 2);
        sprite.setTextureRect(frames[0]);
    }




    void update()
    {
        Frame += speed;
        int n = frames.size();
        if (Frame >= n) Frame -= n;//goes to beginning
        if (n > 0) sprite.setTextureRect(frames[int(Frame)]);
    }

    bool isEnd()
    {
        return Frame + speed >= frames.size();
    }

};


class Entity
{
public:
    float x, y, 
          dx, dy, 
          R, angle;

   
    bool  life;
    std::string name;
    Animation anim;

    Entity()
    {
        life = 1;
    }
   
    //settings(animation, x, y, angle , radius)
    void settings(Animation& a, int X, int Y, float Angle = 0, int radius = 1)
    {
        anim = a;
        x = X; y = Y;
        angle = Angle;
        R = radius;
    }

    void move() {

        x += dx;
        y += dy;

        if (x > W) x = 0;  if (x < 0) x = W;
        if (y > H) y = 0;  if (y < 0) y = H;
    };


    virtual void update() {};

    void draw(RenderWindow& app)
    {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);

    }

    virtual ~Entity() {};
};

class Death_Screen:public Entity {

public:
    Death_Screen() {};

};

void death(sf::RenderWindow& app, Event event, std::list<Entity*> & entities, Sprite background) {

    std::list<Entity*>::iterator range_begin = entities.begin();
    std::list<Entity*>::iterator range_end = entities.end();

    entities.erase(range_begin, range_end);

    Texture tds;
    Animation sDeath_Screen(tds, 0, 0, 64, 64, 16, 0.2);
    tds.loadFromFile("images/death_screen.png");


    Death_Screen* a = new Death_Screen();
    a->settings(sDeath_Screen, W/2, H/2, 270, 25);

  


    while (app.isOpen())
    {

        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();
        }


        a->anim.update();
       
        app.draw(background);
        a->draw(app);
        app.display();
       
    }
}



class asteroid : public Entity
{
public:
    asteroid()
    {
        dx = f_rand() % 7 - 3;
        dy = f_rand() % 7 - 3;
        name = "asteroid";
    }

    void update()
    {
        move();
    }

};


class bullet : public Entity
{
public:
    bullet()
    {
        name = "bullet";
    }

    void  update()
    {
        dx = cos(angle * DEGTORAD) * v_Bullet;
        dy = sin(angle * DEGTORAD) * v_Bullet;
        
        x += dx;
        y += dy;

        if (x > W || x<0 || y>H || y < 0) life = 0;
    }

};


class player : public Entity
{
public:
    bool thrust;
    
    player(){
        name = "player";
    }

    void update()
    {   
        //acceleration
        if (thrust)
        {
            dx += cos(angle * DEGTORAD) * v_Player_rot;
            dy += sin(angle * DEGTORAD) * v_Player_rot;
        }
        else
        {
            dx *= v_Player_accel;
            dy *= v_Player_accel;
        }

        int maxSpeed = v_Player_max;
        float speed = sqrt(dx * dx + dy * dy);
        if (speed > maxSpeed)
        {
            dx *= maxSpeed / speed;
            dy *= maxSpeed / speed;
        }

        move();
    }

};

bool isCollide(Entity* a, Entity* b)
{
    return (b->x - a->x) * (b->x - a->x) +
        (b->y - a->y) * (b->y - a->y) <
        (a->R + b->R) * (a->R + b->R);
}


class Heart_Board {

public:

    int heart_size = 64;
   
    std::vector<Sprite*> sprites;
    std::vector<IntRect> frames;


    Heart_Board(Texture& t) {

        for (int i = 0; i < 2; i++)
            frames.push_back(IntRect(0 + i * heart_size, 0, heart_size, heart_size));

        for (int i = 0; i < n_Heart; ++i) {

            Sprite* sprite = new Sprite;
            sprites.push_back(sprite);

            sprites[i]->setOrigin(heart_size / 2, heart_size / 2);
            sprites[i]->setTexture(t);           
            sprites[i]->setTextureRect(frames[0]);
            sprites[i]->setPosition(W - i * heart_size - 40, 40);
        }

         
    };

   

    void update(int Hearts) {
        for (int i = 0; i < n_Heart; i++) {
            if (i < Hearts) {
                sprites[i]->setTextureRect(frames[0]);
            }
            else { sprites[i]->setTextureRect(frames[1]); }

        }
    }

    void draw(sf::RenderWindow& app) {
        for (int i = 0; i < n_Heart; i++) {
            app.draw(*sprites[i]);
        }
    }
    

};

class Score_Board {
public:
    Text text;
    int value;
    Font font;

   
    Score_Board() { // Declare and load a font
       
        value = 0;
        font.loadFromFile("arial.ttf");
        // Create a text
        sf::Text text(std::to_string(value), font);
        text.setPosition(10, 10);
        text.setCharacterSize(10);
        //text.setFillColor(sf::Color::Red);
        // Draw it
    }
    void update(int Score) {

        value = Score;
        text.setString(std::to_string(value));
    }
    void draw(sf::RenderWindow& app) {
        app.draw(text);
    }

};
 


int main()
{
    int Hearts = n_Heart;
    int Score = 0;

    RenderWindow app(VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    Texture t1, t2, t3, t4, t5, t6, t7, th;
    t1.loadFromFile("images/spaceship.png");
    t2.loadFromFile("images/background.jpg");
    t3.loadFromFile("images/explosions/type_C.png");
    t4.loadFromFile("images/rock.png");
    t5.loadFromFile("images/fire_blue.png");
    t6.loadFromFile("images/rock_small.png");
    t7.loadFromFile("images/explosions/type_B.png");
    th.loadFromFile("images/hearts.png");

    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);
    Heart_Board heart_board(th);
    Score_Board score_board;

    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);
   

    std::list<Entity*> entities;

    for (int i = 0;i < n_Entities ;i++)
    {
        asteroid* a = new asteroid();
        a->settings(sRock, f_rand() % W, f_rand() % H, f_rand() % 360, 25);
        entities.push_back(a);
    }

    player* p = new player();
    p->settings(sPlayer, 200, 200, 0, 20);
    entities.push_back(p);

    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space)
                {
                    bullet* b = new bullet();
                    b->settings(sBullet, p->x, p->y, p->angle, 10);
                    entities.push_back(b);
                }
        }

        if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += a_Player_rot;
        if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle -= a_Player_rot;
        if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust = true;
        else p->thrust = false;


        for (auto a : entities)
            for (auto b : entities)
            {
                if (a->name == "asteroid" && b->name == "bullet")
                    if (isCollide(a, b))
                    {
                        Score += score_multiplier;
                        a->life = false;
                        b->life = false;

                        Entity* e = new Entity();
                        e->settings(sExplosion, a->x, a->y);
                        e->name = "explosion";
                        entities.push_back(e);


                        for (int i = 0;i < n_Rock_small; i++)
                        {
                            if (a->R == 15) continue;
                            Entity* e = new asteroid();
                            e->settings(sRock_small, a->x, a->y, f_rand() % 360, 15);
                            entities.push_back(e);
                        }

                    }

                if (a->name == "player" && b->name == "asteroid")
                    if (isCollide(a, b))
                    {
                        b->life = false;
                        Hearts--;

                        Entity* e = new Entity();
                        e->settings(sExplosion_ship, a->x, a->y);
                        e->name = "explosion";
                        entities.push_back(e);


                        if (Hearts > 0) {
                            p->settings(sPlayer, W / 2, H / 2, 0, 20);
                            p->dx = 0; p->dy = 0;
                        }
                        else {
                            death(app, event, entities, background);
                            return 0;
                        }
                    }
            }


        if (p->thrust)  p->anim = sPlayer_go;
        else   p->anim = sPlayer;


        for (auto e : entities)
            if (e->name == "explosion")
                if (e->anim.isEnd()) e->life = 0;

        if (f_rand() % 150 == 0)
        {
            asteroid* a = new asteroid();
            a->settings(sRock, 0, f_rand() % H, f_rand() % 360, 25);
            entities.push_back(a);
        }

        for (auto i = entities.begin();i != entities.end();)
        {
            Entity* e = *i;

            e->update();
            e->anim.update();

            if (e->life == false) { i = entities.erase(i); delete e; }
            else i++;
        }

        heart_board.update(Hearts);
        score_board.update(Score);

        //////draw//////
        app.draw(background);
        for (auto i : entities) i->draw(app);

        heart_board.draw(app);
        score_board.draw(app);

        app.display();
    }

    return 0;
}
