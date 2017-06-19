#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_net/SDL_net.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <math.h>
#include <vector>
#include <algorithm>
#include "Box2D.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

double PI = 3.14;
bool net = false;
int mapIndex = 0;
int p1_weaponIndex = 0;
int p2_weaponIndex = 0;
int p1_carIndex = 0;
int p2_carIndex = 0;
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

enum _ScreenStatus{
    Main_Screen,
    Game_Setting,
    Game_Instruction,
    Car_Chose,Car_Chose_net,
    Car_Modified,
    Car_Modified_net,
    Weapon_Chose,
    Weapon_Chose_net,
    Skill_Chose,
    Skill_Chose_net,
    Map_Chose,
    Map_Chose_net,
    Game_Start,
    Game_Start_net,
    Net_Work,
    Player_Chose,
    Client_Set,
    Server_Set,
};

/// Initiall Screen Status
_ScreenStatus ScreenStatus = Main_Screen;
//========= Screen Information =================================
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 750;
const char WINDOW_TITLE[20] = "BumperCar";
const int SCREEN_FPS = 60;
const int SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;

//========= Box2D ==============================================
float32 timeStep = 1.0f / 60.0f;
int32 velocityIterations = 6;
int32 positionIterations = 2;

b2Vec2 gravity(0, 0);

b2World* myWorld = new b2World(gravity);

enum _entityCategory{
    NORMAL_WALL = 0x0001,
    CAR = 0x0002,
    WEAPON = 0x0003,
};

enum _engine_rate{
    EngineA = 30000,
    EngineB = 80000,
    EngineC = 130000,
    EngineD = 200000,
};

enum _steer_rate{
    SteerA = 150,
    SteerB = 250,
    SteerC = 350,
    SteerD = 450,
};

enum _tire_rate{
    TireA = 300,
    TireB = 600,
    TireC = 1000,
    TireD = 1400,
};


//========= Header =============================================
/*The Statictexture on the screen*/
class StaticTexture
{
public:
    //Initializes variables
    StaticTexture();
    
    //Deallocates memory
    ~StaticTexture();
    
    //Loads image at specified path
    bool loadFromFile( std::string path );
    
    //Deallocates texture
    void free();
    
    //Renders texture at given point
    void render();
    void render( int x, int y );
    
    void setAlpha(uint8 alpha);
    
    //Gets image dimensions
    int getWidth();
    int getHeight();
    
    int getPosX();
    int getPosY();
    
    bool empty();
    
    //Gets the collision boxes
    std::vector<SDL_Rect>& getColliders();
    
protected:
    //The actual hardware texture
    SDL_Texture* mTexture;
    
    //The X and Y offsets
    int mPosX, mPosY;
    
    //Image dimensions
    int mWidth;
    int mHeight;
    
    //Dot's collision boxes
    std::vector<SDL_Rect> mColliders;
    
    //Moves the collision boxes relative to the dot's offset
    void shiftColliders();
};

/*The DynamicTexture that will move around on the screen*/
class DynamicTexture : public StaticTexture
{
public:
    //Maximum axis velocity of the DynamicTexture
    static const int DynamicTexture_VEL = 1;
    
    //Initializes the variables
    DynamicTexture();
    DynamicTexture(int x, int y);
    DynamicTexture(const DynamicTexture& a);
    
    //Takes key presses and adjusts the DynamicTexture's velocity
    void handleEvent( SDL_Event& e );
    
    //Moves the DynamicTexture
    //void move(std::vector<SDL_Rect>& otherColliders);
    
    //Shows the DynamicTexture on the screen
    void render();
    void render(int x, int y, double angle);
    
private:
    //The velocity of the DynamicTexture
    int mVelX, mVelY;
};

class Text : public StaticTexture{
private:
    std::string text;
    SDL_Color color;
    TTF_Font* font;
public:
    Text();
    void setText(std::string);
    void setColor(SDL_Color color);
    void setFont(TTF_Font* font);
    //Creates image from font string
    bool loadFromRenderedText();
};




class LTimer
{
public:
    //Initializes variables
    LTimer();
    
    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();
    
    //Gets the timer's time
    Uint32 getTicks();
    
    //Checks the status of the timer
    bool isStarted();
    bool isPaused();
    
private:
    //The clock time when the timer started
    Uint32 mStartTicks;
    
    //The ticks stored when the timer was paused
    Uint32 mPausedTicks;
    
    //The timer status
    bool mPaused;
    bool mStarted;
};

class SoundEffect{
private:
    Mix_Chunk* gEffect;
public:
    SoundEffect();
    bool loadSoundFile(std::string path);
    void handleEvent(SDL_Event& e, SDL_Scancode key);
    void free();
    void play();
};

class Music{
private:
    Mix_Music* gMusic;
public:
    Music();
    bool loadSoundFile(std::string path);
    void handleEvent(SDL_Event &e, SDL_Scancode Play_Pause, SDL_Scancode Stop);
    void play();
    void free();
};

int soundEffectChannel = 1;

class boxCar{
public:
    TTF_Font* font;
    Text countDownText;
    int countDown = 3000;
    int currentTime = 0;
    b2Body* car;
    int player = 1;
    int engine = EngineC;
    double steer = SteerC;
    int tire = TireB;
    int moretire = 0;
    double mvelR = 0;
    int mVel = 0;
    int hurt_contacting;
    int weapon_contacting;
    
    SoundEffect enginePower;
    
    boxCar(int x, int y, int player);
    void setWeapon(int weaponIndex);
    void handelEvent(SDL_Event &e);
    void setPerformance(int engine, int steer, int tire);
    void move();
    void friction();
    void increasFriction(int x){
        this->moretire = x;
        b2Vec2 vel = car->GetLinearVelocity();
        vel.x = -vel.x*(moretire);
        vel.y = -vel.y*(moretire);
        car->ApplyForce(vel,car->GetWorldCenter(), true);
    }
    void hurt_startContact() { hurt_contacting ++; }
    void hurt_endContact() { hurt_contacting --; }
    void weapon_startContact() { weapon_contacting ++; }
    void weapon_endContact() { weapon_contacting --; }
    
    int life;
    SDL_Rect lifeBar;
    
    StaticTexture recoverIMG;
    StaticTexture recoverIMGBW;
    bool recoverFlag = true;
    int recoverTime = 8000;
    int recoverCount = 0;
    int recoverTool = 0;
    SDL_Rect recoverBar;
    
    StaticTexture defenseICON;
    StaticTexture defenseIMG;
    StaticTexture defenseICONBW;
    float32 defenseAngle = 0;
    int defenseRadius = 120;
    bool defenseFlag = false;
    int defenseTime = 5000;
    int defenseCount = 0;
    bool defenseTool = true;
    bool renderDefense = false;
    void defenseRender();
    void defense();
    b2Fixture* defenseFixture = NULL;
    
    StaticTexture fireIMG;
    StaticTexture fireICON;
    StaticTexture fireICONBW;
    void fireTimer();
    void fireAttack();
    bool fireFlag = false;
    int fireTime = 10000;
    int fireCount = 0;
    bool fireTool = true;
    SDL_Rect fireBar;
    b2Fixture* fireFixture = NULL;
    b2FixtureDef fireWeapon;
    float32 fireAngle = 0;
    int fireRadius = 120;
    
    void fireRender();
    int fireRenderTime = 5000;
    int fireRenderCount = 0;
    bool renderFire = false;
    bool renderFireFlag = false;
    
    void reduce();
    void hurt(boxCar* enemy);
    void reduce(int weaponIndex);
    void recoverTimer();
    void recover();
    void recover(int m);
    void renderLife(boxCar *enemy);
    bool die();
};

class MyContactListener : public b2ContactListener
{
    void BeginContact(b2Contact* contact) {
        
        void* bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData();
        void* bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();
        b2Filter fixtureA = contact->GetFixtureA()->GetFilterData();
        b2Filter fixtureB = contact->GetFixtureB()->GetFilterData();
        if ( bodyUserDataB && bodyUserDataA ){
            if(fixtureA.categoryBits == WEAPON && fixtureB.categoryBits == CAR){
                static_cast<boxCar*>( bodyUserDataB )->hurt_startContact();
            }
            else if(fixtureA.categoryBits == CAR && fixtureB.categoryBits == WEAPON){
                static_cast<boxCar*>( bodyUserDataA )->hurt_startContact();
            }
            else if(fixtureA.categoryBits == WEAPON && fixtureB.categoryBits == WEAPON){
                static_cast<boxCar*>( bodyUserDataA )->weapon_startContact();
                static_cast<boxCar*>( bodyUserDataB )->weapon_startContact();
            }
        }
    }
    
    void EndContact(b2Contact* contact) {
        
        void* bodyUserDataA = contact->GetFixtureA()->GetBody()->GetUserData();
        void* bodyUserDataB = contact->GetFixtureB()->GetBody()->GetUserData();
        b2Filter fixtureA = contact->GetFixtureA()->GetFilterData();
        b2Filter fixtureB = contact->GetFixtureB()->GetFilterData();
        if ( bodyUserDataB && bodyUserDataA ){
            if(fixtureA.categoryBits == WEAPON && fixtureB.categoryBits == CAR){
                static_cast<boxCar*>( bodyUserDataB )->hurt_endContact();
            }
            else if(fixtureA.categoryBits == CAR && fixtureB.categoryBits == WEAPON){
                static_cast<boxCar*>( bodyUserDataA )->hurt_endContact();
            }
            else if(fixtureA.categoryBits == WEAPON && fixtureB.categoryBits == WEAPON){
                static_cast<boxCar*>( bodyUserDataA )->weapon_endContact();
                static_cast<boxCar*>( bodyUserDataB )->weapon_endContact();
            }
        }
        
    }
};


class MainList{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* font;
    Text choiceText[3];
    StaticTexture ptrIMG;
    std::string choice[3] = {"Enter Game","Game Setting","Game Instructions"};
    int ptr;
public:
    MainList();
    void render();
    void controlHandle(SDL_Event& e);
};

class GameSettingList{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* font;
    Text settingText[3];
    StaticTexture bar;
    StaticTexture bar2;
    StaticTexture cube;
    StaticTexture cube2;
    StaticTexture muteCube;
    StaticTexture muteCube2;
    std::string setting[3] = {"Sound Effect Volumn", "Background Music Volumn", "Back"};
    
    int mainPtr = 0;
    
    //Adjust channel volumn of sound effect
    int soundEffectVolumn = 96;
    int soundEffectPtr = 8;
    
    //Adjust volumn of background music
    int backgroundMusicVolumn = 96;
    int backgroundMusicVolumnPtr = 8;
public:
    GameSettingList();
    void handleControl(SDL_Event& e);
    void render();
};

class GameInstructions{
private:
    SoundEffect beep8;
    TTF_Font* font;
    Text back;
    int ptr = 0;
public:
    GameInstructions();
    void handleControl(SDL_Event &e);
    void render();
};

class CarChoses{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    StaticTexture carBar[10];
    StaticTexture carIcon[10];
    StaticTexture highlight[3];
    int p1_ptr = 0;
    int p2_ptr = 0;
public:
    CarChoses();
    void handleControl(SDL_Event &e);
    void render();
    void setNet(){net = true;};
};

CarChoses::CarChoses(){
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    highlight[0].loadFromFile("content/car/p1.png");
    highlight[1].loadFromFile("content/car/p2.png");
    highlight[2].loadFromFile("content/car/p3.png");
    for(int i = 0; i < 10; i++){
        carBar[i].loadFromFile("content/car/c" + std::to_string(i) + "b.png");
        carIcon[i].loadFromFile("content/car/c" + std::to_string(i) + ".png");
    }
}

