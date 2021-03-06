#include "LuaEngine.h"
#include "LuaObject.h"
#include "IrrlichtDevice.h"
#include "IGUIEnvironment.h"
#include "ILuaEnginePlugin.h"
#include <cstdio>

namespace Script
{

    const struct luaL_reg LuaEngine::lua_globls[] =
    {
    { "suspend", lua_Suspend },
    { NULL, NULL } /* sentinel */
    };

    const char* LuaEngine::Lua_Object_Key = "engineKey";

    LuaEngine::LuaEngine(irr::IrrlichtDevice* irr)
    {
        m_framecount = 0;
        m_device = irr;
        m_lua = lua_open();
        luaL_openlibs(m_lua);

        lua_atpanic(m_lua, lua_AtPanic);

        lua_getglobal(m_lua, "debug");
        lua_getfield(m_lua, -1, "traceback");
        m_errorhandler = lua_gettop(m_lua);

        /* store a number */
        lua_pushlightuserdata(m_lua, (void *) Lua_Object_Key); /* push address */
        lua_pushlightuserdata(m_lua, (void *) this); /* push value */
        /* registry[&Key] = myNumber */
        lua_settable(m_lua, LUA_REGISTRYINDEX);

        luaL_newmetatable(m_lua, LuaObject::Lua_Object_Metatable);
        luaL_openlib(m_lua, NULL, LuaObject::s_methods, 0);
    }

    LuaEngine::~LuaEngine()
    {
        lua_close(m_lua);
        for (irr::u32 i = 0; i < m_plugins.size(); i++)
        {
            delete m_plugins[i];
        }
    }

    void LuaEngine::run()
    {
    	m_framecount++;
    	if(m_yieldlist_frame.size())
    	{
			const YieldState& i = m_yieldlist_frame.top();

			if(m_framecount >= i.m_value)
			{
				YieldState copy = i;
				m_yieldlist_frame.pop();
				resumeState(copy);
			}
    	}

    	if(m_yieldlist_gametime.size())
    	{
			const YieldState& i = m_yieldlist_gametime.top();

			if(m_device->getTimer()->getTime() >= i.m_value)
			{
				YieldState copy = i;
				m_yieldlist_gametime.pop();
				resumeState(copy);
			}
    	}

        if(m_yieldlist_realtime.size())
    	{
			const YieldState& i = m_yieldlist_realtime.top();

			if(m_device->getTimer()->getRealTime() >= i.m_value)
			{
				YieldState copy = i;
				m_yieldlist_realtime.pop();
				resumeState(copy);
			}
    	}

    }

    void LuaEngine::addPlugin(ILuaEnginePlugin* p)
    {
        m_plugins.push_back(p);
    }

    LuaEngine* LuaEngine::getThisPointer(lua_State* pLua)
    {
        lua_pushlightuserdata(pLua, (void *) Lua_Object_Key); /* push address */
        lua_gettable(pLua, LUA_REGISTRYINDEX); /* retrieve value */
        LuaEngine* e = (LuaEngine*) lua_touserdata(pLua, -1);
        lua_pop(pLua, 1);
        return e;
    }

    void LuaEngine::runFile(const irr::core::stringc& file)
    {
        luaL_loadfile(m_lua, file.c_str());

        int error = lua_pcall(m_lua, 0, 0, m_errorhandler);

        if (error)
        {
            const char* c = lua_tostring(m_lua, -1);
            printf("Error in runing Script! %i: \n%s\n", error, c);
        }
    }

    void LuaEngine::init()
    {
        lua_pushvalue(m_lua, LUA_GLOBALSINDEX);

        luaL_register(m_lua, NULL, lua_globls);

        for (irr::u32 i = 0; i < m_plugins.size(); i++)
        {
            m_plugins[i]->registerFunktions(m_lua);
        }
    }

