#include "lua-bindings.hpp"

namespace lua {
    namespace types {
        template <> const char* ctype_name<const bool>() { return "boolean"; }

        AnyRef::AnyRef()
            : L(nullptr), ref(LUA_NOREF)
        {}

        AnyRef::~AnyRef() {
            if (L) {
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
            }
        }

        AnyRef::AnyRef(AnyRef&& other)
            : L(std::move(other.L)), ref(std::move(other.ref))
        {
            other.L = 0; other.ref = 0;
        }

        AnyRef& AnyRef::operator=(AnyRef&& other) {
            L = std::move(other.L);
            ref = std::move(other.ref);
            other.L = 0;
            other.ref = 0;
            return *this;
        }

        void AnyRef::push(lua_State* L) const {
            assert(L == nullptr || L == this->L);
            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        }

        void AnyRef::pop(lua_State* L) {
            if (this->L)
                luaL_unref(this->L, LUA_REGISTRYINDEX, ref);
            this->L = L;
            ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }
    }

    // ------------------------------------------------------------
    //

    Push::Push(lua_State* L)
        : L(L)
    {}

    void Push::operator()(const types::AnyRef& x) {
        x.push(L);
    }

    void Push::operator()(const types::Nil&) {
        lua_pushnil(L);
    }

    void Push::operator()(const char* x) {
        lua_pushstring(L, x);
    }

    void Push::operator()(const std::string& x) {
        lua_pushstring(L, x.c_str());
    }

    template <> void Push::operator()<bool>(const bool& x) {
        lua_pushboolean(L, x);
    }

    //
    // ------------------------------------------------------------
    //
    Pop::Pop(lua_State* L, int n, e_mode mode)
        : L(L), n(n), mode(mode), err(false)
    {}

    void Pop::pop(lua_State* L) {
        lua_pop(L, 1);
        n--;
    }

    void Pop::operator()(std::string& x) {
        int ltype = lua_type(L, -1);
        switch (ltype) {
        case LUA_TBOOLEAN:
            x = (lua_toboolean(L, -1) == 1) ? "true" : "false";
            break;
        case LUA_TSTRING:
        case LUA_TNUMBER:
            x = lua_tostring(L, -1);
            break;
        case LUA_TNIL:
            x = "nil";
            break;
        default:
            raise_error<std::string>(ltype);
        }
        pop(L);
    }

    void Pop::operator()(types::AnyRef& r) {
        r.pop(L);
        n--;
    }

    template <> void Pop::operator()<bool>(bool& x) {
        int x1;
        pop_number_or_bool(x1);
        x = (x1 == 1);
    }

    // Error handler that appends a stack trace to the error message
    int build_stack_trace_error_func(lua_State* L) {
        int top = lua_gettop(L);
        if (!lua_isstring(L, top))  
            return 1;

        // get globals table (+1)
        lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
        lua_getfield(L, -1, "debug"); // (+2)
        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return 1;
        }

        lua_getfield(L, -1, "traceback"); // (+3)
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 3);
            return 1;
        }
        lua_pushvalue(L, top);  // original error message
        lua_pushinteger(L, 2);  // don't include this call in the stacktrace
        lua_call(L, 2, 1);  
        return 1;
    }

    //
    // ------------------------------------------------------------
    //

    State::State() {
        L = luaL_newstate();
        if (!L) // only NULL if memory error
            throw std::bad_alloc();
        luaL_openlibs(L);
    }

    State::~State() {
        lua_close(L);
    }

    void State::doFile(const std::string& file) {
        if (luaL_dofile(L, file.c_str())) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            throw Exception(error);
        }
    }

    void State::doString(const std::string& str) {
       if (luaL_dostring(L, str.c_str())) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            throw Exception(error);
        }
    }

    void State::pcall(int nargs, int nresults) {
        pcall(nargs, nresults, 0);
    }

    void State::pcall(int nargs, int nresults, int err_function) {
        if (lua_pcall(L, nargs, nresults, err_function)) {
            const char* err = lua_tostring(L, -1);
            std::string error = err != nullptr ? err : "unknown script error";
            lua_pop(L, 1);
            throw Exception(error);
        }
    }

    bool State::hasFunction(const char* f) {
        lua_getglobal(L, f);
        bool success = lua_type(L, -1) == LUA_TFUNCTION;
        lua_pop(L, 1);
        return success;
    }

    void State::registerFunction(const char* name, lua_CFunction f) {
        lua_register(L, name, f);
    }
}