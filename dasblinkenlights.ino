
#include "draw.h"
#include <OctoWS2811.h>

//OctoWS2811 Defn. Stuff
#define COLS_LEDs 10  // all of the following params need to be adjusted for screen size
#define ROWS_LEDs 6  // LED_LAYOUT assumed 0 if ROWS_LEDs > 8
#define LEDS_PER_STRIP (COLS_LEDs * (ROWS_LEDs / 6))
#define NUM_PIXELS 60

DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

static Program currentProgram = NONE;
static bool keyDown = 0;

int pointToIndex(LEDPoint point)
{
  return (((ROWS_LEDs-1) - point.y)*COLS_LEDs) + ((point.y%2 != 0) ? (COLS_LEDs-1) - point.x : point.x);
}


void paintPixel(LEDPoint point, int colorIn, float alpha, CompositOp op)
{
    int index = pointToIndex(point);
    int curentPixel = leds.getPixel(index);
    float rCurrent = (curentPixel & 0xFF0000) >> 16;
    float gCurrent =  (curentPixel & 0x00FF00) >> 8;
    float bCurrent =  (curentPixel & 0x0000FF);
    
    float rIn = (colorIn & 0xFF0000) >> 16 ;
    float gIn =  (colorIn & 0x00FF00) >> 8 ;
    float bIn =  (colorIn & 0x0000FF);
    float r,g,b;
    switch (op)
    {
        case ADD:
            r = rIn * alpha + rCurrent * (1-alpha);
            g = gIn * alpha + gCurrent * (1-alpha);
            b = bIn * alpha + bCurrent * (1-alpha);
            break;
            
        case OVER:
            r = rIn * alpha;
            g = gIn * alpha;
            b = bIn + alpha;
            break;
            
        case SUB:
            r = rIn * alpha - rCurrent * (1-alpha);
            g = gIn * alpha - gCurrent * (1-alpha);
            b = bIn * alpha - bCurrent * (1-alpha);
            break;
    }

    rCurrent = (r <= 255) ? r : 255;
    gCurrent = (g <= 255) ? g : 255;
    bCurrent = (b <= 255) ? b : 255;
    
    leds.setPixel(index, leds.color(rCurrent, gCurrent, bCurrent));
}

void circle(int color, float alphaIn, CompositOp op, LEDPoint center, float radius, float thickness)
{
    for (uint8_t y = 0; y < ROWS_LEDs; y++)
    {
        for (uint8_t x = 0; x < COLS_LEDs; x++)
        {
            int dx = x-center.x;
            int dy = y-center.y;
            
            float distanceFactor = (thickness - fabs(sqrt(dx*dx+dy*dy) - radius)) / thickness;
            if (distanceFactor < 0) distanceFactor = 0;
            float alpha = alphaIn * distanceFactor;
            if (alpha > 1.0) alpha = 1.0;
            paintPixel((LEDPoint){x,y}, color, alpha, op);
        }
    }
}


void fade(float fadeAmount)
{
    for (uint8_t i = 0; i < NUM_PIXELS; i++)
    {
      int color = leds.getPixel(i);
      float r = ((color & 0xFF0000) >>16) * fadeAmount;
      float g =  ((color & 0x00FF00) >> 8) * fadeAmount;
      float b =  (color & 0x0000FF) * fadeAmount;
      leds.setPixel(i, leds.color(r,g,b));
    }
}

static float blushValue = 0.20;

void blush(int colorIn, float alpha, LEDPoint start)
{
    static float radius = 0.0;
    circle(BLACK, 1.0, ADD, (LEDPoint){5,5}, radius, 2);
    radius += 0.25;
    if (radius > 10) radius = 0.0;
    
    if (keyDown)
    {
        blushValue *= 1.1;
    }
    else
    {
        blushValue *= 0.8;
    }
    
    if (blushValue > 4) blushValue = 4;
    if (blushValue < 0.02) blushValue = 0.02;
    
    for (uint8_t y = 0; y < ROWS_LEDs; y++)
    {
        for (uint8_t x = 0; x < COLS_LEDs; x++)
        {
            int dx = x-start.x;
            int dy = y-start.y;
            
            float distanceFactor = 6 - sqrt(dx*dx+dy*dy);
            if (distanceFactor < 0) distanceFactor = 0;
            
            float newAlpha = alpha * blushValue * distanceFactor;

            float r = (colorIn & 0xFF0000) >> 16;
            float g = (colorIn & 0x00FF00) >> 8;
            float b = (colorIn & 0x0000FF);
            
            if (newAlpha > 1.0)
            {
                newAlpha -= 1.0;
                if (newAlpha > 1.0)
                {
                    newAlpha = 1.0;
                }
                
                if (r > g && r > b)
                {
                    g = 255 * newAlpha;
                    b = 255 * newAlpha;
                }
                else if (g > b)
                {
                    r = 255 * newAlpha;
                    b = 255 * newAlpha;
                }
                else
                {
                    r = 255 * newAlpha;
                    g = 255 * newAlpha;
                }
            }
            
            paintPixel((LEDPoint){x,y}, leds.color(r, g, b), newAlpha, ADD);
        }
    }
}


static LEDPoint hiPoints[] = {
    {2,0}, {2,1}, {2,2}, {2,3}, {2,4}, {2,5},
    {3,3}, {4,3}, {5,3},
    {5,4}, {5,5},
    {8,1}, {8,3}, {8,4}, {8,5}
};
static int hiPointNum = 15;
static float hiAlpha = 0.2;

