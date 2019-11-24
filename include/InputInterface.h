#ifndef INPUT_INTERFACE_HEADER_DEFINED
#define INPUT_INTERFACE_HEADER_DEFINED

namespace engine {

  class IInput
  {
    public:

    IInput() { }

    virtual void OnEvent(const irr::SEvent& event) { }
  };

}

#endif