void CarChoses::handleControl(SDL_Event &e){
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(p1_ptr > 4){
                    p1_ptr -= 5;
                    beep2.play();
                }
                break;
            case SDLK_DOWN:
                if(p1_ptr < 5){
                    p1_ptr += 5;
                    beep2.play();
                }
                break;
            case SDLK_LEFT:
                if(p1_ptr > 0){
                    p1_ptr--;
                    beep2.play();
                }
                break;
            case SDLK_RIGHT:
                if(p1_ptr < 9){
                    p1_ptr++;
                    beep2.play();
                }
                break;
            case SDLK_w:
                if(p2_ptr > 4){
                    p2_ptr -= 5;
                    beep2.play();
                }
                break;
            case SDLK_s:
                if(p2_ptr < 5){
                    p2_ptr += 5;
                    beep2.play();
                }
                break;
            case SDLK_a:
                if(p2_ptr > 0){
                    p2_ptr--;
                    beep2.play();
                }
                break;
            case SDLK_d:
                if(p2_ptr < 9){
                    p2_ptr++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                if(net){
                    ScreenStatus = Weapon_Chose_net;
                }
                else{
                    ScreenStatus = Weapon_Chose;
                }
                p1_carIndex = p1_ptr;
                p2_carIndex = p2_ptr;
                beep8.play();
                break;
            case SDLK_BACKSPACE:
                ScreenStatus = Main_Screen;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void CarChoses::render(){
    
    for(int i = 0 ; i < 5; i++){
        if(i == p1_ptr && i == p2_ptr){
            highlight[2].render(192+125*i, 20);
        }
        else if(i == p2_ptr){
            highlight[1].render(192+125*i, 20);
        }
        else if(i == p1_ptr){
            highlight[0].render(192+125*i, 20);
        }
        carBar[i].render(200+125*i, 30);
    }
    for(int i = 5; i < 10; i++){
        if(i == p1_ptr && i == p2_ptr){
            highlight[2].render(192+125*(i-5), 180);
        }
        else if(i == p2_ptr){
            highlight[1].render(192+125*(i-5), 180);
        }
        else if(i == p1_ptr){
            highlight[0].render(192+125*(i-5), 180);
        }
        carBar[i].render(200+125*(i-5), 190);
    }
    carIcon[p1_ptr].render(100, 360);
    carIcon[p2_ptr].render(600, 360);
}

class NetWorkList{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* font;
    Text netWorkChose[2];
    std::string netWorkChoseText[2] = {"Offline","Online"};
    int netWork_ptr = 0;
public:
    NetWorkList();
    void handleControll(SDL_Event &e);
    void render();
};

NetWorkList::NetWorkList(){
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    for(int i = 0; i < 2; i++){
        netWorkChose[i].setFont(font);
        netWorkChose[i].setText(netWorkChoseText[i]);
    }
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
}

void NetWorkList::handleControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(netWork_ptr > 0){
                    netWork_ptr--;
                    beep2.play();
                }
                break;
            case SDLK_DOWN:
                if(netWork_ptr < 1){
                    netWork_ptr++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                if(netWork_ptr == 0){
                    ScreenStatus = Car_Chose;
                    beep8.play();
                }
                else if(netWork_ptr == 1){
                    ScreenStatus = Player_Chose;
                    beep8.play();
                }
                break;
            case SDLK_BACKSPACE:
                ScreenStatus = Main_Screen;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void NetWorkList::render(){
    netWorkChose[0].setColor({255,255,255,255});
    netWorkChose[1].setColor({255,255,255,255});
    netWorkChose[netWork_ptr].setColor({201, 198, 28,255});
    netWorkChose[0].render(200, 200);
    netWorkChose[1].render(200, 400);
}

IPaddress ip;
TCPsocket remote;
int selfPlayer = 0;

class PlayerChoseList{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* font;
    Text PlayerChose[2];
    std::string PlayerChoseText[2] = {"I am Player 1","I am Player 2"};
    int playerChose_ptr = 0;

public:
    PlayerChoseList();
    void handleControll(SDL_Event &e);
    void render();
};
        
PlayerChoseList::PlayerChoseList(){
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    for(int i = 0; i < 2; i++){
        PlayerChose[i].setFont(font);
        PlayerChose[i].setText(PlayerChoseText[i]);
    }
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
}

void PlayerChoseList::handleControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(playerChose_ptr > 0){
                    playerChose_ptr--;
                    beep2.play();
                }
                break;
            case SDLK_DOWN:
                if(playerChose_ptr < 1){
                    playerChose_ptr++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                if(playerChose_ptr == 0){
                    selfPlayer = playerChose_ptr;
                    ScreenStatus = Server_Set;
                    beep8.play();
                }
                else if(playerChose_ptr == 1){
                    selfPlayer = playerChose_ptr;
                    ScreenStatus = Client_Set;
                    beep8.play();
                }
                break;
            case SDLK_BACKSPACE:
                ScreenStatus = Net_Work;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void PlayerChoseList::render(){
    PlayerChose[0].setColor({255,255,255,255});
    PlayerChose[1].setColor({255,255,255,255});
    PlayerChose[playerChose_ptr].setColor({201, 198, 28,255});
    PlayerChose[0].render(200, 200);
    PlayerChose[1].render(200, 400);
}

class ServerSet{
private:
    TTF_Font* font;
    TTF_Font* smallFont;
    Text msg;
    Text myIPtest;
    Text workMsg[2];
    std::string myIP = "";
public:
    bool work = false;
    ServerSet();
    void handleControll(SDL_Event &e);
    void render();
    void getMyIP();
    void net_work_server(){
        SDL_Init(SDL_INIT_EVERYTHING);
        SDLNet_Init();
        
        SDLNet_ResolveHost(&ip,NULL,1234);
        TCPsocket server=SDLNet_TCP_Open(&ip);
        
        while(1){
            remote=SDLNet_TCP_Accept(server);
            if(remote){
                this->work = true;
                break;
            }
        }
    }
};

ServerSet::ServerSet(){
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    smallFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 22);
    msg.setFont(font);
    workMsg[0].setFont(smallFont);
    workMsg[1].setFont(smallFont);
    myIPtest.setFont(smallFont);
    getMyIP();
    myIPtest.setText("My IP is : " + myIP);
    msg.setText("Waiting for connection ...");
    workMsg[0].setText("Net Connection is now working");
    workMsg[1].setText("Press Enter to Continue ...");
}

void ServerSet::getMyIP(){
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    
    getifaddrs(&ifAddrStruct);
    
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(strcmp(addressBuffer, "127.0.0.1") != 0)
                myIP.assign(addressBuffer);
        }
    }
    if (ifAddrStruct!=NULL)
        freeifaddrs(ifAddrStruct);//remember to free ifAddrStruct
}

void ServerSet::handleControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0 && work){
        switch (e.key.keysym.sym) {
            case SDLK_RETURN:
                ScreenStatus = Car_Chose_net;
                break;
            default:
                break;
        }
    }
}

void ServerSet::render(){
    if(!work){
        myIPtest.setColor({201, 198, 28,255});
        myIPtest.render(100, 100);
        msg.setColor({201, 198, 28,255});
        msg.render(100,200);
    }
    else{
        workMsg[0].setColor({201, 198, 28,255});
        workMsg[1].setColor({201, 198, 28,255});
        workMsg[0].render(100,200);
        workMsg[1].render(100,400);
    }
}

class ClientSet{
private:
    TTF_Font* font;
    TTF_Font* smallFont;
    Text msg;
    Text workMsg[2];
    Text input;
    std::string inputText = "";
public:
    bool work = false;
    ClientSet();
    void handleControll(SDL_Event &e);
    void render();
    void net_work_client(){
        SDL_Init(SDL_INIT_EVERYTHING);
        SDLNet_Init();
        
        //write "127.0.0.1",1234 to connect to the server.cpp on your local machine
        SDLNet_ResolveHost(&ip,inputText.c_str(),1234);
        
        remote=SDLNet_TCP_Open(&ip);
        
        if(remote != NULL){
            this->work = true;
        }
        else{
            std::cout << "Failed to Connect ...\n";
        }
    }
};

ClientSet::ClientSet(){
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    smallFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 22);
    msg.setFont(font);
    input.setFont(font);
    workMsg[0].setFont(smallFont);
    workMsg[1].setFont(smallFont);
    msg.setText("Enter Player 1's IP ...");
    workMsg[0].setText("Net Connection is now working");
    workMsg[1].setText("Press Enter to Continue ...");
}

void ClientSet::handleControll(SDL_Event &e){
    if(!work){
        //Special key input
        if( e.type == SDL_KEYDOWN)
        {
            //Handle backspace
            if( e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0 )
            {
                //lop off character
                inputText.pop_back();
            }
            else if(e.key.keysym.sym == SDLK_RETURN){
                net_work_client();
            }
            //Handle copy
            else if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
            {
                SDL_SetClipboardText( inputText.c_str() );
            }
            //Handle paste
            else if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
            {
                inputText = SDL_GetClipboardText();
            }
        }
        //Special text input event
        else if( e.type == SDL_TEXTINPUT)
        {
            //Not copy or pasting
            if( !( ( e.text.text[ 0 ] == 'c' || e.text.text[ 0 ] == 'C' ) && ( e.text.text[ 0 ] == 'v' || e.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
            {
                //Append character
                inputText += e.text.text;
            }
        }
    }
    else{
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN){
            ScreenStatus = Car_Chose_net;
        }
    }
}

void ClientSet::render(){
    if(!work){
        msg.setColor({201, 198, 28,255});
        msg.render(100,200);
        input.setText(inputText);
        input.setColor({255,255,255,255});
        input.render(100, 300);
    }
    else{
        workMsg[0].setColor({201, 198, 28,255});
        workMsg[1].setColor({201, 198, 28,255});
        workMsg[0].render(100,200);
        workMsg[1].render(100,400);
    }
}

class CarModified{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* font;
    Text playerText[2];
    std::string player[2] = {"PLAYER 1", "PLAYER 2"};
    Text modified[3];
    std::string modifiedText[3] = {"ENGINE","STEERING","TIRE"};
    SDL_Rect performance[3];
    StaticTexture tire[4];
    StaticTexture steer[4];
    StaticTexture engine[4];
    StaticTexture baclground;
    StaticTexture cube;
    StaticTexture noCube;
    int engine_ptr1 = 2;
    int steer_ptr1 = 2;
    int tire_ptr1 = 1;
    int engine_ptr2 = 2;
    int steer_ptr2 = 2;
    int tire_ptr2 = 1;
    int second_ptr1 = 1;
    int second_ptr2 = 1;
public:
    CarModified();
    void render();
    void handleControl(SDL_Event &e);
    void renderBar();
};

class SkillChose{
private:
    TTF_Font* weaponNamefont;
    SoundEffect beep2;
    SoundEffect beep8;
    StaticTexture weapon[5];
    StaticTexture weaponHighlight[5];
    Text weaponName[5];
    std::string weaponNameText[5];
    int weaponChose_ptr = 0;
public:
    SkillChose();
    void handelControll(SDL_Event &e);
    void render();
};

SkillChose::SkillChose(){
    weaponNamefont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 30);
    for(int i = 0; i < 5; i++){
        weapon[i].loadFromFile("content/weapon/weapon0.png");
        weapon[i].loadFromFile("content/weapon/weapon0Highlighted.png");
    }
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
}

void SkillChose::handelControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(SDLK_UP)
                break;
                
            default:
                break;
        }
    }
}

class WeaponChose{
private:
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* WeaponFont;
    TTF_Font* WeaponNameFont;
    Text weaponName[5];
    std::string weaponNameText[5] = {"Bloody Knife","Fan Knife","Shang Fang Sword", "Push", "Miracle AssPin"};
    Text player[2];
    std::string playerText[2] = {"Player 1","Player 2"};
    StaticTexture weaponBar[5];
    StaticTexture weaponBarHilightp1[5];
    StaticTexture weaponBarHilightp2[5];
    StaticTexture weaponBarHilight[5];
    StaticTexture weapon[5];
    int weapon_ptr_p1 = 0;
    int weapon_ptr_p2 = 0;
public:
    WeaponChose();
    void handelControll(SDL_Event &e);
    void render();
};

