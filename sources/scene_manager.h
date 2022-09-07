#pragma once

#include <map>
#include <memory>

#include "Scene.h"
#include "graphics_pipeline.h"
#include "sprite_dissolve.h"
#include "practical_entities.h"

enum class DISSOLVE_TYPE
{
    NONE = -1,
    TYPE1,
    TYPE2,
    DOT,
    HORIZON,
};

class SceneManager : public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    SceneManager(GraphicsPipeline& graphics);
    ~SceneManager();
public:
    //--------< ŠÖ” >--------//
    void update(GraphicsPipeline& graphics, float elapsedTime);
    void render(GraphicsPipeline& graphics, float elapsedTime);
    void register_shadowmap(GraphicsPipeline& graphics, float elapsedTime);
    void clear(GraphicsPipeline& graphics);
    static void scene_switching(Scene* next, DISSOLVE_TYPE dissolve = DISSOLVE_TYPE::DOT, float d_speed = 1.5f)
    {
        storage_scene = next;
        dissolve_type = dissolve;
        dissolve_speed = d_speed;
    }
    void set_next_scene(Scene* next) { next_scene = next; }
private:
    //--------< •Ï” >--------//
    Scene* current_scene{};
    Scene* next_scene;
    static Scene* storage_scene;
    static DISSOLVE_TYPE dissolve_type;
    static float dissolve_speed;
    float threshold = 1.0f;
    int dissolve_state = 0;
    std::map<DISSOLVE_TYPE, SpriteDissolve*> dissolve_sprites;
    //--------< ŠÖ” >--------//
    void change_scene(GraphicsPipeline& graphics, Scene* s);
};