    int LuaEngine::doCall(int args, int rets)
    {
        YieldState state;
        state.m_thread = lua_newthread(m_lua);
        state.m_refkey = luaL_ref(m_lua, LUA_REGISTRYINDEX);

        lua_xmove(m_lua, state.m_thread, args + 1);

        return resumeState(state);
    }

    irr::IrrlichtDevice* LuaEngine::getIrrlichtDevice()
    {
        return m_device;
    }

    bool LuaEngine::OnEvent(const irr::SEvent& e)
    {
        bool b = false;
        for (irr::u32 i = 0; i < m_plugins.size(); i++)
        {
            b = m_plugins[i]->OnEvent(e) || b;
        }
        return b;
    }

    int LuaEngine::lua_Suspend(lua_State* pLua)
    {
        if (lua_gettop(pLua) == 0)
        {
            lua_pushstring(pLua, "f");
            lua_pushnumber(pLua, 1);
        }
        else if (lua_gettop(pLua) == 1)
        {
            luaL_checknumber(pLua, 1);
            lua_pushstring(pLua, "g");
            lua_insert(pLua, 1);
        }
        else
        {
            luaL_checkstring(pLua, 1);
            luaL_checknumber(pLua, 2);
            lua_settop(pLua, 2);
        }

        return lua_yield(pLua, 2);
    }

    int LuaEngine::lua_AtPanic(lua_State* pLua)
    {
        const char* c = lua_tostring(pLua, -1);
        printf("Master VM Panic! This is facemelt: \n%s\n", c);
        stackdump(pLua);
        return 0;
    }

    int LuaEngine::resumeState(YieldState& state)
    {
        int args = lua_gettop(state.m_thread) - 1;
        int error = lua_resume(state.m_thread, args);

        if (error && error != LUA_YIELD)
        {
            lua_getglobal(state.m_thread, "debug");
            lua_getfield(state.m_thread, -1, "traceback");
            lua_remove(state.m_thread, -2);
            lua_insert(state.m_thread, -2);
            lua_call(state.m_thread, 1, 1);
            const char* c = lua_tostring(state.m_thread, -1);
            printf("Error in runing Script! %i: \n%s\n", error, c);
        }
        if (error == LUA_YIELD)
        {
            const char* mode =  luaL_checkstring(state.m_thread, -2);
            state.m_value    =  luaL_checknumber(state.m_thread, -1);

            switch(*mode)
            {
                case 'f':
                    state.m_value += m_framecount;
                    m_yieldlist_frame.push(state);
                    break;
                case 'g':
                    state.m_value += m_device->getTimer()->getTime();
                    m_yieldlist_gametime.push(state);
                    break;
                case 'r':
                    state.m_value += m_device->getTimer()->getRealTime();
                    m_yieldlist_realtime.push(state);
                    break;
            }
            
            return LUA_YIELD;
        }

        luaL_unref(m_lua, LUA_REGISTRYINDEX, state.m_refkey);

        return error;
    }

    lua_State* LuaEngine::getLuaState() const
    {
        return m_lua;
    }

    void LuaEngine::stackdump(lua_State* l)
    {
        int i;
        int top = lua_gettop(l);

        printf("total in stack %d\n", top);

        for (i = 1; i <= top; i++)
        { /* repeat for each level */
            int t = lua_type(l, i);
            printf("  "); /* put a separator */
            switch (t)
            {
                case LUA_TSTRING: /* strings */
                    printf("string: '%s'\n", lua_tostring(l, i));
                    break;
                case LUA_TBOOLEAN: /* booleans */
                    printf("boolean %s\n", lua_toboolean(l, i) ? "true" : "false");
                    break;
                case LUA_TNUMBER: /* numbers */
                    printf("number: %g\n", lua_tonumber(l, i));
                    break;
                default: /* other values */
                    printf("%s\n", lua_typename(l, t));
                    break;
            }

        }
        printf("\n"); /* end the listing */
    }

}
