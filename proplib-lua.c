#include <lua.h>
#include <lauxlib.h>
#include <prop/proplib.h>
#include <stdlib.h>

#define PROP_ARRAY_MT "prop.array"
#define IDX_RANGE_ERR "index out of range"

static int array_new(lua_State *L) {
    prop_array_t *array;
    
    array = lua_newuserdata(L, sizeof(prop_array_t));
    *array = prop_array_create();
    luaL_getmetatable(L, PROP_ARRAY_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int array_from_file(lua_State *L) {
    prop_array_t *array;
    
    array = lua_newuserdata(L, sizeof(prop_array_t));
    *array = prop_array_internalize_from_file(luaL_checkstring(L, 1));
    luaL_getmetatable(L, PROP_ARRAY_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int array_from_xml(lua_State *L) {
    prop_array_t *array;
    
    array = lua_newuserdata(L, sizeof(prop_array_t));
    *array = prop_array_internalize(luaL_checkstring(L, 1));
    luaL_getmetatable(L, PROP_ARRAY_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int array_set(lua_State *L) {
    prop_array_t *array;
    int i, n;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    i = lua_tonumber(L, 2);
    luaL_argcheck(L,
	i >= 0 && i <= prop_array_count(*array),
	2, IDX_RANGE_ERR);
    n = lua_tonumber(L, 3);
    prop_array_set(*array, i, prop_number_create_integer(n));
    return 0;
}

static int array_get(lua_State *L) {
    prop_array_t *array;
    int i, n;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    i = lua_tonumber(L, 2);
    luaL_argcheck(L,
	i >= 0 && i < prop_array_count(*array),
	2, IDX_RANGE_ERR);
    n = prop_number_integer_value(prop_array_get(*array, i));
    lua_pushinteger(L, n);
    return 1;
}

static int array_append(lua_State *L) {
    prop_array_t *array;
    int n;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    n = lua_tonumber(L, 2);
    prop_array_add(*array, prop_number_create_integer(n));
    return 0;
}

static int array_remove(lua_State *L) {
    prop_array_t *array;
    int i;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    i = lua_tonumber(L, 2);
    luaL_argcheck(L,
	i >= 0 && i < prop_array_count(*array),
	2, IDX_RANGE_ERR);
    prop_array_remove(*array, i);
    return 0;
}

static int array_count(lua_State *L) {
    prop_array_t *array;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    lua_pushinteger(L, prop_array_count(*array));
    return 1;
}

static int array_to_file(lua_State *L) {
    prop_array_t *array;
    
    array = lua_touserdata(L, 1);
    prop_array_externalize_to_file(*array, luaL_checkstring(L, 2));
    return 0;
}

static int array_to_xml(lua_State *L) {
    prop_array_t *array;
    char *xml;
    
    array = lua_touserdata(L, 1);
    xml = prop_array_externalize(*array);
    lua_pushstring(L, xml);
    free(xml);
    return 1;
}

static const struct luaL_Reg proplib[] = {
    {"new_array", array_new},
    {"array_from_file", array_from_file},
    {"array_from_xml", array_from_xml},
    {NULL, NULL}
};

static const struct luaL_Reg array_methods[] = {
    {"set", array_set},
    {"get", array_get},
    {"append", array_append},
    {"remove", array_remove},
    {"count", array_count},
    {"to_file", array_to_file},
    {"to_xml", array_to_xml},
    {NULL, NULL}
};

int luaopen_proplib(lua_State *L) {
    luaL_newmetatable(L, PROP_ARRAY_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, array_methods, 0);
    luaL_newlib(L, proplib);
    return 1;
}