void hi()
{
    if (keyDown)
    {
        for (int i=0; i < hiPointNum; i++)
        {
            LEDPoint point = hiPoints[i];
            paintPixel(point, 0xff3232, hiAlpha, ADD);
        }
    }
}


static int topLength = 28;
static LEDPoint chompTop[] = {
    {0,0}, {1,0}, {2,0}, {3,0}, {4,0},  {5,0}, {6,0}, {7,0}, {8,0}, {9,0},
    {0,1}, {1,1}, {2,1}, {3,1}, {4,1},  {5,1}, {6,1}, {7,1}, {8,1}, {9,1},
                  {2,2}, {3,2},                {6,2}, {7,2},
                  {2,3}, {3,3},                {6,3}, {7,3}
};

static int bottomLength = 32;
static LEDPoint chompBottom[] = {
    {0,2}, {1,2},               {4,2}, {5,2},               {8,2}, {9,2},
    {0,3}, {1,3},               {4,3}, {5,3},               {8,3}, {9,3},
    {0,4}, {1,4}, {2,4}, {3,4}, {4,4}, {5,4}, {6,4}, {7,4}, {8,4}, {9,4},
    {0,5}, {1,5}, {2,5}, {3,5}, {4,5}, {5,5}, {6,5}, {7,5}, {8,5}, {9,5}
};

void chomp()
{
    static bool direction = 1;
    static float yOffset = 4;
    if (keyDown)
    {
        yOffset += (direction) ? -0.1 : 0.1;
        if (yOffset > 4 || yOffset <0) direction = !direction;

        for (int i=0; i < topLength ; i++)
        {
            LEDPoint point = chompTop[i];
            point.y -= yOffset;
            paintPixel(point, RED, 1.0, OVER);
        }
        
        for (int i=0; i < bottomLength ; i++)
        {
            LEDPoint point = chompBottom[i];
            point.y += yOffset;
            paintPixel(point, RED, 1.0, OVER);
        }
    }
}


static int coolColor = 0x64DCDC;
static float flakes[] = {-1, 0, -1, 0, -1, 0, -1, -1, -1, -1};
void cool()
{
    if (keyDown)
    {
        for (int i = 0; i < 10; i++)
        {
            if (flakes[i] > 6) flakes[i] = -1;
            
            if (flakes[i] == -1 && random(50) == 0)
            {
                flakes[i] = 0;
            }
        }
    }
    
    for (int y = 0; y < ROWS_LEDs; y++)
    {
        for (int x = 0; x < COLS_LEDs; x++)
        {
            
            float alpha = random(256)/256.0;
            paintPixel((LEDPoint){x,y}, coolColor, alpha, ADD);
        }
    }
    
    for (int i = 0; i < COLS_LEDs; i++)
    {
        float y = flakes[i];
        if (y >= 0)
        {
            paintPixel((LEDPoint){i,y}, WHITE, 1.0, ADD);
            y += 0.1;
            flakes[i] = y;
        }
    }
}


void tv()
{
    static float tvHeight = 2.0;
    static float tvWidth = 1.0;
    
    int cols = tvWidth * COLS_LEDs;
    for (int y = 0; y < ROWS_LEDs; y++)
    {
        for (int x = round(5 - cols/2.0); x < cols; x++)
        {
            float alpha = (3-fabs(3-y))/3.0 * tvHeight;
            uint8_t value = random(255);
            paintPixel((LEDPoint){x,y}, leds.color(value,250,value), alpha, OVER);
        }
    }

    uint8_t value = random(100);
    paintPixel((LEDPoint){4,3}, leds.color(value, 100, value), 0.2, ADD);
    paintPixel((LEDPoint){5,3}, leds.color(value, 100, value), 0.2, ADD);
    
    if (!keyDown)
    {
        tvHeight -= 0.05;
        if (tvHeight < 0.4)
        {
            tvHeight = 0.4;
            tvWidth -= 0.05;
        }
    }
    else
    {
        tvHeight = 2.0;
        tvWidth = 1.0;
    }
}

void setup()
{
  
  pinMode(23, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  
  Serial.begin(38400);
  
  keyDown = 0;
  
  leds.begin();
  leds.show();

}
  
void loop()
{      
  fade(0.9);
 
  keyDown = 0;
  if (digitalRead(23) == LOW)
  {
     //Serial.println("HI");
    currentProgram = HI;
    keyDown = 1;
  }
  if (digitalRead(22) == LOW)
  {
    //Serial.println("blush");
    currentProgram = BLUSH;
    keyDown = 1;
  }
  if (digitalRead(17) == LOW)
  {
    //Serial.println("cool");
    currentProgram = COOL;
    keyDown = 1;
  }
  if (digitalRead(19) == LOW)
  {
    //Serial.println("tv");
    currentProgram = TV;
    keyDown = 1;
  }
  if (digitalRead(18) == LOW)
  {
    Serial.println("none");
    currentProgram = NONE;
    keyDown = 1;
  }
  
  switch (currentProgram)
    {
        case NONE:
            break;
        case BLUSH:
            blush(RED, 0.5, (LEDPoint){5,5});
            break;
        case HI:
            hi();
            break;
        case CHOMP:
            chomp();
            break;
        case COOL:
            cool();
            break;
        case TV:
            tv();
    }

  //digitalWrite(13, HIGH);
    leds.show();  // not sure if this function is needed  to update each frame
  //digitalWrite(13, LOW);
  delayMicroseconds(33000);
  //delayMicroseconds(1000000);
}



