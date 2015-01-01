#pragma once

#include "lua-bindings.hpp"
#include "../Phasor/Halo/Halo.h"
#include "../Phasor/Halo/Player.h"
#include "../Phasor/Halo/Game/Objects.h"
#include "../Phasor/Halo/Game/Game.h"
#include "../Phasor/Halo/tags.h"
#include "../Common/vect3d.h"
#include "../Common/MyString.h"
#include "../Common/is_container.hpp"
#include <map>

namespace phlua {
    struct ProcessedString {
        std::vector<std::wstring> msgs;
    };

    struct PhasorPush : public lua::Push {
        PhasorPush(lua_State* L)
        : Push(L)
        {}

        using lua::Push::operator();

        void operator()(const halo::ident& x) {
            operator()((unsigned long)x);
        }

        void operator()(const vect3d& v) {
            operator()(v.x);
            operator()(v.y);
            operator()(v.z);
        }

        void operator()(const wchar_t* x) {
            operator()(NarrowString(x));
        }

        void operator()(const std::wstring& x) {
            operator()(NarrowString(x));
        }

        void operator()(const halo::s_tag_entry* x) {
            operator()((unsigned long)x);
        }

        void operator()(const halo::objects::s_halo_object* x) {
            operator()((unsigned long)x);
        }

        void operator()(const halo::s_player* player) {
            if (player) operator()(player->memory_id);
            else operator()(lua::types::Nil());
        }

        void operator()(const halo::s_player& player) {
            operator()(player.memory_id);
        }

        template <typename Container>
        typename std::enable_if<is_container<Container>::value>::type
            operator()(const Container& x) {
            int keyIndex = 1;

            lua_newtable(L);
            for (auto itr = x.cbegin(); itr != x.cend(); ++itr) {
                push_to_table(keyIndex++, *itr);
            }
        }

        template <typename K, template<typename, typename = std::allocator<V>> class C>
        typename std::enable_if<is_container<C>::value>::type
            operator()(const std::map<K, C>& x)
        {
            lua_newtable(L);
            for (auto itr = x.cbegin(); itr != x.cend(); ++itr) {
                push_to_table(itr->first, itr->second);
            }
        }

        template <typename T>
        void operator()(const boost::optional<T>& x) {
            if (!x) operator()(lua::types::Nil());
            else operator()(*x);
        }

    private:

        template <typename Key, typename Value>
        void push_to_table(const Key& key, const Value& value) {
            operator()(key);
            operator()(value);
            lua_settable(L, -3);
        }
    };

    struct PhasorPop : public lua::Pop {
    private:

        void raise_error(int n, const char* msg) {
            if (mode == e_mode::kRaiseError) {
                luaL_argerror(L, n, msg);
            } else {
                err = true;
            }
        }

    public:

        /* msvc needs inheriting constructors! */
        PhasorPop(lua_State* L, int n, e_mode mode)
            : Pop(L, n, mode)
        {}

        using lua::Pop::operator();

        void operator()(std::wstring& x) {
            std::string s;
            operator()(s);
            x = WidenString(s);
        }

        void operator()(halo::ident& x) {
            unsigned long id;
            operator()(id);
            x = halo::make_ident(id);
        }

        void operator()(halo::s_tag_entry*& tag) {
            halo::ident id;
            operator()(id);
            halo::s_tag_entry* t = halo::LookupTag(id);
            if (!t) raise_error(n-1, "invalid tag id");
            else tag = t;
        }

        /*void operator()(vect3d& x) {
            vect3d tmp;
            operator()(tmp.x);
            operator()(tmp.y);
            operator()(tmp.z);
            x = tmp;
            }*/

        template <typename T>
        void operator()(std::vector<T>& x)
        {
            if (lua_type(L, -1) != LUA_TTABLE) {
                // try treating it as a container with one item
                T value;
                operator()(value);
                x.push_back(std::move(value));
                return;
            }

            lua_pushnil(L);
            while (lua_next(L, -2) != 0) { // table is always at -2
                // table -3, key -2, value -1
                int key;
                T value;
                operator()(value);

                // keys should always be integers because we're treating the
                // table as an array. we do this check here for better error
                // messages
                if (lua_type(L, -1) != LUA_TNUMBER)
                    raise_error(n-1, "expected array, got table with key-value pairs");

                // we need the key to remain on the stack, so push it again
                lua_pushvalue(L, -1);
                operator()(key);

                x.push_back(std::move(value));

                // -1 is key, -2 is table
            }
            pop(L);
        }