WeaponChose::WeaponChose(){
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    WeaponNameFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 28);
    WeaponFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    player[0].setFont(WeaponFont);
    player[1].setFont(WeaponFont);
    player[0].setColor({255,255,255,255});
    player[1].setColor({255,255,255,255});
    player[0].setText(this->playerText[0]);
    player[1].setText(this->playerText[1]);
    for(int i = 0; i < 5; i++){
        weaponName[i].setFont(WeaponNameFont);
        weaponName[i].setColor({255,255,255,255});
        weaponName[i].setText(weaponNameText[i]);
        weapon[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + ".png");
        weaponBar[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + "s.png");
        weaponBarHilightp1[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + "highlight.png");
        weaponBarHilightp2[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + "highlight2.png");
        weaponBarHilight[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + "highlight3.png");
    }
}

void WeaponChose::handelControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                if(weapon_ptr_p1 > 0){
                    weapon_ptr_p1--;
                    beep2.play();
                }
                break;
            case SDLK_RIGHT:
                if(weapon_ptr_p1 < 4){
                    weapon_ptr_p1++;
                    beep2.play();
                }
                break;
            case SDLK_d:
                if(weapon_ptr_p2 < 4){
                    weapon_ptr_p2++;
                    beep2.play();
                }
                break;
            case SDLK_a:
                if(weapon_ptr_p2 > 0){
                    weapon_ptr_p2--;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                if(!net){
                    ScreenStatus = Car_Modified;
                }
                else{
                    ScreenStatus = Car_Modified_net;
                }
                beep8.play();
                p1_weaponIndex = weapon_ptr_p1;
                p2_weaponIndex = weapon_ptr_p2;
                break;
            case SDLK_BACKSPACE:
                if(net)
                    ScreenStatus = Car_Chose_net;
                else
                    ScreenStatus = Car_Chose;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void WeaponChose::render(){
    for(int i = 0; i < 5; i++){
        if(i == weapon_ptr_p1 && i == weapon_ptr_p2){
            weaponBarHilight[i].render(100+150*i,50);
        }
        else if(i == weapon_ptr_p1){
            weaponBarHilightp1[i].render(100+150*i, 50);
        }
        else if (i == weapon_ptr_p2){
            weaponBarHilightp2[i].render(100+150*i, 50);
        }
        else{
            weaponBar[i].render(100+150*i, 50);
        }
    }
    player[0].render(70,210);
    player[1].render(530,210);
    weapon[weapon_ptr_p1].render(70, 270);
    weapon[weapon_ptr_p2].render(530, 270);
    weaponName[weapon_ptr_p1].render(70,650);
    weaponName[weapon_ptr_p2].render(530,650);
}

void GameInit();
class MapChose{
private:
    SoundEffect engineStart;
    SoundEffect beep2;
    SoundEffect beep8;
    TTF_Font* NameFont;
    TTF_Font* InfoFont;
    StaticTexture mapthumbnail[5];
    StaticTexture mapbar[5];
    StaticTexture mapbarHighlight[5];
    Text mapName[5];
    std::string napNameText[5] = {"Grass","Ocean","Mars","Desert","Sky"};
    Text mapInfo[5];
    StaticTexture info[5];
    int map_ptr = 0;
public:
    MapChose();
    void handelControll(SDL_Event &e);
    void render();
};


class GamePlaying{
private:
    int p1_angle = 0;
    DynamicTexture weapon[5];
    
    int scrollingOffset = 0;
    uint32 firstTick = 0;
    uint32 secondTick = 0;
    bool firstFlip = true;
    bool flip = true;
    SDL_Rect sand;
    StaticTexture sandIMG;
    DynamicTexture carIMG[10];
    
    Text countDown;
    StaticTexture map[5];
    TTF_Font* font;
    TTF_Font* fontBig;
    Text gameover[3];
    SoundEffect beep2;
    SoundEffect beep8;
    StaticTexture hpBarP1;
public:
    GamePlaying();
    void Init(){firstFlip = true;scrollingOffset = 0;firstTick = 0;secondTick = 0;flip = true;};
    void renderGameOver(int winner);
    void render();
    void handleGameOver(SDL_Event &e);
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

void Box2D_World();

bool keyAvoid(SDL_Event &e);
bool keyPlayingAvoid(SDL_Event &e);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures;
StaticTexture mainBackgroundPic;
StaticTexture gBackgroundTexture;

//Font
TTF_Font *PressStartTwoP;

MyContactListener myContactListenerInstance;


//=========== Main Program ======================================
boxCar car(150,50,1);
boxCar car2(50,50,2);
DynamicTexture P3(car.car->GetPosition().x, car.car->GetPosition().y);
DynamicTexture P4(car2.car->GetPosition().x, car2.car->GetPosition().y);

//Life p1_Life(60,130);
//Life p2_Life(660,130);

SoundEffect high;
Music backgroundMusic;

unsigned int gametime;

/*  Don't declare text here  */

int main( int argc, char* args[] )
{
    
    
    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        if( !loadMedia() ){
            printf("Failed to load media!\n");
        }
        else
        {

            /*  Declare text here  */
            Text soundEffectDetail;
            Text backgroundMusicDetail;
            Text posA;
            Text posB;
            Text carVel;
            Text carVel2;
            Text FPS;
        
            backgroundMusic.play();
            
            myWorld->SetContactListener(&myContactListenerInstance);
            
            //Main loop flag
            bool quit = false;
            
            Box2D_World();
            
            //Event handler
            SDL_Event e;
            SDL_Event e_remote;
            
            //The frames per second timer
            LTimer fpsTimer;
            //The frames per second cap timer
            LTimer capTimer;
            //Start counting frames per second
            int countedFrames = 0;
            fpsTimer.start();
            
            MainList mainlist;
            GameSettingList gamesettinglist;
            GameInstructions gameinstructions;
            CarChoses carchose;
            CarModified carmodified;
            NetWorkList networklist;
            PlayerChoseList playerchoselist;
            ServerSet serverset;
            ClientSet clientset;
            GamePlaying gameplaying;
            MapChose mapchose;
            WeaponChose weaponchose;
            
            //While application is running
            while( !quit )
            {
                // Instruct the world to perform a single step of simulation.
                myWorld->Step(timeStep, velocityIterations, positionIterations);
                
                //Start cap timer
                capTimer.start();
                
                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                SDL_RenderClear( gRenderer );
                
                if(ScreenStatus == Main_Screen){
                    
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        mainlist.controlHandle(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    mainlist.render();
                }
                
                else if(ScreenStatus == Game_Setting){
                    
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        gamesettinglist.handleControl(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    gamesettinglist.render();
                }
                
                else if(ScreenStatus == Game_Instruction){
                    
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        gameinstructions.handleControl(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    gameinstructions.render();
                }
                
                else if(ScreenStatus == Net_Work){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        networklist.handleControll(e);
                    }
                    
                    mainBackgroundPic.render(0,0);
                    networklist.render();
                }
                else if (ScreenStatus == Player_Chose){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        playerchoselist.handleControll(e);
                    }
                    
                    mainBackgroundPic.render(0,0);
                    playerchoselist.render();
                }
                else if(ScreenStatus == Server_Set){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        serverset.handleControll(e);
                    }
                    net = true;
                    mainBackgroundPic.render(0,0);
                    serverset.render();
                    if(!serverset.work){
                        SDL_RenderPresent( gRenderer );
                        serverset.net_work_server();
                    }
                }
                else if (ScreenStatus == Client_Set){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        clientset.handleControll(e);
                    }
                    net = true;
                    mainBackgroundPic.render(0,0);
                    clientset.render();
                }
                
                else if(ScreenStatus == Car_Chose_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        if(keyAvoid(e)){
                            send = true;
                            SDLNet_TCP_Send(remote, &send, sizeof(send));
                            if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
//                                std::cout << "Send Success ...\n";
                            }
                            if( e.type == SDL_QUIT )
                            {
                                quit = true;
                            }
                            carchose.handleControl(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
//                                std::cout << "Receive Success ...\n";
                            }
                            carchose.handleControl(e_remote);
                        }
                        else{
                            break;
                        }
                    }
                    mainBackgroundPic.render(0, 0);
                    carchose.render();
                }
                
                else if(ScreenStatus == Weapon_Chose_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        if(keyAvoid(e)){
                            send = true;
                            SDLNet_TCP_Send(remote, &send, sizeof(send));
                            if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
                                //                                std::cout << "Send Success ...\n";
                            }
                            if( e.type == SDL_QUIT )
                            {
                                quit = true;
                            }
                            weaponchose.handelControll(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
                                //                                std::cout << "Receive Success ...\n";
                            }
                            weaponchose.handelControll(e_remote);
                        }
                        else{
                            break;
                        }
                    }
                    mainBackgroundPic.render(0, 0);
                    weaponchose.render();
                }
                
                else if(ScreenStatus == Car_Modified_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        if(keyAvoid(e)){
                            send = true;
                            SDLNet_TCP_Send(remote, &send, sizeof(send));
                            if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
//                                std::cout << "Send Success ...\n";
                            }
                            if( e.type == SDL_QUIT )
                            {
                                quit = true;
                            }
                            carmodified.handleControl(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
//                                std::cout << "Receive Success ...\n";
                            }
                            carmodified.handleControl(e_remote);
                        }
                        else{
                            break;
                        }
                    }
                    mainBackgroundPic.render(0, 0);
                    carmodified.render();
                }
                
                else if(ScreenStatus == Map_Chose_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        if(keyAvoid(e)){
                            send = true;
                            SDLNet_TCP_Send(remote, &send, sizeof(send));
                            if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
//                                std::cout << "Send Success ...\n";
                            }
                            if( e.type == SDL_QUIT )
                            {
                                quit = true;
                            }
                            mapchose.handelControll(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
//                                std::cout << "Receive Success ...\n";
                            }
                            mapchose.handelControll(e_remote);
                        }
                        else{
                            break;
                        }
                    }
                    mainBackgroundPic.render(0, 0);
                    mapchose.render();
                }
                
                else if(ScreenStatus == Game_Start_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        if(keyPlayingAvoid(e)){
                            send = true;
                            SDLNet_TCP_Send(remote, &send, sizeof(send));
                            if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
                                std::cout << "Send Success ...\n";
                            }
                        }
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        if(car.die() || car2.die()){
                                gameplaying.handleGameOver(e);
                        }
                        else{
                            //Handle texture
                            if(selfPlayer == 0)
                                car.handelEvent(e);
                            if(selfPlayer == 1)
                                car2.handelEvent(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    b2Vec2 sendPosition;
                    float32 sendAngle;
                    if(selfPlayer == 0){
                        sendPosition = car.car->GetPosition();
                        sendAngle = car.car->GetAngle();
                    }
                    else if(selfPlayer == 1){
                        sendPosition = car2.car->GetPosition();
                        sendAngle = car2.car->GetAngle();
                    }
                    SDLNet_TCP_Send(remote, &sendPosition, sizeof(sendPosition));
                    SDLNet_TCP_Send(remote, &sendAngle, sizeof(sendAngle));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
//                                std::cout << "Receive Success ...\n";
                            }
                            if(car.die() || car2.die()){
                                gameplaying.handleGameOver(e_remote);
                            }
                            else{
                                if(selfPlayer == 1)
                                    car.handelEvent(e_remote);
                                if(selfPlayer == 0)
                                    car2.handelEvent(e_remote);
                            }
                        }
                        else{
                            break;
                        }
                    }
                    
                    b2Vec2 recvPosition;
                    float32 recvAngle;
                    SDLNet_TCP_Recv(remote, &recvPosition, sizeof(recvPosition));
                    SDLNet_TCP_Recv(remote, &recvAngle, sizeof(recvAngle));
                    if(selfPlayer == 0){
                        car2.car->SetTransform(recvPosition, recvAngle);
                    }
                    else if(selfPlayer == 1){
                        car.car->SetTransform(recvPosition, recvAngle);
                    }
                    
                    gameplaying.render();
                    
                    if(car.die() || car2.die()){
                        if(car.life != 0 && car2.life == 0)
                            gameplaying.renderGameOver(1);
                        else if(car.life == 0 && car2.life != 0)
                            gameplaying.renderGameOver(2);
                        else if(car.life == 0 && car2.life == 0)
                            gameplaying.renderGameOver(12);
                    }

                }
                else if(ScreenStatus == Game_Start_net){
                    bool send = false;
                    bool receive = false;
                    while (SDL_PollEvent(&e) != 0) {
                        send = true;
                        SDLNet_TCP_Send(remote, &send, sizeof(send));
                        if(SDLNet_TCP_Send(remote, &e, sizeof(e)) == sizeof(e)){
                            //                            std::cout << "Send Success ...\n";
                        }
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        if(car.die() || car2.die()){
                            gameplaying.handleGameOver(e);
                        }
                        else{
                            //Handle texture
                            if(selfPlayer == 0)
                                car.handelEvent(e);
                            if(selfPlayer == 1)
                                car2.handelEvent(e);
                        }
                    }
                    send = false;
                    SDLNet_TCP_Send(remote, &send, sizeof(send));
                    
                    while (1) {
                        SDLNet_TCP_Recv(remote, &receive, sizeof(receive));
                        if(receive){
                            if(SDLNet_TCP_Recv(remote, &e_remote, sizeof(e_remote)) != 0){
                                //                                std::cout << "Receive Success ...\n";
                            }
                            if(car.die() || car2.die()){
                                gameplaying.handleGameOver(e_remote);
                            }
                            else{
                                if(selfPlayer == 1)
                                    car.handelEvent(e_remote);
                                if(selfPlayer == 0)
                                    car2.handelEvent(e_remote);
                            }
                        }
                        else{
                            break;
                        }
                    }
                    gameplaying.render();
                    
                    
                    
                    if(car.die() || car2.die()){
                        if(car.life != 0 && car2.life == 0)
                            gameplaying.renderGameOver(1);
                        else if(car.life == 0 && car2.life != 0)
                            gameplaying.renderGameOver(2);
                        else if(car.life == 0 && car2.life == 0)
                            gameplaying.renderGameOver(12);
                    }
                    
                }
                else if(ScreenStatus == Car_Chose){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        carchose.handleControl(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    carchose.render();
                }
                
                else if(ScreenStatus == Weapon_Chose){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        weaponchose.handelControll(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    weaponchose.render();
                }
                
                else if(ScreenStatus == Skill_Chose){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        carchose.handleControl(e);
                    }
                    mainBackgroundPic.render(0, 0);
                }
                
                else if(ScreenStatus == Car_Modified){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        carmodified.handleControl(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    carmodified.render();
                }
                
                else if(ScreenStatus == Map_Chose){
                    while (SDL_PollEvent(&e) != 0) {
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        mapchose.handelControll(e);
                    }
                    mainBackgroundPic.render(0, 0);
                    mapchose.render();
                }
                
                else if(ScreenStatus == Game_Start){
                    while( SDL_PollEvent( &e ) != 0 )
                    {
                        //User requests quit
                        if( e.type == SDL_QUIT )
                        {
                            quit = true;
                        }
                        if(car.die() || car2.die()){
                            gameplaying.handleGameOver(e);
                        }
                        else{
                            //Handle texture
                            car.handelEvent(e);
                            car2.handelEvent(e);
                        }
                    }
                    gameplaying.render();
                    
                    
                    
                    if(car.die() || car2.die()){
                        if(car.life != 0 && car2.life == 0)
                            gameplaying.renderGameOver(1);
                        else if(car.life == 0 && car2.life != 0)
                            gameplaying.renderGameOver(2);
                        else if(car.life == 0 && car2.life == 0)
                            gameplaying.renderGameOver(12);
                    }
                }

                 
                //Calculate and correct fps, then render to screen
                float avgFPS = countedFrames / ( fpsTimer.getTicks() / 1000.f );
                if( avgFPS > 2000000 )
                {
                    avgFPS = 0;
                }
                //                FPS.setText("Average FPS: " + std::to_string(avgFPS));
                //                FPS.render(620, 720);
                
                //Update screen
                SDL_RenderPresent( gRenderer );
                ++countedFrames;
                
                //If frame finished early
                int frameTicks = capTimer.getTicks();
                if( frameTicks < SCREEN_TICK_PER_FRAME )
                {
                    //Wait remaining time
                    SDL_Delay( SCREEN_TICK_PER_FRAME - frameTicks );
                }
            }
        }
    }
    
    //Free resources and close SDL
    close();
    
    return 0;
}

