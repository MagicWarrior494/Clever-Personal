#include <iostream>
#include "Clever/Window/WindowManager.h"
#include "Clever/Material/MaterialManager.h"
#include "Clever/WorldManager/WorldManager.h"

int main()
{
    std::cout << "Hello World!\n";
    //TODO -----------Desired Code----------------
    //    
    Window::WindowManager window{};
    window.WindowInit({1440, 810, 2});
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
    Material::MaterialManager materials{};
    materials.MaterialInit({"Clever/Resource/Materials/stone.json"});
    //!        IE:
    //            File location of material types and material properties, and how they mix together
    //!        Usage:
    //            This defines the materials of the world and how they act and what they do. No objejct in the world can be made
    //            of a material not defined.
    //
    World::WorldManager world{};
    world.worldInit(window.getVulkan());
    //!        IE:
    //            File location to load world Mesh and initial state
    //!        Usage:
    //            This allows for geometry to be loaded into the world<-- Important/Required
    //            Also can create/load geometry data that has been previous serializated.
    //            This is required for a camera to view the world as its also a piece of geometry that gets loaded/added automaticly.
    //            This Manages the ECS(Entity Component System)
    //
    //TODO     PlayerInputInit(PlayerInputFlags);
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
            while(!window.shouldClose()) {
    //        Each in own thread, this will require more memory because objects will have mulitple versions(copies) and
    //        the different stages will work with what they got until data has been updated, changed or modified in some way.
                window.render(world.getRenderables(), world.getRenderablesSize());
    // !        Usage:
    //              Tells the Rendering API to render all data in the ECS
    //TODO    Input();
    // !        Usage:
    //              Updates all keyboard, mouse, controller position, head position, or other player controlled devices
    //TODO    Systems();
    //!        Usage:
    //              This updates the magic System and Phyisics system and others over all data.
            }

            window.cleanup(world.getRenderables(), world.getRenderablesSize());
            world.cleanup();
            
}