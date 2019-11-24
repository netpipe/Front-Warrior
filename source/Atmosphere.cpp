#include "Atmosphere.h"
#include "Maths.h"
#include "Renderer.h"
#include "Core.h"

using namespace engine;

CAtmosphereManager::CAtmosphereManager(engine::CCore* core) : Core(core), terrainRoughness(0.03), tempKelvins(293.15), pressure(1010.f), lat(68.89123063152088), lon(144.7119140625)
{
    float tempLn;
    for (irr::u32 i=0; i<128; i++) {
        fronts[i].pos = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f);
        fronts[i].direction = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f)/200000.f;
        tempLn = pow(fronts[i].pos.getLength()/7071.0678,0.25);
        fronts[i].pressure = float(Core->getMath()->getRandomInt(pressure*100.f-50000.f*tempLn,100.f*pressure+3000.f*tempLn))*0.01;
        fronts[i].temperature = float(Core->getMath()->getRandomInt((tempKelvins-50)*100.f,100.f*(tempKelvins+37)))*0.01;
    }
    lastTime = -1;
    float perFrontTemp;
    for (irr::u32 i=0; i<128; i++) {
        perFrontTemp = 143500.f*(fronts[i].temperature+tempKelvins);
        windDirAndSpeedPckd.X += 0.064285714*(fronts[i].pos.Y*perFrontTemp*log(pressure/fronts[i].pressure))/(0.00014584*sin(irr::core::degToRad(lat))*pow(1000.f*fronts[i].pos.getLength(),2.0));
        windDirAndSpeedPckd.Z += 0.064285714*(fronts[i].pos.X*perFrontTemp*log(fronts[i].pressure/pressure))/(0.00014584*sin(irr::core::degToRad(lat))*pow(1000.f*fronts[i].pos.getLength(),2.0));
    }
    windDirAndSpeedPckd.Y = 0.0;
    speed = windDirAndSpeedPckd.getLength()+0.7f;
    windDirAndSpeedPckd.normalize();
    windDirAndSpeedPckd.Y = speed;
}

void CAtmosphereManager::setRoughness(float roughClass) {
    if (roughClass<0.f||roughClass>4.f)
        return;
    else if (roughClass<=0.5f)
        terrainRoughness = 0.0002*(1.0-roughClass*2.0)+0.0024*roughClass*2.0;
    else if (roughClass<=1.f)
        terrainRoughness = 0.0024*(1.0-roughClass*2.0+1.0)+0.03*(roughClass-0.5)*2.0;
    else if (roughClass<=1.5f)
        terrainRoughness = 0.03*(1.0-roughClass*2.0+2.0)+0.055*(1.0-roughClass)*2.0;
    else if (roughClass<=2.f)
        terrainRoughness = 0.055*(1.0-roughClass*2.0+3.0)+0.1*(1.5-roughClass)*2.0;
    else if (roughClass<=3.f)
        terrainRoughness = 0.1*(1.0-roughClass+2.0)+0.8-0.4*roughClass;
    else if (roughClass<=4.f)
        terrainRoughness = 0.4*(1.0-roughClass+3.0)+4.8-1.6*roughClass;
}

float CAtmosphereManager::getMsecAtAltitude(float altitude) {
    return (speed*log(28000.f/terrainRoughness)/log(10.f/terrainRoughness))*log(altitude/terrainRoughness)/log(28000.f/terrainRoughness);
}

