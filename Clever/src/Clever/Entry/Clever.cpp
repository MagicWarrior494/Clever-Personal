#include "Clever.h"

Clever::ManagerPointers Clever::managerpointers = Clever::ManagerPointers{};

Clever::Clever()
{

}

Clever::~Clever()
{
}

void Clever::init()
{
    std::cout << "Hello World!\n";
    //TODO -----------Desired Code----------------
    //
    window.reset(new Window::WindowManager{});
    managerpointers.window = &window;
    Window::WindowManager::WindowFlags windowFlags;
    windowFlags.width = 1440;
    windowFlags.height = 810;
    windowFlags.max_frames_in_flight = 2;
    windowFlags.DeveloperMode = true;
    window->WindowInit(windowFlags);
    //!        IE:
    //            Raytracing/Rasterizing
    //            Window Size
    //            Fullscreen
    //            VR/monitor
    //            Debug Mode on or off(Editor mode) Might have different levels
    //            etc
    //!        Usage:
    //            Completely sets up vulkan(Or whatever graphics API) and Window(OS independant)
    //            Allows for the manipulation and querying of camera
    //            etc
    //             After this call a screen opens and can view into "The world"
    //
    material.reset(new Material::MaterialManager{});
    managerpointers.material = &material;
    material->MaterialInit({ "Clever/Resource/Materials/stone.json" });
    //!        IE:
    //            File location of material types and material properties, and how they mix together
    //!        Usage:
    //            This defines the materials of the world and how they act and what they do. No objejct in the world can be made
    //            of a material not defined.
    //
    world.reset(new World::WorldManager{});
    managerpointers.world = &world;
    world->worldInit(managerpointers.window->get()->getVulkan());
    //!        IE:
    //            File location to load world Mesh and initial state
    //!        Usage:
    //            This allows for geometry to be loaded into the world<-- Important/Required
    //            Also can create/load geometry data that has been previous serializated.
    //            This is required for a camera to view the world as its also a piece of geometry that gets loaded/added automaticly.
    //            This Manages the ECS(Entity Component System)
    //
    events.reset(new Event::EventManager{});
    managerpointers.events = &events;
    events->Init(managerpointers.window->get()->getVulkan()->getGLFWwindow());
    //!       IE:
    //            File location for defined key press to function/ aciton
    //            also initilizes device location(s) for VR
    //            If debug mode is on from WindowInt then brings up Editor or based on mode
    //
    //!       Usage:
    //            Allows the definition of key pressed to be saved and modified
    //            This can also be reinitilized while running aka Hotswapping
    //
    //TODO     SystemInit(SystemFlags);
    //!        IE:
    //            File Location of physics and Magic definions
    //            Can be hotswapped
    //!        Usage:
    //            This initilized the Physics and Magic system.
    //            Without this the world is static and without motion
    //
    //TODO    Loop()
    while (!managerpointers.window->get()->shouldClose()) {
        events->UpdateKeyPresses();

        float time = events->getTime();
        float deltatime = time - lastTime;
        lastTime = time;

        //        Each in own thread, this will require more memory because objects will have mulitple versions(copies) and
        //        the different stages will work with what they got until data has been updated, changed or modified in some way.
        window->render(managerpointers.world->get()->getRenderables(), managerpointers.world->get()->getRenderablesSize(), deltatime);
        // !        Usage:
        //              Tells the Rendering API to render all data in the ECS

        world->update();
        // !        Usage:
        //              Updates all keyboard, mouse, controller position, head position, or other player controlled devices
        //TODO    Systems();
        //!        Usage:
        //              This updates the magic System and Phyisics system and others over all data.



        frameCounter++;
        if (time - counter >= 1)
        {
            std::cout << "FPS: " << frameCounter << std::endl;
            counter++;
            frameCounter = 0;
        }
    }

    window->cleanup(managerpointers.world->get()->getRenderables(), managerpointers.world->get()->getRenderablesSize());
    world->cleanup();
}