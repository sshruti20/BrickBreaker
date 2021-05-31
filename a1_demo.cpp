// CS 349 Fall 2018
// A1: Breakout code sample
// You may use any or all of this code in your assignment!
// See makefile for compiling instructions

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

// X11 structures
Display* display;
Window window;

struct XInfo {
    Display	 *display;
    int		 screen;
    Window	 window;
};

/* Static Global Variables */
static GC rbColors[7]; //vibgyor colors
static GC scoreFontGC;

void showFailedAllocateColorErrorMessage();

string IntToString (int n)
{
    ostringstream ss;
    ss << n;
    return ss.str();
}


class Ball {
public:
    int x;
    int y;
    int ballSize;
    
    int dirX;
    int dirY;
    
    GC gc;
    
    void init(){
        x = 1280/2;
        y = 500;
        ballSize = 50;
        
        dirX = 3;
        dirY = 3;
        
        gc = XCreateGC(display, window, 0, 0);
    }
    
    void reset() {
        x = 1280/2;
        y = 500;
    }
    
    void paint(Window& w) {
        XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
        XSetBackground(display, gc, BlackPixel(display, DefaultScreen(display)));
        
        // draw ball from centre
        XFillArc(display, w, gc,
                 x - ballSize/2,
                 y - ballSize/2,
                 ballSize, ballSize,
                 0, 360*64);
    }
};

static bool onSplashScreen;
static bool onquitScreen;

void initStaticGlobalVars(Window& pixmap) {
    
    onSplashScreen = true;
    onquitScreen = false;
    
    XColor colors[7];
    Colormap colormap = DefaultColormap(display, DefaultScreen(display));
    XColor violet, purple, blue, green, yellow, orange, red;
    Status rc; //return status of various Xlib functions.
    
    // allocate the set of colors to be used for the drawing
     rc = XAllocNamedColor(display, colormap, "violet", &violet, &violet);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "purple", &purple, &purple);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "blue", &blue, &blue);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "green", &green, &green);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "yellow", &yellow, &yellow);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "orange", &orange, &orange);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
     rc = XAllocNamedColor(display, colormap, "red", &red, &red);
     if (rc == 0) { showFailedAllocateColorErrorMessage(); }
    
    //now , should refactor and add stuff to loops
    colors[0] = violet;
    colors[1] = purple;
    colors[2] = blue;
    colors[3] = green;
    colors[4] = yellow;
    colors[5] = orange;
    colors[6] = red;
    
    int screen = DefaultScreen(display);
    for(int i=0; i<7; i++){
        rbColors[i] = XCreateGC(display, pixmap, 0, 0);
        XSetForeground(display, rbColors[i], colors[i].pixel);
        XSetBackground(display, rbColors[i], WhitePixel(display, screen));
    }
    
    scoreFontGC = XCreateGC(display, pixmap, 0, 0);
    XSetForeground(display, scoreFontGC, colors[3].pixel);
    XSetBackground(display, scoreFontGC, WhitePixel(display, screen));
    XFontStruct * font;
    font = XLoadQueryFont (display, "12x24");
    XSetFont(display, scoreFontGC, font->fid);
}

/*
 * An abstract class representing displayable things.
 */
class Displayable {
public:
    virtual void paint() = 0;
};

// fixed frames per second animation
int FPS = 30;

// get current time
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void showFailedAllocateColorErrorMessage() {
    cout<<"XAllocNamedColor - failed to allocate color\n";
    exit(1);
}

class Brick {
    public:
    int x,y; //Position
    int width;
    int height;
    bool shown;
    int ox;
    int oy;
    GC gc;
    int posX; //i.e the row of the brick in 'bricks', to help determine gc & color
    
    void init(int x, int y, bool shown, int posX) {
        this->x = x;
        this->y = y;
        this->ox = x;
        this->oy = y;
        this->shown = shown;
        this->posX = posX;
        this->gc = rbColors[posX];
        int screen = DefaultScreen(display);
        
        width = 70;
        height = 35;
    }
    
    void paint(Window& w) {
        XFillRectangle(display, w, gc, x, y, width, height);
    }
    
    
    void reset() {
        width = 70;
        height = 35;
        bool shown = true;
        x = ox;
        y = oy;
    }
    
    bool didCollide(Ball& ball) {
        
            if(((float)ball.x > (float)this->x - ball.ballSize) && ((float)ball.x < (float)this->x + (float)this->width) &&
               ((float)ball.y > (float)this->y - ball.ballSize) && ((float)ball.y < (float)this->y + (float)this->height)) {
                if(((float)ball.x > (float)this->x - ball.ballSize) && ((float)ball.x <= (float)this->x + (float)this->width)
                   && (!((float)ball.y > (float)this->y - ball.ballSize)) && (!((float)ball.y< (float)this->y + (float)this->height))) {
                    ball.dirX = -ball.dirX;
                    x = 0;
                    y = 0;
                    width = 0;
                    height = 0;
                    shown = false;
                    return true;
                }
                else if(((float)ball.y > (float)this->y - ball.ballSize) && ((float)ball.y< (float)this->y + (float)this->height)) {
                    ball.dirY = -ball.dirY;
                    x = 0;
                    y = 0;
                    width = 0;
                    height = 0;
                    shown = false;
                    return true;
                }
            }
            else {
                return false;
            }
        return false;
    }
};

