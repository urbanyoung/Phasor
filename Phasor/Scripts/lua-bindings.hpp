#pragma once

#include <stdexcept>
#include <string>
#include <boost/format.hpp>
#include "../../lua/lua.hpp"
#include "../Common/tuple-iterate.hpp"

namespace lua {
    class Exception : public std::exception {
        const std::string msg;
    public:
        explicit Exception(const std::string& what)
            : msg("lua_exception: " + what)
        {}

        virtual const char* what() const override {
            return msg.c_str();
        }
    };

    namespace types {
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, const char*>::type
            ctype_name()
        {
                return "number";
            }

        template <> const char* ctype_name<const bool>();

        template <typename T>
        typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, const char*>::type
            ctype_name()
        {
                return "string";
            }

        //
        // ------------------------------------------------------------------------------
        //
        struct Nil {};

        // Reference to any Lua type, stored within the Lua VM
        struct AnyRef {
            lua_State* L;
            int ref;

            AnyRef();
            ~AnyRef();

            // Non-copyable
            AnyRef(const AnyRef&) = delete;
            AnyRef& operator=(const AnyRef&) = delete;

            AnyRef(AnyRef&& other);
            AnyRef& operator=(AnyRef&& other);

            // lua_State* is no longer valid so just reset
            inline void invalidate() { L = nullptr; }

            void push(lua_State* L) const;
            void pop(lua_State* L);
        };
    }

    struct Push {
        lua_State* L;

        Push(lua_State* L);

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            operator()(const T& x)
        {
                lua_pushnumber(L, x);
            }

        void operator()(const types::AnyRef& x);
        void operator()(const types::Nil&);
        void operator()(const char* x);
        void operator()(const std::string& x);
    };

    template <> void Push::operator()<bool>(const bool& x);

    struct Pop {
        enum class e_mode {
            kRaiseError,
            kDontRaiseError
        };

        int n;
        e_mode mode;
        bool err;
        lua_State* L;

    protected:

        void pop(lua_State* L);

        template <typename Y>
        void raise_error(int got) {
            if (mode == e_mode::kRaiseError) {
                auto f = boost::format("expected %s, got %s") % types::ctype_name<Y>() % lua_typename(L, got);
                luaL_argerror(L, n, f.str().c_str());
            } else {
                err = true;
            }
        }

    public:

        Pop(lua_State* L, int n, e_mode mode);

        template <typename Y>
        void pop_number_or_bool(Y& x) {
            const char* str;
            char* end;
            int ltype = lua_type(L, -1);
            switch (ltype) {
            case LUA_TNUMBER:
                x = static_cast<Y>(lua_tonumber(L, -1));
                break;
            case LUA_TBOOLEAN:
                x = static_cast<Y>(lua_toboolean(L, -1));
                break;
            case LUA_TSTRING:
                str = lua_tostring(L, -1);
                x = static_cast<Y>(strtol(str, &end, 10));
                if (*end != '\0')
                    raise_error<Y>(ltype);
                break;
            default:
                raise_error<Y>(ltype);
                break;
            }
            pop(L);
        }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type
            operator()(T& x)
        {
                pop_number_or_bool(x);
            }

        void operator()(std::string& x);
        void operator()(types::AnyRef& r);
    };

    template <> void Pop::operator()<bool>(bool& x);

    struct CFunc {
        lua_CFunction func;
        const char* name;
    };

    // Error handler that appends a stack trace to the error message
    int build_stack_trace_error_func(lua_State*);

    struct State  {
        lua_State* L;

        State();
        ~State();

        inline operator lua_State*() { return L; }

        void doFile(const std::string& file);
        void doString(const std::string& str);
        void pcall(int nargs, int nresults);
        void pcall(int nargs, int nresults, int err_function);
        bool hasFunction(const char* f);
        void registerFunction(const char* name, lua_CFunction f);

        template <class PushType, typename T>
        void setGlobal(const std::string& name, const T& value) {
            PushType p(L);
            p(value);
            lua_setglobal(L, name.c_str());
        }

        template <class PopType, typename T>
        boost::optional<T> getGlobal(const std::string& name) {
            lua_getglobal(L, name.c_str());
            boost::optional<T> result;
            PopType p(L, 0, PopType::e_mode::kDontRaiseError);
            p(result);
            return result;
        }
    };

