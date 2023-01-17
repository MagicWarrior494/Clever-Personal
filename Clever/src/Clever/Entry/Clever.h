#pragma once
#include <iostream>
#include "Clever/Window/WindowManager.h"
#include "Clever/WorldManager/WorldManager.h"
#include "Clever/Material/MaterialManager.h"
#include "Clever/EventSystem/EventManager.h"

class Clever
{
public:
    struct ManagerPointers
    {
        std::unique_ptr<World::WorldManager>* world = nullptr;
        std::unique_ptr<Window::WindowManager>* window = nullptr;
        std::unique_ptr<Material::MaterialManager>* material = nullptr;
        std::unique_ptr<Event::EventManager>* events = nullptr;
    };

public:
    Clever();
    ~Clever();
    void init();

private:
    std::unique_ptr<World::WorldManager> world;
    std::unique_ptr<Window::WindowManager> window;
    std::unique_ptr<Material::MaterialManager> material;
    std::unique_ptr<Event::EventManager> events;

    int counter = 0;
    int frameCounter = 0;
    float lastTime;

    static ManagerPointers managerpointers;
};


int main(){
    Clever clever{};
    clever.init();
}