class Bricks {
public:
    static const int width = 70;
    static const int height = 35;
    
    static const int startPosX = 250;
    static const int startPosY = 100;
    
    static const int rows = 7;
    static const int columns = 10;
    
    static const int scoreX = 50;
    static const int scoreY = 70;
    
    int score;
    
    Brick bricks[7][10];
    
    void init() {
        int x = startPosX, y = startPosY;
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < columns; j++) {
                bricks[i][j].init(x,y, true, i);
                x+= width + 2;
            }
            x = startPosX;
            y+= height + 2;
        }
        score = 0;
    }
    
    void paint(Window& w) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                bricks[i][j].paint(w);
            }
        }
    }
    
    void resetBricks() {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                bricks[i][j].reset();
            }
        }
    }
    
    void didBricksCollide(Ball& ball) {
            for(int i = 0; i < rows; i++) {
                for(int j = 0; j < columns; j++) {
                    if(bricks[i][j].didCollide(ball)) {
                        score += 1;
                        //debugging - check winning condition TODO
                    }
                }
            }
    }
    
    void displayScore(Window &w) {
        string sscore = "Score: ";
        sscore += IntToString(score);
        
        XDrawImageString(display, w, scoreFontGC, this->scoreX, this->scoreY, sscore.c_str(), sscore.length());
    }
    
};


class Board {
public:
    float x;
    float y;
    GC gc;
    float width;
    float height;
    
    void init() {
        x = 800;
        y = 700;
        width = 100;
        height = 30;
        gc = XCreateGC(display, window, 0, 0);
        XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
        XSetBackground(display, gc, BlackPixel(display, DefaultScreen(display)));
    }
    
    void paint(Window& w) {
        XDrawRectangle(display, w, gc, x, y, 70, 30);
    }
    
    void handleBoardCollision(Ball& ball) {
        if(((float)ball.x > (float)this->x - ball.ballSize) && ((float)ball.x < (float)this->x + (float)this->width) &&
           ((float)ball.y > (float)this->y - ball.ballSize) && ((float)ball.y < (float)this->y + (float)this->height)) {
            if(((float)ball.x > (float)this->x - ball.ballSize) && ((float)ball.x <= (float)this->x + (float)this->width)
               && (!((float)ball.y > (float)this->y - ball.ballSize)) && (!((float)ball.y< (float)this->y + (float)this->height))) {
                ball.dirX = -ball.dirX;
            }
            else if(((float)ball.y > (float)this->y - ball.ballSize) && ((float)ball.y< (float)this->y + (float)this->height)) {
                ball.dirY = -ball.dirY;
            }
        }
    }
    
};

void drawSplashScreen(Window& w){
    
    int x = 350;
    int y = 250;
    
    string name = "Shruti";
    string sid = "nope";
    
    string controls = "To control board movement: a for LEFT , d for RIGHT";
    string q = "Press q to quit";
    string c = "Press c to continue to game";
    
    XDrawImageString(display, w, scoreFontGC, x, y, name.c_str(), name.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 50, sid.c_str(), sid.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 150, controls.c_str(), controls.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 200, q.c_str(), q.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 250, c.c_str(), c.length());
}

void drawQuitScreen(Window& w){
    
    int x = 350;
    int y = 250;
    
    string name = "Game Over";
    string q = "Press q to quit";
    string c = "Press p to play again";
    
    XDrawImageString(display, w, scoreFontGC, x, y, name.c_str(), name.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 50, q.c_str(), q.length());
    XDrawImageString(display, w, scoreFontGC, x, y + 100, c.c_str(), c.length());
}