    template <typename... ResultTypes>
    class Caller {
    private:
        static const size_t nresults = sizeof...(ResultTypes);
        State& L;
        bool err;
        int err_handler_index;

    public:

        Caller(State& L, lua_CFunction errHandler)
            : L(L), err(false), err_handler_index(0)
        {
            if (errHandler != nullptr) {
                err_handler_index = lua_gettop(L);
                lua_pushcfunction(L, errHandler);
               // err_handler_index = lua_gettop(L);
            }
        }

        Caller(State& L) 
            : Caller(L, build_stack_trace_error_func)
        {
            if (err_handler_index != 0)
                lua_pop(L, 1); // err handler
        }

        // whether or not an error occurred popping return values
        inline bool hasError() const {
            return err;
        }

        template <class PushType, class PopType, typename... ArgTypes>
        std::tuple<ResultTypes...> call(const char* func, const std::tuple<ArgTypes...>& args) {
            lua_getglobal(L, func);
            if (lua_type(L, -1) != LUA_TFUNCTION) {
                lua_pop(L, 1);
                throw Exception("attempt to call non-function entity");
            }

            size_t nargs = sizeof...(ArgTypes);

            PushType push(L);
            TupleHelpers::citerate<TupleHelpers::forward_comparator, PushType>(args, push);

            L.pcall(nargs, nresults, err_handler_index);

            std::tuple<ResultTypes...> results;
            PopType pop(L, nresults, Pop::e_mode::kDontRaiseError);
            TupleHelpers::iterate<TupleHelpers::reverse_comparator, PopType>(results, pop);
            err = pop.err;
            return results;
        }
    };

    namespace callback {
            

        // Counts number of boost::optional<> parameters (starting from end).
        // -------------------------------------------------------------------------------------
        //
        template <typename Y>
        size_t do_count(boost::optional<Y>) { return 1; }
        template <typename T>
        size_t do_count(T&) { return 0; }
        template <size_t N>
        struct more_than_zero {
            static const bool value = N > 0;
        };

        template <std::size_t N, typename... T>
        size_t count_optional(const std::tuple<T...>&,
                              typename std::enable_if<!more_than_zero<N>::value>::type* = 0) {
            return 0;
        }

        template <std::size_t N, typename... T>
        size_t count_optional(const std::tuple<T...>& x,
                              typename std::enable_if<more_than_zero<N>::value>::type* = 0) {
            auto c = do_count(std::get<N-1>(x));
            if (c == 0) return 0;
            return 1 + count_optional<N - 1, T...>(x);
        }

        template <typename... T>
        size_t count_optional(const std::tuple<T...>& x) {
            return count_optional<sizeof...(T), T...>(x);
        }
        //
        // -------------------------------------------------------------------------------------

        template <class PopType, typename... ResultTypes>
        std::tuple<ResultTypes...> getArguments(lua_State* L, const char* funcname) {
            std::tuple<ResultTypes...> args;
            static const size_t noptional = count_optional(args);
            static const size_t nrequired = sizeof...(ResultTypes)-noptional;

            size_t nargs = lua_gettop(L);
            if (nargs < nrequired) {
                luaL_error(L, "'%s' expects at least %d argument(s) (got %d)", funcname, nrequired, nargs);
            } else if (nargs > nrequired + noptional) {
                luaL_error(L, "'%s' expects at most %d argument(s) (got %d)", funcname, noptional + nrequired, nargs);
            } else {
                size_t diff = noptional - (nargs - nrequired);
                for (size_t x = 0; x < diff; x++)
                    lua_pushnil(L);
            }
            nargs = nrequired + noptional;

            PopType p(L, nargs, Pop::e_mode::kRaiseError);
            TupleHelpers::iterate<TupleHelpers::reverse_comparator, PopType>(args, p);
            return args;
        }

        template <class PushType, typename... Types>
        int pushReturns(lua_State* L, const std::tuple<Types...>& t) {
            int top = lua_gettop(L);
            PushType p(L);
            TupleHelpers::citerate<TupleHelpers::forward_comparator, PushType>(t, p);
            return lua_gettop(L) - top;
        }
    }
}