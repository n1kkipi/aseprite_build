// Aseprite
// Copyright (C) 2019-2022  Igara Studio S.A.
// Copyright (C) 2018  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/commands/command.h"
#include "app/commands/commands.h"
#include "app/commands/new_params.h"
#include "app/commands/params.h"
#include "app/context.h"
#include "app/script/luacpp.h"

namespace app {
namespace script {

namespace {

struct AppCommand { };

int Command_call(lua_State* L)
{
  app::Context* ctx = App::instance()->context();
  if (!ctx)
    return 0;

  auto command = get_ptr<Command>(L, 1);
  Params params;

  if (auto commandLua = dynamic_cast<CommandWithNewParamsBase*>(command)) {
    commandLua->loadParamsFromLuaTable(L, 2);
  }
  else {
    if (lua_istable(L, 2)) {
      lua_pushnil(L);
      while (lua_next(L, 2) != 0) {
        if (const char* k = lua_tostring(L, -2)) {
          const char* v = luaL_tolstring(L, -1, nullptr);
          if (v)
            params.set(k, v);
          lua_pop(L, 1);        // pop the value generated by luaL_tolstring()
        }
        lua_pop(L, 1);          // pop the value lua_next(), leave the key in the stack
      }
    }
  }

  ctx->executeCommand(command, params);

  if (ctx->commandResult().type() == CommandResult::kOk) {
    lua_pushboolean(L, true);
  }
  else {
    // TODO rollback/cancel the whole current transaction?
    //      or just throw an luaL_error()?
    lua_pushboolean(L, false);
  }
  return 1;
}

int AppCommand_index(lua_State* L)
{
  const char* id = lua_tostring(L, 2);
  if (!id)
    return luaL_error(L, "id in app.command.id() must be a string");

  Command* cmd = Commands::instance()->byId(id);
  if (!cmd)
    return luaL_error(L, "command '%s' not found", id);

  push_ptr(L, cmd);
  return 1;
}

const luaL_Reg Command_methods[] = {
  { "__call", Command_call },
  { nullptr, nullptr }
};

const luaL_Reg AppCommand_methods[] = {
  { "__index", AppCommand_index },
  { nullptr, nullptr }
};

} // anonymous namespace

DEF_MTNAME(Command);
DEF_MTNAME(AppCommand);

void register_app_command_object(lua_State* L)
{
  REG_CLASS(L, Command);
  REG_CLASS(L, AppCommand);

  lua_getglobal(L, "app");
  lua_pushstring(L, "command");
  push_new<AppCommand>(L);
  lua_rawset(L, -3);
  lua_pop(L, 1);
}

} // namespace script
} // namespace app