void Box2D_World(){
    b2BodyDef myBodyDef;
    
    //shape definition
    b2PolygonShape polygonShape;
    polygonShape.SetAsBox(1, 1); //a 2x2 rectangle
    
    //a static body
    myBodyDef.type = b2_staticBody;
    myBodyDef.position.Set(0, 0);
    b2Body* staticBody = myWorld->CreateBody(&myBodyDef);
    
    //fixture definition
    b2FixtureDef myFixtureDef;
    myFixtureDef.shape = &polygonShape;
    myFixtureDef.density = 1;
    //    myFixtureDef.filter.categoryBits = NORMAL_WALL;
    //    myFixtureDef.filter.maskBits = NORMAL_WALL;
    
    //add four walls to the static body
    polygonShape.SetAsBox( 300, 2, b2Vec2(3, 138), 0);//ground
    staticBody->CreateFixture(&myFixtureDef);
    polygonShape.SetAsBox( 300, 2, b2Vec2(3, 4), 0);//ceiling
    staticBody->CreateFixture(&myFixtureDef);
    polygonShape.SetAsBox( 2, 300, b2Vec2(-5, 3), 0);//left wall
    staticBody->CreateFixture(&myFixtureDef);
    polygonShape.SetAsBox( 2, 300, b2Vec2(201, 5), 0);//right wall
    staticBody->CreateFixture(&myFixtureDef);
}

//=========== function Definition ======================================
bool init()
{
    //Initialization flag
    bool success = true;
    
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }
        
        //Create window
        gWindow = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                
                //Initialize SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
                
                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
            }
        }
    }
    
    return success;
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;
    
    
    //Load texture
    
    
    if( !P3.loadFromFile( "content/car/car.png" ) )
    {
        printf( "Failed to load P2 texture image!\n" );
        success = false;
    }
    
    if( !P4.loadFromFile( "content/car/car2.png" ) )
    {
        printf( "Failed to load P2 texture image!\n" );
        success = false;
    }
    
    //Load sound effect
    if(!high.loadSoundFile("content/sound/high.wav")){
        printf("Failed to load high sound effect!\n");
        success = false;
    }
    
    //Load music
    if(!backgroundMusic.loadSoundFile("content/backGroundMusic.wav")){
        printf("Failed to load fuck music!\n");
        success = false;
    }
    
    //Load background texture
    if( !mainBackgroundPic.loadFromFile( "content/steel.png" ) )
    {
        printf( "Failed to load background texture image!\n" );
        success = false;
    }
    
    
    return success;
}



void close()
{
    
    //Free loaded images
    gBackgroundTexture.free();
    
    //Free loaded soundEffects
    high.free();
    
    //Free loaded musics
    backgroundMusic.free();
    
    
    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;
    
    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

struct Axis{
    double x;
    double y;
};

struct Corner{
    double x;
    double y;
    double m;
    double n;
    void rotated(double h, double k, double d, double h2, double k2);
    void project(Axis axis);
};

void Corner::rotated(double h, double k, double d, double h2, double k2){
    x=0;y=0;
    x = (h-h2)*cos(d) + (k-k2)*sin(d) + h2;
    y = -(h-h2)*sin(d) + (k-k2)*cos(d) + k2;
}

void Corner::project(Axis axis){
    m=0;n=0;
    m = (x*axis.x+y*axis.y)*axis.x;
    n = (x*axis.x+y*axis.y)*axis.y;
}

//=========== class StaticTexture ======================================
StaticTexture::StaticTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

StaticTexture::~StaticTexture()
{
    //Deallocate
    free();
}

DynamicTexture::DynamicTexture(const DynamicTexture& a){
    this->mTexture = a.mTexture;
}

bool StaticTexture::loadFromFile( std::string path )
{
    //Get rid of preexisting texture
    free();
    
    //The final texture
    SDL_Texture* newTexture = NULL;
    
    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        /*Color key image
         SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
         */
        
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }
        
        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }
    
    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

void StaticTexture::free()
{
    //Free texture if it exists
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void StaticTexture::setAlpha(uint8 alpha){
    SDL_SetTextureBlendMode( mTexture, SDL_BLENDMODE_BLEND );
    SDL_SetTextureAlphaMod(mTexture, alpha);
}

void StaticTexture::render()
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { mPosX, mPosY, mWidth, mHeight };
    SDL_RenderCopy( gRenderer, mTexture, NULL, &renderQuad );
}

void StaticTexture::render( int x, int y )
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    SDL_RenderCopy( gRenderer, mTexture, NULL, &renderQuad );
}

int StaticTexture::getWidth()
{
    return mWidth;
}

int StaticTexture::getHeight()
{
    return mHeight;
}

int StaticTexture::getPosX(){
    return mPosX;
}

int StaticTexture::getPosY(){
    return mPosY;
}

bool StaticTexture::empty(){
    if(mTexture == NULL){
        return true;
    }
    return false;
}

//=========== class DynamicTexture ======================================
DynamicTexture::DynamicTexture()
{
    //Initialize the offsets
    mPosX = 0;
    mPosY = 0;
    
    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

DynamicTexture::DynamicTexture(int x, int y)
{
    //Initialize the offsets
    mPosX = x;
    mPosY = y;
    
    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

void DynamicTexture::handleEvent( SDL_Event& e )
{
    //If a key was pressed
    if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY -= DynamicTexture_VEL; break;
            case SDLK_DOWN: mVelY += DynamicTexture_VEL; break;
            case SDLK_LEFT: mVelX -= DynamicTexture_VEL; break;
            case SDLK_RIGHT: mVelX += DynamicTexture_VEL; break;
        }
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY += DynamicTexture_VEL; break;
            case SDLK_DOWN: mVelY -= DynamicTexture_VEL; break;
            case SDLK_LEFT: mVelX += DynamicTexture_VEL; break;
            case SDLK_RIGHT: mVelX -= DynamicTexture_VEL; break;
        }
    }
}
/*
 void DynamicTexture::move( std::vector<SDL_Rect>& otherColliders )
 {
 //Move the DynamicTexture left or right
 mPosX += mVelX;
 
 //If the DynamicTexture went too far to the left or right
 if( ( mPosX < 0 ) || ( mPosX + mWidth > SCREEN_WIDTH ) || checkCollision( mColliders, otherColliders ) )
 {
 //Move back
 mPosX -= mVelX;
 }
 
 //Move the DynamicTexture up or down
 mPosY += mVelY;
 
 //If the DynamicTexture went too far up or down
 if( ( mPosY < 0 ) || ( mPosY + mHeight > SCREEN_HEIGHT ) || checkCollision( mColliders, otherColliders ) )
 {
 //Move back
 mPosY -= mVelY;
 }
 }*/

void DynamicTexture::render()
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { mPosX, mPosY, mWidth, mHeight };
    SDL_RenderCopy( gRenderer, mTexture, NULL, &renderQuad );
}

void DynamicTexture::render(int x, int y, double angle){
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    
    //Render to screen
    SDL_RenderCopyEx( gRenderer, mTexture, NULL, &renderQuad, -angle, NULL, SDL_FLIP_NONE );
}

//========= class SoundEffect ==========================================
SoundEffect::SoundEffect(){
    this->gEffect = NULL;
}

bool SoundEffect::loadSoundFile(std::string path){
    this->gEffect = Mix_LoadWAV( path.c_str() );
    if( this->gEffect == NULL )
    {
        printf( "Failed to load sound effect from : %s ! SDL_mixer Error: %s\n", path.c_str(), Mix_GetError() );
        return false;
    }
    return true;
}

void SoundEffect::handleEvent(SDL_Event &e, SDL_Scancode key){
    if(e.type == SDL_KEYDOWN){
        if(e.key.keysym.scancode == key){
            Mix_PlayChannel(soundEffectChannel, this->gEffect, 0);
        }
    }
}

