/******************************************************************************
 * Copyright (C) 2012 Moritz K�hner, Germany.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************/
#ifndef Main_UserInterface_GuiImage_h
#define Main_UserInterface_GuiImagen_h

#include "GuiElement.h"
#include <lua.hpp>
#include <IGUIImage.h>

namespace Script
{
    class LuaEngine;
}

namespace Gui
{

    class GuiImage: public GuiElement
    {

    public:
        GuiImage(GuiPlugin* plugin, Script::LuaEngine* engine, lua_State* plua);

        ~GuiImage();

        //---- Lua Constants ------

        static const char* lua_libName;

        static const struct luaL_reg lua_lib_m[];

        static const struct luaL_reg lua_lib_f[];

        static const struct luaL_reg lua_lib_p[];

    private:
        //---- Lua funktions ------

        static int luaNew(lua_State* pLua);

        static int luaUseAlpha(lua_State* pLua);

        static int luaScaled(lua_State* pLua);

        static int luaSetColor(lua_State* pLua);

        static int luaSetTextur(lua_State* pLua);

        static GuiImage* lua_toGuiImage(lua_State* pLua,int index);

    };

}

#endif
