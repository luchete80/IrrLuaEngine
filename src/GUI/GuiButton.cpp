#include "GuiButton.h"
#include "LuaEngine.h"
#include "IrrlichtDevice.h"
#include "IGUIEnvironment.h"


namespace Gui {

    const char* GuiButton::lua_libName = "Button";

    const char* GuiButton::lua_metatableName = "Lua.Button";

    const struct luaL_reg GuiButton::lua_lib_m [] = {
        {"__gc",lua_GC},
        {"__index",lua_index},
        {"__newindex",lua_newindex},
        {NULL, NULL}  /* sentinel */
    };

    const struct luaL_reg GuiButton::lua_lib_f [] = {
        {"new",lua_new},
        {NULL, NULL}  /* sentinel */
    };

    void GuiButton::createMatatable(lua_State* pLua)
    {
        luaL_newmetatable(pLua, lua_metatableName);
    
        //lua_pushstring(pLua, "__index");
        //lua_pushvalue(pLua, -2);  /* pushes the metatable */
        //lua_settable(pLua, -3);  /* metatable.__index = metatable */

        ///* set its __gc field */
        //lua_pushstring(pLua, "__gc");
        //lua_pushcfunction(pLua, lua_GC);
        //lua_settable(pLua, -3);
    
        luaL_openlib(pLua, NULL, lua_lib_m, 0);
        luaL_openlib(pLua, lua_libName, lua_lib_f, 0);
    }


    GuiButton::GuiButton(Script::LuaEngine* engine, GuiElement* parent, lua_State* plua):GuiElement(engine,parent,plua)
    {
        m_irrElement = NULL;
    }
    
    GuiButton::~GuiButton()
    {
        printf("GuiButton::~GuiButton\n");
        if(m_irrElement)
        {
            m_irrElement->drop();
            m_irrElement = NULL;
        }
    }

        const luaL_reg* GuiButton::getMethods()
    {
        return lua_lib_m;
    }
        
    const luaL_reg* GuiButton::getFunktions()
    {
        return lua_lib_f;
    }

    const char* GuiButton::getTableName()
    {
        return lua_libName;
    }

    const char* GuiButton::getMetaTableName()
    {
        return lua_metatableName;
    }


    int GuiButton::lua_GC(lua_State* pLua)
    {
        printf("GuiButton::lua_GC\n");
        GuiButton* pthis = *static_cast<GuiButton**>(luaL_checkudata(pLua, 1, lua_metatableName));
        
        pthis->onLuaGC();

        return 0;
    }

    int GuiButton::lua_new(lua_State* pLua)
    {
        Script::LuaEngine* engine = Script::LuaEngine::getThisPointer<Script::LuaEngine>(pLua);
        GuiButton* pthis = new GuiButton(engine,NULL,pLua);

        pthis->pushToStack();

        pthis->drop();

        int x = lua_tonumber(pLua,1);
        int y = lua_tonumber(pLua,2);
        int w = lua_tonumber(pLua,3);
        int h = lua_tonumber(pLua,4);

        size_t len;
        const char* text    = luaL_optlstring(pLua,5,"",&len);
        wchar_t* textw      =  new wchar_t[++len];
        mbstowcs(textw, text, len);
        const char* tooltip = luaL_optlstring(pLua,6,"",&len);
        wchar_t* tooltipw      =  new wchar_t[++len];
        mbstowcs(tooltipw, tooltip, len);



        irr::gui::IGUIElement* b = (irr::gui::IGUIElement*) 
            engine->getIrrlichtDevice()->getGUIEnvironment()->addButton(
                irr::core::recti(x,y,x+w,y+h),
                NULL,
                pthis->getId(),
                textw,
                tooltipw
            );

        pthis->m_irrElement = b;


        delete textw;
        delete tooltipw;


        return 1;
    }

    
    int GuiButton::lua_index(lua_State* pLua)
    {
        GuiButton* pthis = *static_cast<GuiButton**>(luaL_checkudata(pLua, 1, lua_metatableName));
        const char* key = luaL_checkstring(pLua, 2);

        if(!strcmp("toolTip",key))
        {
            const irr::core::stringw& wtip = pthis->m_irrElement->getToolTipText();
            char* tip = new char[wtip.size()+1];
            wcstombs(tip,wtip.c_str(),wtip.size()+1);
            lua_pushstring(pLua,tip);
            delete tip;
            return 1;
        }
        else if(!strcmp("text",key))
        {
            const irr::core::stringw& wtip = pthis->m_irrElement->getText();
            char* tip = new char[wtip.size()+1];
            wcstombs(tip,wtip.c_str(),wtip.size()+1);
            lua_pushstring(pLua,tip);
            delete tip;
            return 1;
        }


        pthis->onLuaIndex();
        return 1;
    }


    int GuiButton::lua_newindex(lua_State* pLua)
    {
        GuiButton* pthis = *static_cast<GuiButton**>(luaL_checkudata(pLua, 1, lua_metatableName));
        const char* key = luaL_checkstring(pLua, 2);

        if(!strcmp("toolTip",key))
        {
            const char* text = luaL_checkstring(pLua, 3);
            irr::core::stringw wtext(text);
            pthis->m_irrElement->setToolTipText(wtext.c_str());

            return 0;
        }
        else if(!strcmp("text",key))
        {
            const char* text = luaL_checkstring(pLua, 3);
            irr::core::stringw wtext(text);
            pthis->m_irrElement->setText(wtext.c_str());

            return 0;
        }


        pthis->onLuaNewIndex();
        return 0;
    }

    void GuiButton::draw()
    {
        if(m_irrElement)
        {
            m_irrElement->draw();
        }
    }
   

} /* End of namespace Gui */