void CAtmosphereManager::moveFront(irr::u32 index, irr::u32 otherIndex, irr::u32 dTime) {
    float distance = (fronts[index].pos-fronts[otherIndex].pos).getLength();
    fronts[index].pos += fronts[index].direction*dTime;
    fronts[otherIndex].pos += fronts[otherIndex].direction*dTime;
    float distAfter = (fronts[index].pos-fronts[otherIndex].pos).getLength();
    irr::core::vector2df direction;
    direction.X += 0.001*(abs(fronts[index].pos.Y-fronts[otherIndex].pos.Y)*143500.f*(fronts[index].temperature+fronts[otherIndex].temperature)*log(fronts[index].pressure/fronts[otherIndex].pressure))/(0.00014584*sin(irr::core::degToRad(lat)+fronts[index].pos.Y/6357.f)*pow(1000.f*distAfter,2.0));
    direction.Y += 0.001*(abs(fronts[index].pos.X-fronts[otherIndex].pos.X)*143500.f*(fronts[index].temperature+fronts[otherIndex].temperature)*log(fronts[otherIndex].pressure/fronts[index].pressure))/(0.00014584*sin(irr::core::degToRad(lat)+fronts[index].pos.Y/6357.f)*pow(1000.f*distAfter,2.0));
    fronts[index].direction += direction;
    fronts[otherIndex].direction -= direction;
    if (distance!=0.0) {
        fronts[index].pressure += (fronts[otherIndex].pressure-fronts[index].pressure)*irr::core::max_(0.0,1.0-distAfter/distance);
        fronts[otherIndex].pressure += (fronts[index].pressure-fronts[otherIndex].pressure)*irr::core::max_(0.0,1.0-distAfter/distance);
        fronts[index].temperature += (fronts[otherIndex].temperature-fronts[index].temperature)*irr::core::max_(0.0,1.0-distAfter/distance);
        fronts[otherIndex].temperature += (fronts[index].temperature-fronts[otherIndex].temperature)*irr::core::max_(0.0,1.0-distAfter/distance);
    }
    if (fronts[index].pos.getLength()>7000.f) {
        float tempLn;
        fronts[index].pos = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f);
        fronts[index].direction = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f)/200000.f;
        tempLn = pow(fronts[index].pos.getLength()/7071.0678,0.25);
        fronts[index].pressure = float(Core->getMath()->getRandomInt(pressure*100.f-50000.f*tempLn,100.f*pressure+3000.f*tempLn))*0.01;
        fronts[index].temperature = float(Core->getMath()->getRandomInt((tempKelvins-50)*100.f,100.f*(tempKelvins+37)))*0.01;
    }
    if (fronts[otherIndex].pos.getLength()>7000.f) {
        float tempLn;
        fronts[otherIndex].pos = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f);
        fronts[otherIndex].direction = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f)/200000.f;
        tempLn = pow(fronts[otherIndex].pos.getLength()/7071.0678,0.25);
        fronts[otherIndex].pressure = float(Core->getMath()->getRandomInt(pressure*100.f-50000.f*tempLn,100.f*pressure+3000.f*tempLn))*0.01;
        fronts[otherIndex].temperature = float(Core->getMath()->getRandomInt((tempKelvins-50)*100.f,100.f*(tempKelvins+37)))*0.01;
    }
}

void CAtmosphereManager::update()
{
    irr::u32 dTime = Core->time.total - lastTime;

    if (lastTime==-1)
      dTime = 0.f;

    float perFrontTemp;
    irr::core::vector3df tempwindDS;

    for (irr::u32 i=0; i<128; i++)
    {
        perFrontTemp = 143500.f*(fronts[i].temperature+tempKelvins);
        tempwindDS.X += 0.064285714*(fronts[i].pos.Y*perFrontTemp*log(pressure/fronts[i].pressure))/(0.00014584*sin(irr::core::degToRad(lat))*pow(1000.f*fronts[i].pos.getLength(),2.0));
        tempwindDS.Z += 0.064285714*(fronts[i].pos.X*perFrontTemp*log(fronts[i].pressure/pressure))/(0.00014584*sin(irr::core::degToRad(lat))*pow(1000.f*fronts[i].pos.getLength(),2.0));
    }

    windDirAndSpeedPckd.X = windDirAndSpeedPckd.X*0.99999+tempwindDS.X*0.00001;
    windDirAndSpeedPckd.Z = windDirAndSpeedPckd.Z*0.99999+tempwindDS.Z*0.00001;
    windDirAndSpeedPckd.Y = 0.0;
    speed = speed*0.0125f+(windDirAndSpeedPckd.getLength()+0.7f)*0.9875f;
    windDirAndSpeedPckd.normalize();
    windDirAndSpeedPckd.Y = speed;
    irr::u32 randomNum1,randomNum2;

    for (irr::u32 i=0; i<128; i++)
    {
        randomNum1 = Core->getMath()->getRandomInt(0,127);
        randomNum2 = Core->getMath()->getRandomInt(0,127);
        if (randomNum1==randomNum2) {
            i--;
            continue;
        }
        moveFront(randomNum1,randomNum2,dTime);
    }

    if(speed!=speed)
    {
    // little not a number check
        float tempLn;
        for (irr::u32 i=0; i<128; i++)
        {
            fronts[i].pos = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f);
            fronts[i].direction = irr::core::vector2df(float(Core->getMath()->getRandomInt(0,10000))-5000.f,float(Core->getMath()->getRandomInt(0,10000))-5000.f)/200000.f;
            tempLn = pow(fronts[i].pos.getLength()/7071.0678,0.25);
            fronts[i].pressure = float(Core->getMath()->getRandomInt(pressure*100.f-50000.f*tempLn,100.f*pressure+3000.f*tempLn))*0.01;
            fronts[i].temperature = float(Core->getMath()->getRandomInt((tempKelvins-50)*100.f,100.f*(tempKelvins+37)))*0.01;
        }
    }

    lastTime=Core->getRenderer()->getTimer()->getTime();
}
