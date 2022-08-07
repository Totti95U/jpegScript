#include <lua.hpp>

struct Pixel_RGBA {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};

int jpegComp(lua_State* L) {
    Pixel_RGBA* pixels = reinterpret_cast<Pixel_RGBA*>(lua_touserdata(L, 1));
    int w = static_cast<int>(lua_tointeger(L, 2));
    int h = static_cast<int>(lua_tointeger(L, 3));
    double q = static_cast<double>(lua_tonumber(L, 4));
    bool comp_alpha = static_cast<bool>(lua_toboolean(L, 5));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = x + w * y;
            pixels[index].r = 0;
        }
    }

	return 0;
}

static luaL_Reg functions[] = {
    {"jpegComp", jpegComp},
    {nullptr, nullptr}
};

extern "C" {
    __declspec(dllexport) int luaopen_jpegComp(lua_State* L) {
        luaL_register(L, "jpegComp", functions);
        return 1;
    }
}