// entry point
int main( int argc, char *argv[] ) {
    
    int screenWidth = 1280;
    int screenHeight = 800;
    int ballSpeed = 20;
    
    if (argc >= 2){
        istringstream temp(argv[1]);
        temp >> FPS;
    }
    
    // create window
    display = XOpenDisplay("");
    if (display == NULL) exit (-1);
    int screennum = DefaultScreen(display);
    long background = WhitePixel(display, screennum);
    long foreground = WhitePixel(display, screennum);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display),
                                 0, 0, screenWidth, screenHeight, 2, foreground, background);
    
    //Disable window from getting resized
    XSizeHints *hints = XAllocSizeHints();
    hints->flags = PMinSize|PMaxSize;
    hints->min_width = hints->max_width = screenWidth;
    hints->max_height = hints->min_height = screenHeight;
    XSetWMNormalHints(display, window, hints);
    XSetWMSizeHints(display, window, hints, PMinSize|PMaxSize);
    
    // set events to monitor and display window
    XSelectInput(display, window, ButtonPressMask | KeyPressMask);
    XMapRaised(display, window);
    XFlush(display);
    
    // create gc for drawing
    GC gc = XCreateGC(display, window, 0, 0);
    XWindowAttributes w;
    XGetWindowAttributes(display, window, &w);
    
    // save time of last window paint
    unsigned long lastRepaint = 0;
    
    // event handle for current event
    XEvent event;
    
    /* Double buffering code */
    // create bimap (pximap) to use a other buffer
    int depth = DefaultDepth(display, DefaultScreen(display));
    Pixmap	buffer = XCreatePixmap(display, window, w.width, w.height, depth);
    bool useBuffer = true;
    
    //Initializing static global variables
    initStaticGlobalVars(buffer); //Might need to change if buffer changes
    
    /* The class that handles drawing of bricks */
    Bricks* bricks = new Bricks();
    if (bricks == NULL) {
        cout<<"Couldn't allocate memory for bricks\n";
    }
    bricks->init();
    
    Board board;
    board.init();
    
    Ball ball;
    ball.init();
    
    // event loop
    while ( true ) {
        
        // process if we have any events
        if (XPending(display) > 0) {
            XNextEvent( display, &event );
            
            switch ( event.type ) {
                    
                    // mouse button press
                case ButtonPress:
                    //cout << "CLICK" << endl;
                    break;
                    
                case KeyPress: // any keypress
                    KeySym key;
                    char text[10];
                    int i = XLookupString( (XKeyEvent*)&event, text, 10, &key, 0 );
                    
                    // move right
                    if ( i == 1 && text[0] == 'd' ) {
                        board.x += 10;
                    }
                    
                    // move left
                    if ( i == 1 && text[0] == 'a' ) {
                        board.x -= 10;
                    }
                    
                    if ( i == 1 && text[0] == 'c' && onSplashScreen ) {
                        onSplashScreen = false;
                    }
                    
                    if ( i == 1 && text[0] == 'p' && onquitScreen ) {
                        bricks->score = 0;
                        bricks->resetBricks();
                        ball.reset();
                        onquitScreen = false;
                    }
                    
                    // quit game
                    if ( i == 1 && text[0] == 'q' ) {
                        XCloseDisplay(display);
                        exit(0);
                    }
                    break;
            }
        }
        
        unsigned long end = now();	// get current time in microsecond
        
        if (end - lastRepaint > 1000000 / FPS) {
            
            Pixmap pixmap;
            
            if (useBuffer) {
                pixmap = buffer;
                // draw into the buffer
                // note that a window and a pixmap are “drawables”
                XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
                /* Clearing the buffer */
                XFillRectangle(display, pixmap, gc, 0, 0, w.width, w.height);
            } else {
                pixmap = window;
                // clear background
                XClearWindow(display, pixmap);
            }
            
            // clear background
            XClearWindow(display, window);
            
            if (onSplashScreen) {
                drawSplashScreen(pixmap);
            } else if (onquitScreen) {
                drawQuitScreen(pixmap);
            } else {
                /* Draw bricks */
                bricks->paint(pixmap);
                
                /* Draw Board */
                board.paint(pixmap);
                
                ball.paint(pixmap);
                
                XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
                XSetBackground(display, gc, BlackPixel(display, DefaultScreen(display)));
                
                ball.x += ball.dirX;
                ball.y += ball.dirY;
                
                board.handleBoardCollision(ball);
                bricks->didBricksCollide(ball);
                
                // bounce ball
                if (ball.y + ball.ballSize/2 > w.height) {
                    onquitScreen = true;
                }
                if (ball.x + ball.ballSize/2 > w.width ||
                    ball.x - ball.ballSize/2 < 0)
                    ball.dirX = -ball.dirX;
                
                if (ball.y + ball.ballSize/2 > w.height ||
                    ball.y - ball.ballSize/2 < 0)
                    ball.dirY = -ball.dirY;
                
                bricks->displayScore(pixmap);
            }
            
            /* copy buffer to window */
            if (useBuffer) {
                XCopyArea(display, pixmap, window, gc,
                          0, 0, w.width, w.height,  // region of pixmap to copy
                          0, 0); // position to put top left corner of pixmap in window
            }
            
            XFlush(display);
            //XFlush(pixmap);
            
            lastRepaint = now(); // remember when the paint happened
        }
        
        // IMPORTANT: sleep for a bit to let other processes work
        if (XPending(display) == 0) {
            usleep(1000000 / FPS - (now() - lastRepaint));
        }
    }
    XCloseDisplay(display);
}
