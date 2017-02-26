#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kthread.h>
#include <sys/lua.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <lua.h>
#include <lauxlib.h>

dev_type_open(luadevsw_open);
dev_type_close(luadevsw_close);
dev_type_write(luadevsw_write);
dev_type_read(luadevsw_read);
dev_type_ioctl(luadevsw_ioctl);

int
luadevsw_open(dev_t self, int flag, int mod, struct lwp *l)
{
    return 0;
}

int
luadevsw_close(dev_t self, int flag, int mod, struct lwp *l)
{
    return 0;
}

int
luadevsw_read(dev_t self, struct uio *uio, int flags)
{
    return 0;
}

int
luadevsw_write(dev_t self, struct uio *uio, int flags)
{
    return 0;
}

int
luadevsw_ioctl(dev_t self, u_long cmd, void *data, int flags, struct lwp*l)
{
    return 0;
}

static struct cdevsw lua_cdevsw;
static struct bdevsw lua_bdevsw;

static int
luadevsw_attach(lua_State *L)
{
    const struct cdevsw *cdevsw;
    const struct bdevsw *bdevsw;
    devmajor_t bmajor, cmajor;
    const char *devname;
    
    devname = lua_tostring(L, 1);
    
    if (!lua_istable(L, 2)) bdevsw = NULL;
    
    if (lua_isnumber(L, 3)) bmajor = lua_tointeger(L, 3);
    else bmajor = -1;
    
    if (lua_istable(L, 4)) {
	#define CHECK_METHOD(method) \
	    lua_getfield(L, 4, #method); \
	    if (lua_isfunction(L, -1)) \
		lua_cdevsw.d_##method = luadevsw_##method; \
            else \
	        lua_cdevsw.d_##method = no##method;
	      
	CHECK_METHOD(open)
	CHECK_METHOD(close)
	CHECK_METHOD(read)
	CHECK_METHOD(write)
	CHECK_METHOD(ioctl)
    }
    else cdevsw = NULL; 
    
    if (lua_isnumber(L, 5)) cmajor = lua_tointeger(L, 5);
    else cmajor = -1;
    
    devsw_attach(devname, bdevsw, &bmajor, cdevsw, &cmajor);
    /*
    {
	.d_open = noopen,
	.d_close = noclose,
	.d_read = noread,
	.d_write = nowrite,
	.d_ioctl = noioctl,
	.d_stop = nostop,
	.d_tty = notty,
	.d_poll = nopoll,
	.d_mmap = nommap,
	.d_kqfilter = nokqfilter,
	.d_discard = nodiscard,
	.d_flag = D_OTHER
    };*/
    lua_pushinteger(L, bmajor);
    lua_pushinteger(L, cmajor);
    return 2;
}

static int
luadevsw_dettach(lua_State *L)
{
    return 0;
}

static int
luaopen_luadevsw(lua_State *L)
{
    const luaL_Reg kthread_lib[] = {
        {"attach", luadevsw_attach},
        {"dettach", luadevsw_dettach},
	{NULL, NULL}
    };
    luaL_newlib(L, kthread_lib);
    return 1;
}

MODULE(MODULE_CLASS_MISC, luadevsw, "lua");

static int
luadevsw_modcmd(modcmd_t cmd, void *data)
{
    int error;

    switch (cmd) {
	case MODULE_CMD_INIT:
	    error = klua_mod_register("luadevsw", luaopen_luadevsw);
	    break;
	case MODULE_CMD_FINI:
	    error = klua_mod_unregister("luadevsw");
	    break;
	default:
	    error = ENOTTY;
	    break;
    }
    return error;
}