void SoundEffect::play(){
    Mix_PlayChannel(soundEffectChannel, this->gEffect, 0);
}

void SoundEffect::free(){
    Mix_FreeChunk(this->gEffect);
    this->gEffect = NULL;
}

//========== class Music ============================================
Music::Music(){
    this->gMusic = NULL;
}

bool Music::loadSoundFile(std::string path){
    this->gMusic = Mix_LoadMUS(path.c_str());
    if(this->gMusic == NULL){
        printf( "Failed to load scratch sound effect from : %s ! SDL_mixer Error: %s\n", path.c_str(), Mix_GetError() );
        return false;
    }
    return true;
}

void Music::handleEvent(SDL_Event &e, SDL_Scancode Play_Pause, SDL_Scancode Stop){
    if(e.type == SDL_KEYDOWN){
        if(e.key.keysym.scancode == Play_Pause){
            //If there is no music playing
            if(Mix_PlayingMusic() == 0){
                //Play the music
                Mix_PlayMusic(this->gMusic, -1);
                
            }
            //If music is being played
            else{
                //If the music is paused
                if(Mix_PausedMusic() == 1){
                    //Resume the music
                    Mix_ResumeMusic();
                }
                //If the music is playing
                else{
                    //Pause the music
                    Mix_PauseMusic();
                }
            }
        }
        if(e.key.keysym.scancode == Stop){
            //Stop the music
            Mix_HaltMusic();
        }
    }
}

void Music::play(){
    Mix_PlayMusic(this->gMusic, -1);
}

void Music::free(){
    Mix_FreeMusic(this->gMusic);
    this->gMusic = NULL;
}

//============= class Text ======================================
Text::Text(){
    font = TTF_OpenFont( "content/font/PressStartTwoP/PressStartTwoP.ttf", 18 );
    this->color = {255,255,255,255};
    this->text.assign("hello!!");
}

void Text::setText(std::string text){
    this->text.assign(text);
    this->loadFromRenderedText();
}

void Text::setColor(SDL_Color color){
    this->color = color;
    this->loadFromRenderedText();
}

void Text::setFont(TTF_Font *font){
    this->font = font;
    this->loadFromRenderedText();
}

bool Text::loadFromRenderedText(){
    //Get rid of preexisting texture
    this->free();
    
    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( this->font, this->text.c_str(), this->color );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        this->mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( this->mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            this->mWidth = textSurface->w;
            this->mHeight = textSurface->h;
        }
        
        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    
    if(this->mTexture == NULL){
        printf( "Failed to render text texture!\n" );
    }
    
    //Return success
    return mTexture != NULL;
}

//========= class MainList =======================================
MainList::MainList(){
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    ptr = 0;
    ptrIMG.loadFromFile("content/ptr.png");
    font = TTF_OpenFont( "content/font/PressStartTwoP/PressStartTwoP.ttf", 40 );
    for(int i = 0; i < 3; i++){
        choiceText[i].setFont(font);
        choiceText[i].setColor({255,255,255,255});
        choiceText[i].setText(choice[i]);
    }
}

void MainList::render(){
    choiceText[0].setColor({255,255,255,255});
    choiceText[1].setColor({255,255,255,255});
    choiceText[2].setColor({255,255,255,255});
    choiceText[ptr].setColor({201, 198, 28,255});
    ptrIMG.render(110, 360 + (ptr*80));
    choiceText[0].render(250, 420);
    choiceText[1].render(250, 500);
    choiceText[2].render(250, 580);
}