        template <typename T>
        void operator()(std::map<std::string, T>& x)
        {
            if (lua_type(L, -1) != LUA_TTABLE)
                raise_error(n-1, "key-value mapping table expected");

            lua_pushnil(L);
            while (lua_next(L, -2) != 0) { // table is always at -2
                // table -3, key -2, value -1
                std::string key;
                T value;
                operator()(value);

                // we need the key to remain on the stack, so push it again
                lua_pushvalue(L, -1);
                operator()(key);

                x.insert(std::make_pair(key, value));
                
                // -1 is key, -2 is table
            }
            pop(L);
        }

        void operator()(ProcessedString& x) {
            std::wstring str;
            operator()(str);
            x.msgs = Tokenize<std::wstring>(str, L"\n");
            // Format each message and replace {i} with i's name.
            for (auto itr = x.msgs.begin(); itr != x.msgs.end(); ++itr) {
                size_t brace_pos = itr->find(L'{');
                size_t end_brace_pos = itr->find(L'}', brace_pos);

                while (brace_pos != itr->npos && end_brace_pos != itr->npos) {
                    size_t diff = end_brace_pos - brace_pos;
                    if (diff == 2 || diff == 3) { // ids can only be at most 2 digits
                        std::string str = NarrowString(itr->substr(brace_pos + 1, diff - 1));
                        int id;
                        if (StringToNumber<int>(str, id)) {
                            halo::s_player* player = halo::game::getPlayer(id);
                            if (player) {
                                itr->erase(brace_pos, diff + 1);
                                itr->insert(brace_pos, player->mem->playerName);
                            }
                        }
                    }
                    brace_pos = itr->find(L'{', brace_pos + 1);
                    end_brace_pos = itr->find(L'}', brace_pos);
                }
            }
        }

        // --------------------------------------------------------------

        void operator()(halo::s_player*& player) {
            int id;
            operator()(id);
            halo::s_player* p = halo::game::getPlayer(id);
            if (!p) raise_error(n-1, "invalid player id");
            else player = p;
        }

        void operator()(boost::optional<halo::s_player*>& player) {
            boost::optional<int> id;
            operator()(id);
            if (id) {
                halo::s_player* p = halo::game::getPlayer(*id);
                if (p) player = p;
            }
        }

        // --------------------------------------------------------------

        void operator()(halo::objects::s_halo_object*& object) {
            halo::ident id;
            operator()(id);
            halo::objects::s_halo_object* obj = static_cast<halo::objects::s_halo_object*>(halo::objects::GetObjectAddress(id));
            if (!obj) raise_error(n-1, "invalid object id");
            else object = obj;
        }

        void operator()(boost::optional<halo::objects::s_halo_object*>& object) {
            boost::optional<halo::ident> id;
            operator()(id);
            if (id) {
                halo::objects::s_halo_object* obj = static_cast<halo::objects::s_halo_object*>(halo::objects::GetObjectAddress(*id));
                if (obj) object = obj;
            }
        }

        // --------------------------------------------------------------

        template <typename Y>
        void operator()(boost::optional<Y>& x) {
            if (lua_type(L, -1) == LUA_TNIL) {
                pop(L);
            } else {
                Y val;
                operator()(val);
                x = val;
            }
        }
    };

    /* Just for convenience (don't need to specify PhasorPush/Pop */
    template <typename... ResultTypes>
    class Caller : public ::lua::Caller<ResultTypes...> {
    public:
        Caller(::lua::State& L)
            : ::lua::Caller<ResultTypes...>(L)
        {}

        template <typename... ArgTypes>
        inline std::tuple<ResultTypes...> call(const char* func,
                                               const std::tuple<ArgTypes...>& args)
        {
            return ::lua::Caller<ResultTypes...>::call<PhasorPush, PhasorPop, ArgTypes...>(func, args);
        }
    };

    namespace callback {
        template <typename... ResultTypes>
        inline std::tuple<ResultTypes...> getArguments(lua_State* L, const char* funcname) {
            return ::lua::callback::getArguments<PhasorPop, ResultTypes...>(L, funcname);
        }

        template <typename... Types>
        inline int pushReturns(lua_State* L, const std::tuple<Types...>& t) {
            return ::lua::callback::pushReturns<PhasorPush, Types...>(L, t);
        }
    }
}