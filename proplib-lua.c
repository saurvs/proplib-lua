#include <lua.h>
#include <lauxlib.h>
#include <prop/proplib.h>
#include <stdlib.h>

#define PROP_ARRAY_MT "prop.array"
#define PROP_DICT_MT "prop.dict"
#define IDX_RANGE_ERR "index out of range"

static prop_object_t arg_to_prop_object(lua_State *L, int arg) {
    prop_object_t obj;
    
    switch (lua_type(L, arg)) {
	case LUA_TNUMBER:
	    obj = prop_number_create_integer(lua_tonumber(L, arg));
	    break;
	case LUA_TSTRING:
	    obj = prop_string_create_cstring(lua_tostring(L, arg));
	    break;
	case LUA_TBOOLEAN:
	    obj = prop_bool_create(lua_toboolean(L, arg));
	    break;
	default:
	    luaL_argerror(L, arg, "type not supported");
	    break;
    }
    
    return obj;
}

static void push_arg_using_prop_object(lua_State *L, prop_object_t obj) {
    char *str;
    
    switch (prop_object_type(obj)) {
	case PROP_TYPE_NUMBER:
	    lua_pushinteger(L, prop_number_integer_value(obj));
	    break;
	case PROP_TYPE_STRING:
	    str = prop_string_cstring(obj);
	    lua_pushstring(L, str);
	    free(str);
	    break;
	case PROP_TYPE_BOOL:
	    lua_pushboolean(L, prop_bool_true(obj));
	    break;
	default:
	    break;
    }
}

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
    prop_object_t obj;
    int i;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    i = lua_tonumber(L, 2);
    luaL_argcheck(L,
	i >= 0 && i <= prop_array_count(*array),
	2, IDX_RANGE_ERR);
    obj = arg_to_prop_object(L, 3);
    prop_array_set(*array, i, obj);
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
    push_arg_using_prop_object(L, prop_array_get(*array, i));
    return 1;
}

static int array_append(lua_State *L) {
    prop_array_t *array;
    prop_object_t obj;
    
    array = luaL_checkudata(L, 1, PROP_ARRAY_MT);
    obj = arg_to_prop_object(L, 2);
    prop_array_add(*array, obj);
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

static int dict_new(lua_State *L) {
    prop_dictionary_t *dict;
    
    dict = lua_newuserdata(L, sizeof(prop_dictionary_t));
    *dict = prop_dictionary_create();
    luaL_getmetatable(L, PROP_DICT_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int dict_from_file(lua_State *L) {
    prop_dictionary_t *dict;
    
    dict = lua_newuserdata(L, sizeof(prop_dictionary_t));
    *dict = prop_dictionary_internalize_from_file(luaL_checkstring(L, 1));
    luaL_getmetatable(L, PROP_DICT_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int dict_from_xml(lua_State *L) {
    prop_dictionary_t *dict;
    
    dict = lua_newuserdata(L, sizeof(prop_dictionary_t));
    *dict = prop_dictionary_internalize(luaL_checkstring(L, 1));
    luaL_getmetatable(L, PROP_DICT_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int dict_set(lua_State *L) {
    const char *key;
    prop_dictionary_t *dict;
    prop_object_t obj;
    
    dict = luaL_checkudata(L, 1, PROP_DICT_MT);
    key = luaL_checkstring(L, 2);
    obj = arg_to_prop_object(L, 3);
    prop_dictionary_set(*dict, key, obj);
    return 0;
}

static int dict_get(lua_State *L) {
    prop_dictionary_t *dict;
    const char *key;
    int n;
    
    dict = luaL_checkudata(L, 1, PROP_DICT_MT);
    key = luaL_checkstring(L, 2);
    push_arg_using_prop_object(L, prop_dictionary_get(*dict, key));
    return 1;
}

static int dict_remove(lua_State *L) {
    prop_dictionary_t *dict;
    const char *key;
    
    dict = luaL_checkudata(L, 1, PROP_DICT_MT);
    key = luaL_checkstring(L, 2);
    prop_dictionary_remove(*dict, key);
    return 0;
}

static int dict_count(lua_State *L) {
    prop_dictionary_t *dict;
    
    dict = luaL_checkudata(L, 1, PROP_DICT_MT);
    lua_pushinteger(L, prop_dictionary_count(*dict));
    return 1;
}

static int dict_keys(lua_State *L) {
    prop_array_t *array;
    prop_dictionary_t *dict;
    
    dict = luaL_checkudata(L, 1, PROP_DICT_MT);
    array = lua_newuserdata(L, sizeof(prop_array_t));
    *array = prop_dictionary_all_keys(*dict);
    luaL_getmetatable(L, PROP_ARRAY_MT);
    lua_setmetatable(L, -2);
    return 1;
}

static int dict_to_file(lua_State *L) {
    prop_dictionary_t *dict;
    
    dict = lua_touserdata(L, 1);
    prop_dictionary_externalize_to_file(*dict, luaL_checkstring(L, 2));
    return 0;
}

static int dict_to_xml(lua_State *L) {
    prop_dictionary_t *dict;
    char *xml;
    
    dict = lua_touserdata(L, 1);
    xml = prop_dictionary_externalize(*dict);
    lua_pushstring(L, xml);
    free(xml);
    return 1;
}

static const struct luaL_Reg dict_methods[] = {
    {"set", dict_set},
    {"get", dict_get},
    {"remove", dict_remove},
    {"count", dict_count},
    {"keys", dict_keys},
    {"to_file", dict_to_file},
    {"to_xml", dict_to_xml},
    {NULL, NULL}
};

static const struct luaL_Reg proplib[] = {
    {"new_array", array_new},
    {"array_from_file", array_from_file},
    {"array_from_xml", array_from_xml},
    
    {"new_dict", dict_new},
    {"dict_from_file", dict_from_file},
    {"dict_from_xml", dict_from_xml},
    {NULL, NULL}
};

int luaopen_proplib(lua_State *L) {
    luaL_newmetatable(L, PROP_ARRAY_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, array_methods, 0);
    
    luaL_newmetatable(L, PROP_DICT_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, dict_methods, 0);
    
    luaL_newlib(L, proplib);
    return 1;
}