void MainList::controlHandle(SDL_Event &e){
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(ptr > 0){
                    ptr--;
                    beep2.play();
                }
                break;
            case SDLK_DOWN:
                if(ptr < 2){
                    ptr++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                if(ptr == 0)
                    if(net){
                        ScreenStatus = Car_Chose_net;
                    }
                    else{
                        ScreenStatus = Net_Work;
                    }
                else if(ptr == 1)
                    ScreenStatus = Game_Setting;
                else if(ptr == 2)
                    ScreenStatus = Game_Instruction;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

//========= class GameSettingList ==========================
GameSettingList::GameSettingList(){
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    font = TTF_OpenFont( "content/font/PressStartTwoP/PressStartTwoP.ttf", 28 );
    for(int i = 0 ; i < 3; i++){
        settingText[i].setFont(this->font);
        settingText[i].setText(setting[i]);
    }
    bar.loadFromFile("content/bar.png");
    bar2.loadFromFile("content/bar.png");
    cube.loadFromFile("content/cube.png");
    cube2.loadFromFile("content/cube.png");
    muteCube.loadFromFile("content/mutecube.png");
    muteCube2.loadFromFile("content/mutecube.png");
}

void GameSettingList::handleControl(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_DOWN:
                if(mainPtr < 2){
                    mainPtr++;
                    beep2.play();
                }
                break;
            case SDLK_UP:
                if(mainPtr > 0){
                    mainPtr--;
                    beep2.play();
                }
                break;
            case SDLK_LEFT:
                if(mainPtr == 0){
                    if(soundEffectVolumn > 0){
                        soundEffectVolumn -= 12;
                        soundEffectPtr--;
                        Mix_Volume(soundEffectChannel, soundEffectVolumn);
                        beep2.play();
                    }
                }
                else if(mainPtr == 1){
                    if(backgroundMusicVolumn > 0){
                        backgroundMusicVolumn -= 12;
                        backgroundMusicVolumnPtr--;
                        Mix_VolumeMusic(backgroundMusicVolumn);
                        beep2.play();
                    }
                }
                break;
            case SDLK_RIGHT:
                if(mainPtr == 0){
                    if(soundEffectVolumn < 132){
                        soundEffectVolumn += 12;
                        soundEffectPtr++;
                        Mix_Volume(soundEffectChannel, soundEffectVolumn);
                        beep2.play();
                    }
                }
                else if(mainPtr == 1){
                    if(backgroundMusicVolumn < 132){
                        backgroundMusicVolumn += 12;
                        backgroundMusicVolumnPtr++;
                        Mix_VolumeMusic(backgroundMusicVolumn);
                        beep2.play();
                    }
                }
                break;
            case SDLK_RETURN:
                if(mainPtr == 2){
                    ScreenStatus = Main_Screen;
                    beep8.play();
                }
                break;
            case SDLK_BACKSPACE:
                ScreenStatus = Main_Screen;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void GameSettingList::render(){
    settingText[0].setColor({255,255,255,255});
    settingText[1].setColor({255,255,255,255});
    settingText[2].setColor({255,255,255,255});
    settingText[mainPtr].setColor({201, 198, 28,255});
    bar.render(90,255);
    bar2.render(90,450);
    settingText[0].render(90,200);
    settingText[1].render(90,400);
    settingText[2].render(90,600);
    if(soundEffectPtr != 0){
        cube.render(75 + (soundEffectPtr*70), 230);
    }
    else{
        muteCube.render(70, 230);
    }
    
    if(backgroundMusicVolumnPtr != 0){
        cube2.render(75 + (backgroundMusicVolumnPtr*70), 430);
    }
    else{
        muteCube2.render(70, 430);
    }
}

//======== class GameInstructions =============================
GameInstructions::GameInstructions(){
    beep8.loadSoundFile("content/sound/Beep8.wav");
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 28 );
    back.setText("Back");
    back.setFont(font);
    back.setColor({201, 198, 28,255});
}

void GameInstructions::handleControl(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_RETURN:
                if(ptr == 0){
                    ScreenStatus = Main_Screen;
                    beep8.play();
                }
                break;
            case SDLK_BACKSPACE:
                ScreenStatus = Main_Screen;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void GameInstructions::render(){
    back.render(90,600);
}

//=========== class CarModified ===========================
CarModified::CarModified(){
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    font = TTF_OpenFont( "content/font/PressStartTwoP/PressStartTwoP.ttf", 26 );
    for(int i = 0; i < 2; i++){
        playerText[i].setFont(font);
        playerText[i].setText(player[i]);
    }
    for(int i = 0; i < 3; i++){
        modified[i].setFont(font);
        modified[i].setText(modifiedText[i]);
    }
    for(int i = 0; i < 4; i++){
        tire[i].loadFromFile("content/carmodified/tire"+std::to_string(i)+".png");
        steer[i].loadFromFile("content/carmodified/steer"+std::to_string(i)+".png");
        engine[i].loadFromFile("content/carmodified/engine"+std::to_string(i)+".png");
    }
    baclground.loadFromFile("content/steellll.png");
    cube.loadFromFile("content/carmodified/bar.png");
    noCube.loadFromFile("content/carmodified/nullbar.png");
}

void CarModified::handleControl(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                switch (second_ptr1) {
                    case 1:
                        if(engine_ptr1 > 0){
                            engine_ptr1--;
                            beep2.play();
                        }
                        break;
                    case 2:
                        if(steer_ptr1 > 0){
                            steer_ptr1--;
                            beep2.play();
                        }
                        break;
                    case 3:
                        if(tire_ptr1 > 0){
                            tire_ptr1--;
                            beep2.play();
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDLK_w:
                switch (second_ptr2) {
                    case 1:
                        if(engine_ptr2 > 0){
                            engine_ptr2--;
                            beep2.play();
                        }
                        break;
                    case 2:
                        if(steer_ptr2 > 0){
                            steer_ptr2--;
                            beep2.play();
                        }
                        break;
                    case 3:
                        if(tire_ptr2 > 0){
                            tire_ptr2--;
                            beep2.play();
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDLK_DOWN:
                switch (second_ptr1) {
                    case 1:
                        if(engine_ptr1 < 3){
                            engine_ptr1++;
                            beep2.play();
                        }
                        break;
                    case 2:
                        if(steer_ptr1 < 3){
                            steer_ptr1++;
                            beep2.play();
                        }
                        break;
                    case 3:
                        if(tire_ptr1 < 3){
                            tire_ptr1++;
                            beep2.play();
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDLK_s:
                switch (second_ptr2) {
                    case 1:
                        if(engine_ptr2 < 3){
                            engine_ptr2++;
                            beep2.play();
                        }
                        break;
                    case 2:
                        if(steer_ptr2 < 3){
                            steer_ptr2++;
                            beep2.play();
                        }
                        break;
                    case 3:
                        if(tire_ptr2 < 3){
                            tire_ptr2++;
                            beep2.play();
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDLK_LEFT:
                if(second_ptr1 > 1){
                    second_ptr1--;
                    beep2.play();
                }
                break;
            case SDLK_a:
                if(second_ptr2 > 1){
                    second_ptr2--;
                    beep2.play();
                }
                break;
            case SDLK_RIGHT:
                if(second_ptr1 < 3){
                    second_ptr1++;
                    beep2.play();
                }
                break;
            case SDLK_d:
                if(second_ptr2 < 3){
                    second_ptr2++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                car.setPerformance(engine_ptr1, steer_ptr1, tire_ptr1);
                car2.setPerformance(engine_ptr2, steer_ptr2, tire_ptr2);
                if(net)
                    ScreenStatus = Map_Chose_net;
                else
                    ScreenStatus = Map_Chose;
                beep8.play();
                break;
            case SDLK_BACKSPACE:
                if(net){
                    ScreenStatus = Weapon_Chose_net;
                }
                else{
                    ScreenStatus = Weapon_Chose;
                }
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void CarModified::renderBar(){
    switch (engine_ptr1) {
        case 0:
            cube.render(220, 340);
            for(int i = 1; i < 6; i++)
                noCube.render(220+i*40,340);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(220+i*40,340);
            for(int i = 3; i < 6; i++)
                noCube.render(220+i*40,340);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(220+i*40,340);
            for(int i = 5; i < 6; i++)
                noCube.render(220+i*40,340);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(220+i*40,340);
            break;
        default:
            break;
    }
    switch (steer_ptr1) {
        case 0:
            cube.render(480, 340);
            for(int i = 1; i < 6; i++)
                noCube.render(480+i*40,340);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(480+i*40,340);
            for(int i = 3; i < 6; i++)
                noCube.render(480+i*40,340);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(480+i*40,340);
            for(int i = 5; i < 6; i++)
                noCube.render(480+i*40,340);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(480+i*40,340);
            break;
        default:
            break;
    }
    switch (tire_ptr1) {
        case 0:
            cube.render(740, 340);
            for(int i = 1; i < 6; i++)
                noCube.render(740+i*40,340);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(740+i*40,340);
            for(int i = 3; i < 6; i++)
                noCube.render(740+i*40,340);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(740+i*40,340);
            for(int i = 5; i < 6; i++)
                noCube.render(740+i*40,340);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(740+i*40,340);
            break;
        default:
            break;
    }
    switch (engine_ptr2) {
        case 0:
            cube.render(220, 700);
            for(int i = 1; i < 6; i++)
                noCube.render(220+i*40,700);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(220+i*40,700);
            for(int i = 3; i < 6; i++)
                noCube.render(220+i*40,700);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(220+i*40,700);
            for(int i = 5; i < 6; i++)
                noCube.render(220+i*40,700);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(220+i*40,700);
            break;
        default:
            break;
    }
    switch (steer_ptr2) {
        case 0:
            cube.render(480, 700);
            for(int i = 1; i < 6; i++)
                noCube.render(480+i*40,700);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(480+i*40,700);
            for(int i = 3; i < 6; i++)
                noCube.render(480+i*40,700);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(480+i*40,700);
            for(int i = 5; i < 6; i++)
                noCube.render(480+i*40,700);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(480+i*40,700);
            break;
        default:
            break;
    }
    switch (tire_ptr2) {
        case 0:
            cube.render(740, 700);
            for(int i = 1; i < 6; i++)
                noCube.render(740+i*40,700);
            break;
        case 1:
            for(int i = 0; i < 3; i++)
                cube.render(740+i*40,700);
            for(int i = 3; i < 6; i++)
                noCube.render(740+i*40,700);
            break;
        case 2:
            for(int i = 0; i < 5; i++)
                cube.render(740+i*40,700);
            for(int i = 5; i < 6; i++)
                noCube.render(740+i*40,700);
            break;
        case 3:
            for(int i = 0; i < 6; i++)
                cube.render(740+i*40,700);
            break;
        default:
            break;
    }
}

void CarModified::render(){
    baclground.render(0,0);
    
    renderBar();
    
    playerText[0].setColor({255,255,255,255});
    playerText[1].setColor({255,255,255,255});
    
    playerText[0].render(25,150);
    playerText[1].render(25,510);
    
    modified[0].setColor({255,255,255,255});
    modified[1].setColor({255,255,255,255});
    modified[2].setColor({255,255,255,255});
    modified[second_ptr1-1].setColor({201, 198, 28,255});
    
    modified[0].render(265, 300);
    modified[1].render(495, 300);
    modified[2].render(800, 300);
    
    modified[0].setColor({255,255,255,255});
    modified[1].setColor({255,255,255,255});
    modified[2].setColor({255,255,255,255});
    modified[second_ptr2-1].setColor({201, 198, 28,255});
    
    modified[0].render(265, 669);
    modified[1].render(495, 669);
    modified[2].render(800, 669);
    engine[engine_ptr1].render(279,46);
    steer[steer_ptr1].render(522,51);
    tire[tire_ptr1].render(766,74);
    engine[engine_ptr2].render(279,415);
    steer[steer_ptr2].render(522,420);
    tire[tire_ptr2].render(766,453);
}

//========== class LTimer ==================================
LTimer::LTimer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;
    
    mPaused = false;
    mStarted = false;
}

void LTimer::start()
{
    //Start the timer
    mStarted = true;
    
    //Unpause the timer
    mPaused = false;
    
    //Get the current clock time
    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;
    
    //Unpause the timer
    mPaused = false;
    
    //Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;
        
        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;
        
        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;
        
        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks()
{
    //The actual timer time
    Uint32 time = 0;
    
    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }
    
    return time;
}

bool LTimer::isStarted()
{
    //Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
    //Timer is running and paused
    return mPaused && mStarted;
}
bool keyPlayingAvoid(SDL_Event &e){
    if(net){
        if(e.key.keysym.sym == SDLK_w ||
           e.key.keysym.sym == SDLK_a ||
           e.key.keysym.sym == SDLK_s ||
           e.key.keysym.sym == SDLK_d ||
           e.key.keysym.sym == SDLK_UP ||
           e.key.keysym.sym == SDLK_LEFT ||
           e.key.keysym.sym == SDLK_RIGHT ||
           e.key.keysym.sym == SDLK_DOWN ){
            return false;
        }
    }
    return true;
}

bool keyAvoid(SDL_Event &e){
    if(net && selfPlayer == 0){
        if (e.key.keysym.sym == SDLK_w ||
            e.key.keysym.sym == SDLK_a ||
            e.key.keysym.sym == SDLK_s ||
            e.key.keysym.sym == SDLK_d ) {
            return false;
        }
    }
    if(net && selfPlayer == 1){
        if (e.key.keysym.sym == SDLK_UP ||
            e.key.keysym.sym == SDLK_LEFT ||
            e.key.keysym.sym == SDLK_RIGHT ||
            e.key.keysym.sym == SDLK_DOWN ||
            e.key.keysym.sym == SDLK_RETURN) {
            return false;
        }
    }
    return true;
}

void GamePlaying::render(){
    
    //Scroll background
    if(mapIndex != 2 && mapIndex != 4){
        scrollingOffset -= 2;
    }
    if( scrollingOffset < -map[mapIndex].getWidth() )
    {
        scrollingOffset = 0;
    }
    map[mapIndex].render(scrollingOffset,110);
    map[mapIndex].render(scrollingOffset + map[mapIndex].getWidth(),110);
    hpBarP1.render(20, 10);
    hpBarP1.render(620, 10);
    if(mapIndex == 0){
        b2Vec2 pos1 = car.car->GetPosition();
        b2Vec2 pos2 = car2.car->GetPosition();
        if(pos1.x < 30 || pos1.x > 160 || pos1.y < 30 || pos1.y > 130){
            car.increasFriction(4000);
        }
        else{
            car.friction();
        }
        if(pos2.x < 30 || pos2.x > 160 || pos2.y < 30 || pos2.y > 130){
            car2.increasFriction(4000);
        }
        else{
            car2.friction();
        }
    }
    else if(mapIndex == 1){
        car.friction();
        car2.friction();
        if(this->firstFlip){
            firstTick = SDL_GetTicks();
            car.car->SetLinearVelocity(b2Vec2(0,1000));
            car2.car->SetLinearVelocity(b2Vec2(0,1000));
            firstFlip = false;
        }
        if(SDL_GetTicks() - firstTick > 1300){
            firstTick = SDL_GetTicks();
            if(flip){
                car.car->SetLinearVelocity(b2Vec2(0,-2000));
                car2.car->SetLinearVelocity(b2Vec2(0,-2000));
            }
            else{
                car.car->SetLinearVelocity(b2Vec2(0,2000));
                car2.car->SetLinearVelocity(b2Vec2(0,2000));
            }
            flip = !flip;
        }
    }
    else if(mapIndex == 2){
        car.friction();
        car2.friction();
        if(firstFlip){
            srand( (int)time(NULL));
            int x =  rand()%100;
            if(x%4 == 0){
                myWorld->SetGravity({0,100});
            }
            else if(x%4 == 1){
                myWorld->SetGravity({0,-100});
            }
            else if(x%4 == 2){
                myWorld->SetGravity({100,0});
            }
            else if(x%4 == 3){
                myWorld->SetGravity({-100,0});
            }
            firstFlip = false;
        }
    }
    
    if(mapIndex == 4){
        srand( (int)time(NULL));
        int time = rand() % 6000 + 2000;
        if(this->firstFlip){
            firstTick = SDL_GetTicks();
            firstFlip = false;
        }
        if(SDL_GetTicks() - firstTick > time){
            firstTick = SDL_GetTicks();
            int force = rand()%1000000+90000000;
            double angle = rand() % 360;
            car.car->ApplyForce({static_cast<float32>(force*cos(angle)), static_cast<float32>(force*sin(angle))}, car.car->GetWorldCenter(), true);
            car2.car->ApplyForce({static_cast<float32>(force*cos(angle)), static_cast<float32>(force*sin(angle))}, car2.car->GetWorldCenter(), true);
        }
    }
    
    
    //Render car to the screen
    b2Vec2 pos = car.car->GetPosition();
    pos.x -= static_cast<double>(carIMG[p1_carIndex].getWidth())/20;
    pos.y += static_cast<double>(carIMG[p1_carIndex].getHeight())/20;
    
    if(p1_weaponIndex == 0){
        weapon[0].render(pos.x*5 - weapon[0].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + carIMG[p1_carIndex].getHeight()/2*sinf(car.car->GetAngle()), pos.y*5 - weapon[0].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - carIMG[p1_carIndex].getHeight()/2*cosf(car.car->GetAngle()), p1_angle);
        p1_angle += 10;
    }
    else if(p1_weaponIndex == 1){
        weapon[1].render(pos.x*5 - weapon[1].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2, pos.y*5 - weapon[1].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2, p1_angle);
        p1_angle += 10;
    }
    else if(p1_weaponIndex == 2){
        weapon[2].render(pos.x*5 - weapon[2].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + (carIMG[p1_carIndex].getHeight()/2 + 28)*sinf(car.car->GetAngle()), pos.y*5 - weapon[2].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - (carIMG[p1_carIndex].getHeight()/2 + 28)*cosf(car.car->GetAngle()), -car.car->GetAngle()*RADTODEG);
    }
    else if(p1_weaponIndex == 3){
        weapon[3].render(pos.x*5 - weapon[3].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + (carIMG[p1_carIndex].getHeight()/2 + 10)*sinf(car.car->GetAngle()), pos.y*5 - weapon[3].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - (carIMG[p1_carIndex].getHeight()/2 + 10)*cosf(car.car->GetAngle()), -car.car->GetAngle()*RADTODEG);
    }
    else if(p1_weaponIndex == 4){
        weapon[4].render(pos.x*5 - weapon[4].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 - (carIMG[p1_carIndex].getHeight()/2 + 20)*sinf(car.car->GetAngle()), pos.y*5 - weapon[4].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 + (carIMG[p1_carIndex].getHeight()/2 + 20)*cosf(car.car->GetAngle()), -car.car->GetAngle()*RADTODEG);
    }

    car.move();
    carIMG[p1_carIndex].render(pos.x*5, pos.y*5, -car.car->GetAngle()*RADTODEG);
    
    b2Vec2 pos2 = car2.car->GetPosition();
    pos2.x -= static_cast<double>(carIMG[p2_carIndex].getWidth())/20;
    pos2.y += static_cast<double>(carIMG[p2_carIndex].getHeight())/20;
    
    if(p2_weaponIndex == 0){
        weapon[0].render(pos2.x*5 - weapon[0].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + carIMG[p1_carIndex].getHeight()/2*sinf(car2.car->GetAngle()), pos2.y*5 - weapon[0].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - carIMG[p1_carIndex].getHeight()/2*cosf(car2.car->GetAngle()), p1_angle);
        p1_angle += 10;
    }
    else if(p2_weaponIndex == 1){
        weapon[1].render(pos2.x*5 - weapon[1].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2, pos2.y*5 - weapon[1].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2, p1_angle);
        p1_angle += 10;
    }
    else if(p2_weaponIndex == 2){
        weapon[2].render(pos2.x*5 - weapon[2].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + (carIMG[p1_carIndex].getHeight()/2 + 28)*sinf(car2.car->GetAngle()), pos2.y*5 - weapon[2].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - (carIMG[p1_carIndex].getHeight()/2 + 28)*cosf(car2.car->GetAngle()), -car2.car->GetAngle()*RADTODEG);
    }
    else if(p2_weaponIndex == 3){
        weapon[3].render(pos2.x*5 - weapon[3].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 + (carIMG[p1_carIndex].getHeight()/2 + 10)*sinf(car2.car->GetAngle()), pos2.y*5 - weapon[3].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 - (carIMG[p1_carIndex].getHeight()/2 + 10)*cosf(car2.car->GetAngle()), -car2.car->GetAngle()*RADTODEG);
    }
    else if(p2_weaponIndex == 4){
        weapon[4].render(pos2.x*5 - weapon[4].getWidth()/2 + carIMG[p1_carIndex].getWidth()/2 - (carIMG[p1_carIndex].getHeight()/2 + 20)*sinf(car2.car->GetAngle()),
                         pos2.y*5 - weapon[4].getHeight()/2 + carIMG[p1_carIndex].getHeight()/2 + (carIMG[p1_carIndex].getHeight()/2 + 20)*cosf(car2.car->GetAngle()), -car2.car->GetAngle()*RADTODEG);
    }
    
    car2.move();
    carIMG[p2_carIndex].render(pos2.x*5, pos2.y*5, -car2.car->GetAngle()*RADTODEG);
    
    //Life Bar
    car.renderLife(&car2);
    car2.renderLife(&car);
    
    car.fireTimer();
    car2.fireTimer();
    car.fireRender();
    car2.fireRender();
    car.defenseRender();
    car2.defenseRender();
    
    //count Down
    int timeLIMIT = 60;
    int currentTime = (SDL_GetTicks() - gametime)/1000;
    int remainTime = (timeLIMIT - currentTime < 0 ? 0 : timeLIMIT - currentTime);
    countDown.setFont(font);
    if (remainTime < 20) {
        countDown.setColor({237, 26, 57, 255});
        if((SDL_GetTicks() - gametime)%1000 < 500){
            countDown.setFont(fontBig);
        }
    }
    countDown.setText(std::to_string(remainTime));
    countDown.render(452,13);
    
    if(remainTime <= 0){
        if(car.life > car2.life){
            car2.life = 0;
        }
        if(car.life < car2.life){
            car.life = 0;
        }
        else{
            car.life = 0;
            car2.life = 0;
        }
    }
    
    if(mapIndex == 3){
        car.friction();
        car2.friction();
        
        firstTick = SDL_GetTicks();
        int alpha = (sin(static_cast<double>(firstTick)/450)+1)*255/2;
        
        if(alpha > 255){
            alpha = 255;
        }
        
        sandIMG.setAlpha(alpha);
        sandIMG.render(0, 110);
//        sand.x = 0;
//        sand.w = 1000;
//        sand.h = 750;
//        sand.y = 0;
//        SDL_SetRenderDrawColor(gRenderer, 148, 97, 49, alpha);
//        SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
//        SDL_RenderFillRect(gRenderer, &this->sand);
//        SDL_RenderDrawRect(gRenderer, &this->sand);
    }
}

GamePlaying::GamePlaying(){
    for(int i = 0; i < 5; i++){
        weapon[i].loadFromFile("content/weapon/weapon" + std::to_string(i) + "g.png");
    }
    
    sandIMG.loadFromFile("content/map/3.png");
    
    for(int i = 0; i < 5; i++){
        map[i].loadFromFile("content/map/" + std::to_string(i) + ".png");
    }
    
    for(int i = 0; i < 10; i++){
        carIMG[i].loadFromFile("content/car/c" + std::to_string(i) + "g.png");
    }
    
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    fontBig = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 60);
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    hpBarP1.loadFromFile("content/hpBar.png");
    gameover[0].setFont(this->font);
    gameover[0].setColor({201, 198, 28,255});
    gameover[0].setText("... GAME OVER ...");
    
    gameover[1].setFont(this->font);
    gameover[1].setColor({201, 198, 28,255});
    gameover[1].setText("Press Enter to Restart");
    
    gameover[2].setFont(this->font);
    gameover[2].setColor({201, 198, 28,255});
    
    countDown.setFont(font);
    countDown.setColor({255,255,255,255});
}

void GamePlaying::handleGameOver(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        if(e.key.keysym.sym == SDLK_RETURN){
            if(net){
                ScreenStatus = Car_Chose_net;
            }
            else{
                ScreenStatus = Car_Chose;
            }
            beep8.play();
            myWorld->SetGravity(b2Vec2(0, 0));
            this->Init();
        }
    }
}

void GamePlaying::renderGameOver(int winner){
    car.mVel = 0;
    car2.mVel = 0;
    car.mvelR = 0;
    car2.mvelR = 0;
    gameover[0].render(140,100);
    gameover[1].render(80,200);
    
    gameover[2].setText("Player " + std::to_string(winner) + " Win !!!!!");
    gameover[2].render(100,300);
}

void GameInit(){
    car.life = 100;
    car2.life = 100;
    car.car -> SetTransform(b2Vec2(150, 50), 0);
    car2.car -> SetTransform(b2Vec2(50, 50), 0);
    car.mVel = 0;
    car2.mVel = 0;
    car.mvelR = 0;
    car2.mvelR = 0;
    
    car.recoverCount = 0;
    car.recoverTool = 0;
    car.recoverFlag = true;
    car.defenseTool = true;
    car.defenseCount = 0;
    car.defenseFlag = false;
    car.renderDefense = false;
    car.fireCount = 0;
    car.fireTool = true;
    car.fireFlag = false;
    
    car2.recoverCount = 0;
    car2.recoverTool = 0;
    car2.recoverFlag = true;
    car2.defenseTool = true;
    car2.defenseCount = 0;
    car2.defenseFlag = false;
    car2.renderDefense = false;
    car2.fireCount = 0;
    car2.fireTool = true;
    car2.fireFlag = false;
    
}

MapChose::MapChose(){
    NameFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    InfoFont = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 30);
    for(int i = 0; i < 5; i++){
        mapthumbnail[i].loadFromFile("content/map/" + std::to_string(i) + "t.png");
        mapbar[i].loadFromFile("content/map/" + std::to_string(i) + "s.png");
        mapbarHighlight[i].loadFromFile("content/map/" + std::to_string(i) + "h.png");
        info[i].loadFromFile("content/map/" + std::to_string(i) + "f.png");
        mapName[i].setFont(NameFont);
        mapName[i].setColor({255,255,255,255});
        mapName[i].setText(napNameText[i]);
        mapInfo[i].setFont(InfoFont);
    }
    beep2.loadSoundFile("content/sound/Beep2.wav");
    beep8.loadSoundFile("content/sound/Beep8.wav");
    mapInfo[0].setText("Seemingly peaceful grassland");
    engineStart.loadSoundFile("content/sound/start.wav");
}

void MapChose::handelControll(SDL_Event &e){
    if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                if(map_ptr > 0){
                    map_ptr--;
                    beep2.play();
                }
                break;
            case SDLK_DOWN:
                if(map_ptr < 4){
                    map_ptr++;
                    beep2.play();
                }
                break;
            case SDLK_RETURN:
                mapIndex = map_ptr;
                if(net)
                    ScreenStatus = Game_Start_net;
                else
                    ScreenStatus = Game_Start;
                GameInit();
                car.life = 100;
                car2.life = 100;
                gametime = SDL_GetTicks();
                car.setWeapon(p1_weaponIndex);
                car2.setWeapon(p2_weaponIndex);
                engineStart.play();
                break;
            case SDLK_BACKSPACE:
                if(net)
                    ScreenStatus = Car_Modified_net;
                else
                    ScreenStatus = Car_Modified;
                beep8.play();
                break;
            default:
                break;
        }
    }
}

void MapChose::render(){
    mapthumbnail[map_ptr].render(350, 100);
    for(int i = 0; i < 5; i++){
        if(i == map_ptr){
            mapbarHighlight[i].render(50, 130+i*80);
        }
        else{
            mapbar[i].render(50, 130+i*80);
        }
    }
    mapName[map_ptr].render(50, 50);
    info[map_ptr].render(350, 570);
}

//=======================================
boxCar::boxCar(int x, int y, int player){
    font = TTF_OpenFont("content/font/PressStartTwoP/PressStartTwoP.ttf", 40);
    this->player = player;
    this->enginePower.loadSoundFile("content/sound/engine.wav");
    
    //body definition
    b2BodyDef carDef;
    carDef.type = b2_dynamicBody;
    //    carDef.bullet = true;
    
    //shape definition
    b2PolygonShape boxShape;
    boxShape.SetAsBox(7.7,10, b2Vec2(0, 0), 0);
    
    //fixture definition
    b2FixtureDef boxFixtureDef;
    boxFixtureDef.shape = &boxShape;
    boxFixtureDef.density = 1;
    boxFixtureDef.filter.categoryBits = CAR;
    boxFixtureDef.filter.maskBits = CAR | NORMAL_WALL | WEAPON;
    
    //create dynamic body
    carDef.position.Set(x, y);
    car = myWorld->CreateBody(&carDef);
    car->CreateFixture(&boxFixtureDef);
    
    hurt_contacting = 0;
    car->SetUserData( this );
    
    this->life = 100;
    if(player == 1){
        recoverBar.x = 660;
        recoverBar.y = 18;
        this->lifeBar.x = 660;
        this->lifeBar.y = 23;
        fireBar.x = 810;
        fireBar.y = 95;
    }
    if(player == 2){
        recoverBar.x = 60;
        recoverBar.y = 18;
        this->lifeBar.x = 60;
        this->lifeBar.y = 23;
        fireBar.x = 215;
        fireBar.y = 95;
    }
    this->lifeBar.h = 30;
    this->lifeBar.w = this->life*2.5;
    recoverBar.h = 5;
    recoverBar.w = 0;
    fireBar.h = 5;
    fireBar.w = 0;
    
    //=======
    
}

void boxCar::reduce(int weaponIndex){
    int reduceAmount = 0;
    switch (weaponIndex) {
        case 0:
            reduceAmount = 5;
            break;
        case 1:
            reduceAmount = 3;
            break;
        case 2:
            reduceAmount = 8;
            break;
        case 3:
            reduceAmount = 13;
            break;
        case 4:
            reduceAmount = 10;
            break;
        default:
            break;
    }
    if(this->life >= reduceAmount)
        this->life -= reduceAmount;
    else
        this->life = 0;
}

void boxCar::recoverTimer(){
    if(recoverFlag){
        recoverCount = SDL_GetTicks();
        recoverFlag = false;
        recoverIMG.loadFromFile("content/recover.png");
        recoverIMGBW.loadFromFile("content/recoverBW.png");
//        std::cout << "start Count\n";
    }
    if(SDL_GetTicks() -  recoverCount > recoverTime){
        if(recoverTool < 3)
            recoverTool++;
        recoverFlag = true;
//        std::cout << "recover tool\n";
    }
}

void boxCar::fireTimer(){
    if(fireICON.empty())
        fireICON.loadFromFile("content/fire.png");
    if(fireICONBW.empty())
        fireICONBW.loadFromFile("content/fireBW.png");
    if(fireFlag){
        fireCount = SDL_GetTicks();
        fireFlag = false;
        std::cout << "start\n";
    }
    if(SDL_GetTicks() - fireCount > fireTime){
        if(!fireTool){
            fireTool = true;
        }
    }
}

void boxCar::defense(){
    if(defenseTool){
        renderDefense = true;
        defenseTool = false;
        defenseFlag = true;
        defenseCount = SDL_GetTicks();
        
        //defense
        b2CircleShape defenseShape;
        defenseShape.m_p.Set(0, 0);
        defenseShape.m_radius = 30;
        b2FixtureDef defenseT;
        defenseT.shape = &defenseShape;
        defenseT.density = 0.1;
        defenseT.filter.categoryBits = NORMAL_WALL;
        defenseT.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        defenseFixture = car->CreateFixture(&defenseT);
    }
}
    
void boxCar::defenseRender(){
    if(defenseIMG.empty()){
        defenseIMG.loadFromFile("content/defenseIMG.png");
    }
    if(defenseICON.empty()){
        defenseICON.loadFromFile("content/defense.png");
    }
    if(defenseICONBW.empty()){
        defenseICONBW.loadFromFile("content/defenseBW.png");
    }
    if(defenseTool){
        if(player == 1){
            defenseICON.render(860,60);
        }
        if(player == 2){
            defenseICON.render(265,60);
        }
    }
    else{
        if(player == 1){
            defenseICONBW.render(860,60);
        }
        if(player == 2){
            defenseICONBW.render(265,60);
        }
    }
    if(renderDefense){
        if(SDL_GetTicks() - defenseCount > defenseTime){
            renderDefense = false;
            if(defenseFixture){
                car->DestroyFixture(defenseFixture);
                defenseFixture = NULL;
            }
        }
        b2Vec2 pos = car->GetPosition(); pos*=5;
        for(int i = 0; i < 18; i++){
            defenseIMG.render(pos.x + defenseRadius*cos((defenseAngle+20*i)*DEGTORAD), pos.y + defenseRadius*sin((defenseAngle+20*i)*DEGTORAD)+50);
        }
        defenseAngle += 1;
    }
    
}
    
void boxCar::fireAttack(){
    if(fireTool){
        renderFire = true;
        fireTool = false;
        fireFlag = true;
        renderFireFlag = true;
    }
}

void boxCar::fireRender(){
    if(renderFire){
        if(fireIMG.empty())
            fireIMG.loadFromFile("content/fireTurn.png");
        b2Vec2 pos = car->GetPosition(); pos*=5;
        for(int i = 0; i < 18; i++){
            fireIMG.render(pos.x + fireRadius*cos((fireAngle+20*i)*DEGTORAD), pos.y + fireRadius*sin((fireAngle+20*i)*DEGTORAD)+50);
        }
        fireAngle += 2;
    }
    if(renderFireFlag){
        fireRenderCount = SDL_GetTicks();
        renderFireFlag = false;
        
        //weapon setup
        //fire
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2CircleShape weaponShape;
        weaponShape.m_p.Set(0, 0);
        weaponShape.m_radius = 22;
        fireWeapon.shape = &weaponShape;
        fireWeapon.density = 0.1;
        fireWeapon.filter.categoryBits = WEAPON;
        fireWeapon.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        fireFixture = car->CreateFixture(&fireWeapon);
    }
    if(fireTool){
        if(player == 1)
            fireICON.render(810, 60);
        if(player == 2)
            fireICON.render(215, 60);
    }
    if(SDL_GetTicks() - fireRenderCount > fireRenderTime){
        renderFire = false;
        if(fireFixture){
            car->DestroyFixture(fireFixture);
            fireFixture = NULL;
        }
    }
    if(!fireTool){
        if(player == 1)
            fireICONBW.render(810, 60);
        if(player == 2)
            fireICONBW.render(215, 60);
        SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);
        SDL_RenderFillRect(gRenderer, &this->fireBar);
        this->fireBar.w = (SDL_GetTicks() - fireCount)/(250);
        SDL_RenderDrawRect(gRenderer, &this->fireBar);
    }
    
}

void boxCar::recover(){
    if(recoverTool > 0){
        if(this->life <= 95)
            this->life += 10;
        else
            this->life = 100;
        recoverTool--;
    }
}

void boxCar::recover(int m){
    if(this->life <= m)
        this->life += m;
    else
        this->life = 100;
}

void boxCar::renderLife(boxCar *enemy){
    if (this->hurt_contacting > 0) {
        this->hurt(enemy);
        if(enemy->player == 1){
            this->reduce(p1_weaponIndex);
        }
        else if(enemy->player == 2){
            this->reduce(p2_weaponIndex);
        }
    }
    if (this->weapon_contacting > 0) {
        this->hurt(enemy);
    }
    
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
    SDL_RenderFillRect(gRenderer, &this->lifeBar);
    this->lifeBar.w = this->life*3;
    SDL_RenderDrawRect(gRenderer, &this->lifeBar);
    
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 255);
    SDL_RenderFillRect(gRenderer, &this->recoverBar);
    this->recoverBar.w = (SDL_GetTicks() - recoverCount)/(recoverTime/250);
    SDL_RenderDrawRect(gRenderer, &this->recoverBar);
    
    //recover Tool
    recoverTimer();
    for(int i = 0; i < 3; i++){
        if(i < recoverTool){
            if(player == 1){
                recoverIMG.render(660 + i * 50,60);
            }
            else if(player == 2){
                recoverIMG.render(65 + i * 50,60);
            }
        }
        else{
            if(player == 1){
                recoverIMGBW.render(660 + i * 50,60);
            }
            else if(player == 2){
                recoverIMGBW.render(65 + i * 50,60);
            }
        }
    }
}

void boxCar::hurt(boxCar *enemy){
    float32 enemy_angle = enemy->car->GetAngle();
    float32 forceMySelf = 100000;
    float32 force = 100000;
    this->car->SetLinearVelocity(b2Vec2(-sinf(enemy_angle)*forceMySelf, -cosf(enemy_angle)*forceMySelf));
    enemy->car->SetLinearVelocity(b2Vec2(sinf(enemy_angle)*force, cosf(enemy_angle)*force));
//    this->car->ApplyForce(b2Vec2(-sinf(enemy_angle)*forceMySelf, -cosf(enemy_angle)*forceMySelf), this->car->GetWorldCenter(), true);
//    enemy->car->ApplyForce(b2Vec2(-sinf(enemy_angle)*force, -cosf(enemy_angle)*force), enemy->car->GetWorldCenter(), true);
}

bool boxCar::die(){
    if(this->life <= 0){
        return true;
    }
    else{
        return false;
    }
}

void boxCar::setWeapon(int weaponIndex){
    if(weaponIndex == 0){
        //weapon
        b2FixtureDef weaponFixtureDef;
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2CircleShape weaponShape;
        weaponShape.m_p.Set(0, -10);
        weaponShape.m_radius = 10;
        weaponFixtureDef.shape = &weaponShape;
        weaponFixtureDef.density = 1;
        weaponFixtureDef.filter.categoryBits = WEAPON;
        weaponFixtureDef.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        car->CreateFixture(&weaponFixtureDef);
        
    }
    else if(weaponIndex == 1){
        //weapon
        b2FixtureDef weaponFixtureDef;
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2PolygonShape weaponShape;
        weaponShape.SetAsBox(5, 20);
        weaponFixtureDef.shape = &weaponShape;
        weaponFixtureDef.density = 3;
        weaponFixtureDef.filter.categoryBits = WEAPON;
        weaponFixtureDef.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        car->CreateFixture(&weaponFixtureDef);
    }
    else if(weaponIndex == 2){
        //weapon
        b2FixtureDef weaponFixtureDef;
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2PolygonShape weaponShape;
        weaponShape.SetAsBox(3.5, 9.5, b2Vec2(0, -18.5), 0);
        weaponFixtureDef.shape = &weaponShape;
        weaponFixtureDef.density = 3;
        weaponFixtureDef.filter.categoryBits = WEAPON;
        weaponFixtureDef.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        car->CreateFixture(&weaponFixtureDef);
    }
    else if(weaponIndex == 3){
        b2FixtureDef weaponFixtureDef;
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2PolygonShape weaponShape;
        weaponShape.SetAsBox(7.7, 4.3, b2Vec2(0, -14), 0);
        weaponFixtureDef.shape = &weaponShape;
        weaponFixtureDef.density = 0.05;
        weaponFixtureDef.filter.categoryBits = WEAPON;
        weaponFixtureDef.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        car->CreateFixture(&weaponFixtureDef);
    }
    else if(weaponIndex == 4){
        b2FixtureDef weaponFixtureDef;
        b2BodyDef weapon;
        weapon.type = b2_dynamicBody;
        b2PolygonShape weaponShape;
        weaponShape.SetAsBox(0.7, 6.1, b2Vec2(0, 16), 0);
        weaponFixtureDef.shape = &weaponShape;
        weaponFixtureDef.density = 0.01;
        weaponFixtureDef.filter.categoryBits = WEAPON;
        weaponFixtureDef.filter.maskBits = CAR | WEAPON | NORMAL_WALL;
        weapon.position.Set(car->GetPosition().x, car->GetPosition().y);
        car->CreateFixture(&weaponFixtureDef);
    }
}

void boxCar::setPerformance(int engine, int steer, int tire){
    switch (engine) {
        case 0:
            this->engine = EngineA;
            break;
        case 1:
            this->engine = EngineB;
            break;
        case 2:
            this->engine = EngineC;
            break;
        case 3:
            this->engine = EngineD;
        default:
            break;
    }
    switch (steer) {
        case 0:
            this->steer = SteerA;
            break;
        case 1:
            this->steer = SteerB;
            break;
        case 2:
            this->steer = SteerC;
            break;
        case 3:
            this->steer = SteerD;
            break;
        default:
            break;
    }
    switch (tire) {
        case 0:
            this->tire = TireA;
            break;
        case 1:
            this->tire = TireB;
            break;
        case 2:
            this->tire = TireC;
            break;
        case 3:
            this->tire = TireD;
        default:
            break;
    }
}

void boxCar::handelEvent(SDL_Event &e){
    if(player == 1){
        if( e.key.state == SDL_PRESSED && e.key.repeat == 0){
            switch( e.key.keysym.sym )
            {
                case SDLK_LEFT:
                    if(mVel < 0){
                        mvelR -= steer;
                    }
                    else{
                        mvelR += steer;
                    }
                    break;
                case SDLK_RIGHT:
                    if(mVel < 0){
                        mvelR += steer;
                    }
                    else{
                        mvelR -= steer;
                    }
                    break;
                case SDLK_UP:
                    mVel += engine;
                    enginePower.play();
                    break;
                case SDLK_DOWN:
                    mVel -= engine;
                    enginePower.play();
                    break;
                    
                    //shoot
                case SDLK_COMMA:
                    recover();
                    break;
                case SDLK_PERIOD:
                    fireAttack();
                    break;
                case SDLK_SLASH:
                    defense();
                    break;
                default:
                    break;
            }
        }
        else if( e.key.state == SDL_RELEASED && e.key.repeat == 0){
            switch( e.key.keysym.sym )
            {
                    //move
                case SDLK_LEFT:
                    mvelR = 0;
                    break;
                case SDLK_RIGHT:
                    mvelR = 0;
                    break;
                case SDLK_UP:
                    mVel = 0;
                    break;
                case SDLK_DOWN:
                    mVel = 0;
                    break;
            }
        }
    }
    if(player == 2){
        if( e.key.state == SDL_PRESSED && e.key.repeat == 0){
            switch( e.key.keysym.sym )
            {
                case SDLK_a:
                    if(mVel < 0){
                        mvelR -= steer;
                    }
                    else{
                        mvelR += steer;
                    }
                    break;
                case SDLK_d:
                    if(mVel < 0){
                        mvelR += steer;
                    }
                    else{
                        mvelR -= steer;
                    }
                    break;
                case SDLK_w:
                    mVel += engine;
                    enginePower.play();
                    break;
                case SDLK_s:
                    mVel -= engine;
                    enginePower.play();
                    break;
                    
                    //shoot
                case SDLK_1:
                    recover();
                    break;
                case SDLK_2:
                    fireAttack();
                    break;
                case SDLK_3:
                    defense();
                    break;
                default:
                    break;
            }
        }
        else if( e.key.state == SDL_RELEASED && e.key.repeat == 0){
            switch( e.key.keysym.sym )
            {
                    //move
                case SDLK_a:
                    mvelR = 0;
                    break;
                case SDLK_d:
                    mvelR = 0;
                    break;
                case SDLK_w:
                    mVel = 0;
                    break;
                case SDLK_s:
                    mVel = 0;
                    break;
            }
        }
    }
}

void boxCar::move(){
    double angle = car->GetAngle();
    car->SetAngularVelocity(-mvelR*DEGTORAD);
    //    car->SetLinearVelocity(b2Vec2(-mVel*sin(angle), -mVel*cos(angle)));
    car->ApplyForce(b2Vec2(mVel*sin(angle),-mVel*cos(angle)), car->GetWorldCenter(), true);
}

void boxCar::friction(){
    b2Vec2 vel = car->GetLinearVelocity();
    vel.x = -vel.x*(tire);
    vel.y = -vel.y*(tire);
    car->ApplyForce(vel,car->GetWorldCenter(), true);
}
