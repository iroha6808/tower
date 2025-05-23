#ifndef TRAFFICLIGHT_HPP
#define TRAFFICLIGHT_HPP
#include "Turret.hpp"

class TrafficLight : public Turret {
public:
    static const int Price;
    TrafficLight(float x, float y);
    void CreateBullet() override;
    void Update(float deltaTime) override;
};
#endif  
