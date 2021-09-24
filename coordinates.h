#ifndef _Coordinates_h_
#define _Coordinates_h_
#include "math.h"
#ifndef PI
#define     PI 3.1415926535897932384626433832795
#endif /*\ #ifndef PI \*/

class Coordinates {
    public:
        /* start value, start speed, update confidence, evolution confidence */
        Coordinates(float x = 0, float y = 0);
        void fromCartesian(float x, float y);
        void setCartesian(float x, float y);
        void fromPolar(float r, float phi);
        void fromPolar(float r, float phi, float x_off, float y_off);

        float getR() {
          return r;
        }
        float getAngle() {
          return phi;
        }
        float getX() {
          return x;
        }
        float getY() {
          return y;
        }

    private:
        float r;
        float phi;
        float x;
        float y;
};

#endif /*\ #ifndef __Coordinates_h_ \*/
