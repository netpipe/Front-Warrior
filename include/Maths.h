#ifndef MATH_HEADER_DEFINED
#define MATH_HEADER_DEFINED

#include <irrlicht.h>
#include <stdlib.h>
#include <time.h>

namespace engine {

  class CMaths
  {
  public:
    CMaths(){ }

    irr::u32 getRandomInt(irr::s32 from, irr::s32 to) { return rnd(from, to); }

    void alignToUpVector(
      irr::core::matrix4 &mat_,
      const irr::core::matrix4 &oldMat_,
      const irr::core::vector3df &newUp_,
      irr::f32 interpolate_);

    irr::core::vector3df lerp(irr::core::vector3df vec1, irr::core::vector3df vec2, irr::f32 ratio) {
      return (vec1 * (1.0f-ratio) + vec2 * ratio);
    }


  private:

    //
    // Random number generation
    //

    struct Random
    {
       //--- CONSTRUCTOR ---
       Random()
       {
          time_t seconds;
          time(&seconds);
          srand((unsigned int) seconds);
       }

       //--- DESTRUCTOR ---
       ~Random(){}

       //--- FIND RANDOM NUMBER BETWEEN 0 AND NUMBER ---
       int operator () (int num)
       {
          return rand() % (num + 1);
       }

       //--- FIND RANDOM BETWEEN TWO NUMBERS ---
       int operator () (int num1, int num2)
       {
          return rand() % (num2 - num1 + 1) + num1;
       }
    } rnd;


  };

}

#endif

