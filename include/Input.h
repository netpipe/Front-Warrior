#ifndef INPUT_HEADER_DEFINED
#define INPUT_HEADER_DEFINED

//#include "Engine.h"
#include <irrlicht.h>
#include "Input.h"

namespace engine {

  struct SKey
  {
      bool isHeldDown(irr::EKEY_CODE &keycode){
          return KeyCodeInUse[keycode];
      }

      bool isPressedOnce(irr::EKEY_CODE &keycode){
          if(KeyCodeInUse[keycode]){
              KeyCodeInUse[keycode] = false;

              return true;
          }
          return false;
      }

      void reset(){
           for(irr::u32 i=0; i<irr::KEY_KEY_CODES_COUNT; ++i)
             KeyCodeInUse[i] = false;
      }

      bool KeyCodeInUse[irr::KEY_KEY_CODES_COUNT];

  };

  struct SMouse
  {
  public:
      bool isLeftButtonHeld(){
          return leftButton;
      }

      bool isMiddleButtonHeld(){
          return middleButton;
      }

      bool isRightButtonHeld(){
          return rightButton;
      }

      bool isLeftButtonClicked(){
          if(leftButton){
              leftButton = false;
              return true;
          }
          return false;
      }

      bool isMiddleButtonClicked(){
          if(middleButton){
              middleButton = false;
              return true;
          }
          return false;
      }

      bool isRightButtonClicked(){
          if(rightButton){
              rightButton = false;
              return true;
          }
          return false;
      }

      void reset()
      {
          X = 0;
          Y = 0;
          oldX = 0;
          oldY = 0;
          wheelValue = 0.0f;
          leftButton = false;
          middleButton = false;
          rightButton = false;
      }

      irr::s16 X, oldX;
      irr::s16 Y, oldY;
      irr::f32 wheelValue;
      bool leftButton;
      bool middleButton;
      bool rightButton;
      bool hasMoved;

  };

  class CInput : public irr::IEventReceiver
  {
  public:

    virtual bool OnEvent(const irr::SEvent& event)
    {
      printf("event ");
    }

    //
    // Mouse
    //

    inline bool isCursorVisible(){ return b_CursorVisible; }

    inline void setCursorVisible(bool gameCursor) { b_CursorVisible = gameCursor; }

    inline irr::core::position2d<irr::s32> getCursorPosition() {
      return irr::core::position2d<irr::s32>(Mouse.X, Mouse.Y); }

    bool isLeftButtonClicked(){ return Mouse.isLeftButtonClicked(); }
    bool isRightButtonClicked(){ return Mouse.isRightButtonClicked(); }
    bool isMiddleButtonClicked(){ return Mouse.isMiddleButtonClicked(); }
    bool isLeftButtonHeld(){ return Mouse.isLeftButtonHeld(); }
    bool isRightButtonHeld(){ return Mouse.isRightButtonHeld(); }
    bool isMiddleButtonHeld(){ return Mouse.isMiddleButtonHeld(); }
    bool hasMouseMoved(){ return Mouse.hasMoved; }
    irr::f32 getWheelValue(){ return Mouse.wheelValue; }

    //
    // Keyboard
    //

    bool isKeyHeldDown(irr::EKEY_CODE keycode){ return Key.isHeldDown(keycode); }
    bool isKeyPressedOnce(irr::EKEY_CODE keycode){ return Key.isPressedOnce(keycode); }

  protected:

    SMouse Mouse;
    SKey Key;

    irr::video::ITexture *m_CursorTexture;

    bool b_CursorVisible;
  };

}

#endif
