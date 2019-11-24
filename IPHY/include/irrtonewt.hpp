// Copyright (C) 2006 Mancuso Raffaele
// This file is part of the "IrrNewt library".
// For conditions of distribution and use, see copyright notice in IrrNewt.hpp

#ifndef RAFFAELE_MANCUSO_PHYSICS_IRRLICHT_LIB_IRRTONEWT_HPP_5416
#define RAFFAELE_MANCUSO_PHYSICS_IRRLICHT_LIB_IRRTONEWT_HPP_5416
#pragma once


//Define this constant if you are compiling IrrNewt with irrlicht version 1.3 or higher 
//Don't define this if your are using irrlicht 1.2 or less

//#define COMPILE_WITH_1_3



#include <irrlicht.h>

#define IRRNEWT_VER "0.4"


//!Convert a position from newton to irrlicht
const irr::f32 NewtonToIrr =  32.0f;

//!Convert a position from irrlicht to newton
const irr::f32 IrrToNewton = ( 1.0f / NewtonToIrr ) ;



//!gravity force on Earth
#define GRAVITY 9.80665f

//!gravity force on Moon
#define GRAVITY_MOON (GRAVITY/6.0f)

#define UNDEFINED_PARAMETER -999999

#endif
