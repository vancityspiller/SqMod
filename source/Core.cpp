#include "Core.hpp"
#include "Logger.hpp"
#include "Entity.hpp"
#include "Register.hpp"

// ------------------------------------------------------------------------------------------------
#include "Base/Color3.hpp"
#include "Base/Vector2i.hpp"
#include "Misc/Automobile.hpp"
#include "Misc/Model.hpp"

// ------------------------------------------------------------------------------------------------
#include <SimpleIni.h>
#include <format.h>

// ------------------------------------------------------------------------------------------------
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>

// ------------------------------------------------------------------------------------------------
#include <cstdarg>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <exception>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
const Core::Pointer _Core = Core::Inst();

// ------------------------------------------------------------------------------------------------
static std::shared_ptr<CSimpleIni> g_Config;

// ------------------------------------------------------------------------------------------------
Core::Core() noexcept
    : m_State(SQMOD_SUCCESS)
    , m_Options()
    , m_VM(nullptr)
    , m_RootTable(nullptr)
    , m_Scripts()
    , m_ErrorMsg()
    , m_PlayerTrack()
    , m_VehicleTrack()
{
    // Create a few shared buffers
    MakeBuffer(8);
}

// ------------------------------------------------------------------------------------------------
Core::~Core()
{
    this->Terminate();
}

// ------------------------------------------------------------------------------------------------
void Core::_Finalizer(Core * ptr) noexcept
{
    if (ptr) delete ptr;
}

// ------------------------------------------------------------------------------------------------
Core::Pointer Core::Inst() noexcept
{
    if (!_Core)
    {
        return Pointer(new Core(), &Core::_Finalizer);
    }

    return Pointer(nullptr, &Core::_Finalizer);
}

// ------------------------------------------------------------------------------------------------
bool Core::Init() noexcept
{
    LogMsg("%s", CenterStr("INITIALIZING", '*'));

    if (!this->Configure() || !this->CreateVM() || !this->LoadScripts())
    {
        return false;
    }

    LogMsg("%s", CenterStr("SUCCESS", '*'));

    return true;
}

bool Core::Load() noexcept
{
    LogMsg("%s", CenterStr("LOADING", '*'));

    if (!this->Execute())
    {
        return false;
    }

    LogMsg("%s", CenterStr("SUCCESS", '*'));

    return true;
}

// ------------------------------------------------------------------------------------------------
void Core::Deinit() noexcept
{
    this->DestroyVM();
}

void Core::Unload() noexcept
{

}

// ------------------------------------------------------------------------------------------------
void Core::Terminate() noexcept
{
    this->Deinit();
    this->Unload();
}

// ------------------------------------------------------------------------------------------------
void Core::SetState(SQInteger val) noexcept
{
    m_State = val;
}

SQInteger Core::GetState() const noexcept
{
    return m_State;
}

// ------------------------------------------------------------------------------------------------
string Core::GetOption(const string & name) const noexcept
{
    OptionPool::const_iterator elem = m_Options.find(name);
    return (elem == m_Options.cend()) ? string() : elem->second;
}

void Core::SetOption(const string & name, const string & value) noexcept
{
    m_Options[name] = value;
}

// ------------------------------------------------------------------------------------------------
Core::Buffer Core::PullBuffer(unsigned sz) noexcept
{
    Buffer buf;
    if (m_BufferPool.empty())
    {
        buf.resize(sz);
    }
    else
    {
        buf = std::move(m_BufferPool.back());
        m_BufferPool.pop();
    }
    return std::move(buf);
}

// ------------------------------------------------------------------------------------------------
void Core::PushBuffer(Buffer && buf) noexcept
{
    m_BufferPool.push(std::move(buf));
}

// ------------------------------------------------------------------------------------------------
void Core::MakeBuffer(unsigned num, unsigned sz) noexcept
{
    while (num--)
    {
        m_BufferPool.emplace(sz);
    }
}

// ------------------------------------------------------------------------------------------------
void Core::ConnectPlayer(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CPlayer >::Activate(id, false))
    {
        // Trigger the specific event
        OnPlayerCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CPlayer> instance");
    }
}

void Core::DisconnectPlayer(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    // Check to be sure we have this player instance active
    if (Reference< CPlayer >::Verify(id))
    {
        // Trigger the specific event
        OnPlayerDestroyed(id, header, payload);
    }
}

// ------------------------------------------------------------------------------------------------
bool Core::Configure() noexcept
{
    LogDbg("Attempting to instantiate the configuration file");

    if (g_Config)
    {
        g_Config->Reset();
    }
    else
    {
        g_Config.reset(new CSimpleIniA(true, true, true));
    }

    if (!g_Config)
    {
        LogFtl("Unable to instantiate the configuration class");

        return false;
    }

    LogDbg("Attempting to load the configuration file.");

    SI_Error ini_ret = g_Config->LoadFile(_SC("./sqmod.ini"));

    if (ini_ret < 0)
    {
        switch (ini_ret)
        {
            case SI_FAIL:   LogErr("Failed to load the configuration file. Probably invalid"); break;
            case SI_NOMEM:  LogErr("Run out of memory while loading the configuration file"); break;
            case SI_FILE:   LogErr("Failed to load the configuration file. %s", strerror(errno)); break;
            default:        LogErr("Failed to load the configuration file for some unforeseen reason");
        }
        return false;
    }

    LogDbg("Applying the specified logging filters");

    if (!SToB(g_Config->GetValue("ConsoleLog", "Debug", "true")))   _Log->DisableConsoleLevel(Logger::LEVEL_DBG);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Message", "true"))) _Log->DisableConsoleLevel(Logger::LEVEL_MSG);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Success", "true"))) _Log->DisableConsoleLevel(Logger::LEVEL_SCS);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Info", "true")))    _Log->DisableConsoleLevel(Logger::LEVEL_INF);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Warning", "true"))) _Log->DisableConsoleLevel(Logger::LEVEL_WRN);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Error", "true")))   _Log->DisableConsoleLevel(Logger::LEVEL_ERR);
    if (!SToB(g_Config->GetValue("ConsoleLog", "Fatal", "true")))   _Log->DisableConsoleLevel(Logger::LEVEL_FTL);

    if (!SToB(g_Config->GetValue("FileLog", "Debug", "true")))      _Log->DisableFileLevel(Logger::LEVEL_DBG);
    if (!SToB(g_Config->GetValue("FileLog", "Message", "true")))    _Log->DisableFileLevel(Logger::LEVEL_MSG);
    if (!SToB(g_Config->GetValue("FileLog", "Success", "true")))    _Log->DisableFileLevel(Logger::LEVEL_SCS);
    if (!SToB(g_Config->GetValue("FileLog", "Info", "true")))       _Log->DisableFileLevel(Logger::LEVEL_INF);
    if (!SToB(g_Config->GetValue("FileLog", "Warning", "true")))    _Log->DisableFileLevel(Logger::LEVEL_WRN);
    if (!SToB(g_Config->GetValue("FileLog", "Error", "true")))      _Log->DisableFileLevel(Logger::LEVEL_ERR);
    if (!SToB(g_Config->GetValue("FileLog", "Fatal", "true")))      _Log->DisableFileLevel(Logger::LEVEL_FTL);

    LogDbg("Reading the options from the general section");

    CSimpleIniA::TNamesDepend general_options;

    if (g_Config->GetAllKeys("Options", general_options) || general_options.size() > 0)
    {
        for (const auto & cfg_option : general_options)
        {
            CSimpleIniA::TNamesDepend option_list;

            if (!g_Config->GetAllValues("Options", cfg_option.pItem, option_list))
            {
                continue;
            }

            option_list.sort(CSimpleIniA::Entry::LoadOrder());

            for (const auto & cfg_value : option_list)
            {
                m_Options[cfg_option.pItem] = cfg_value.pItem;
            }
        }
    }
    else
    {
        LogInf("No options specified in the configuration file");
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::CreateVM() noexcept
{
    LogDbg("Acquiring the virtual machine stack size");

    // Start with an unknown stack size
    SQInteger stack_size = SQMOD_UNKNOWN;

    // Attempt to get it from the configuration file
    try
    {
        stack_size = SToI< SQInteger >::Fn(GetOption(_SC("VMStackSize")), 0, 10);
    }
    catch (const std::invalid_argument & e)
    {
        LogWrn("Unable to extract option value: %s", e.what());
    }

    // Validate the stack size
    if (stack_size <= 0)
    {
        LogWrn("Invalid stack size. Reverting to default size: %d", SQMOD_STACK_SIZE);
        SetOption(_SC("VMStackSize"), std::to_string(SQMOD_STACK_SIZE));
        stack_size = SQMOD_STACK_SIZE;
    }

    LogInf("Creating a virtual machine with a stack size of: %d", stack_size);

    m_VM = sq_open(stack_size);

    if (!m_VM)
    {
        LogFtl("Unable to open a virtual machine with a stack size: %d", stack_size);

        return false;
    }
    else
    {
        DefaultVM::Set(m_VM);
        ErrorHandling::Enable(true);
        m_RootTable.reset(new RootTable(m_VM));
        m_Scripts.clear();
    }

    LogDbg("Registering the standard libraries");
    // Register the standard libraries
    sq_pushroottable(m_VM);
    sqstd_register_iolib(m_VM);
    sqstd_register_bloblib(m_VM);
    sqstd_register_mathlib(m_VM);
    sqstd_register_systemlib(m_VM);
    sqstd_register_stringlib(m_VM);
    sq_pop(m_VM, 1);

    LogDbg("Setting the base output function");
    // Set the function that handles the text output
    sq_setprintfunc(m_VM, PrintFunc, ErrorFunc);

    LogDbg("Setting the base error handlers");
    // Se the error handlers
    sq_setcompilererrorhandler(m_VM, CompilerErrorHandler);
    sq_newclosure(m_VM, RuntimeErrorHandler, 0);
    sq_seterrorhandler(m_VM);

    LogDbg("Registering the plugin API");
    // Register the plugin API
    if (!RegisterAPI(m_VM))
    {
        LogFtl("Unable to register the plugin API");
        return false;
    }
    // At this point the VM is ready
    return true;
}

void Core::DestroyVM() noexcept
{
    // See if the Virtual Machine wasn't already destroyed
    if (m_VM != nullptr)
    {
        // Let instances know that they should release links to this VM
        VMClose.Emit();
        // Release the references to the script objects
        m_Scripts.clear();
        // Release the reference to the root table
        m_RootTable.reset();
        // Assertions during close may cause double delete
        HSQUIRRELVM sq_vm = m_VM;
        // Explicitly set the virtual machine to null
        m_VM = nullptr;
        // Close the Virtual Machine
        sq_close(sq_vm);
    }
}

// ------------------------------------------------------------------------------------------------
bool Core::LoadScripts() noexcept
{
    LogDbg("Attempting to compile the specified scripts");

    if (!g_Config)
    {
        LogWrn("Cannot compile any scripts without the configurations");

        return false;
    }

    CSimpleIniA::TNamesDepend script_list;
    g_Config->GetAllValues("Scripts", "Source", script_list);

    if (script_list.size() <= 0)
    {
        LogWrn("No scripts specified in the configuration file");

        return false;
    }

    script_list.sort(CSimpleIniA::Entry::LoadOrder());

    for (auto const & cfg_script : script_list)
    {
        string path(cfg_script.pItem);

        if (m_Scripts.find(path) != m_Scripts.cend())
        {
            LogWrn("Script was already loaded: %s", path.c_str());

            continue;
        }
        else if (!Compile(path))
        {
            return false;
        }
        else
        {
            LogScs("Successfully compiled script: %s", path.c_str());
        }
    }

    if (m_Scripts.empty())
    {
        LogErr("No scripts compiled. No reason to load the plugin");

        return false;
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::Compile(const string & name) noexcept
{
    if (name.empty())
    {
        LogErr("Cannot compile script without a valid name");

        return false;
    }

    std::pair< SqScriptPool::iterator, bool > res = m_Scripts.emplace(name, Script(m_VM));

    if (res.second)
    {
        res.first->second.CompileFile(name);

        if (Error::Occurred(m_VM))
        {
            LogErr("Unable to compile script: %s", name.c_str());
            LogInf("=> %s", Error::Message(m_VM).c_str());
            m_Scripts.erase(res.first);

            return false;
        }
    }
    else
    {
        LogErr("Unable to queue script: %s", name.c_str());

        return false;
    }

    return true;
}

bool Core::Execute() noexcept
{
    LogDbg("Attempting to execute the specified scripts");

    for (auto & elem : m_Scripts)
    {
        elem.second.Run();

        if (Error::Occurred(m_VM))
        {
            LogErr("Unable to execute script: %s", elem.first.c_str());
            LogInf("=> %s", Error::Message(m_VM).c_str());

            return false;
        }
        else
        {
            LogScs("Successfully executed script: %s", elem.first.c_str());
        }
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
void Core::PrintCallstack() noexcept
{
    SQStackInfos si;


    LogMsg("%s", CenterStr("CALLSTACK", '*'));

    for (SQInteger level = 1; SQ_SUCCEEDED(sq_stackinfos(m_VM, level, &si)); ++level)
    {
        LogInf("FUNCTION %s()", si.funcname ? si.funcname : _SC("unknown"));
        LogInf("=> [%d] : {%s}", si.line, si.source ? si.source : _SC("unknown"));
    }

    const SQChar * s_ = 0, * name = 0;
    SQInteger i_, seq = 0;
    SQFloat f_;
    SQUserPointer p_;

    LogMsg("%s", CenterStr("LOCALS", '*'));

    for (SQInteger level = 0; level < 10; level++) {
        seq = 0;
        while((name = sq_getlocal(m_VM, level, seq))) {
            ++seq;
            switch(sq_gettype(m_VM, -1))
            {
                case OT_NULL:
                    LogInf("NULL [%s] : ...", name);
                    break;
                case OT_INTEGER:
                    sq_getinteger(m_VM, -1, &i_);
                    LogInf("INTEGER [%s] : {%d}", name, i_);
                    break;
                case OT_FLOAT:
                    sq_getfloat(m_VM, -1, &f_);
                    LogInf("FLOAT [%s] : {%f}", name, f_);
                    break;
                case OT_USERPOINTER:
                    sq_getuserpointer(m_VM, -1, &p_);
                    LogInf("USERPOINTER [%s] : {%p}\n", name, p_);
                    break;
                case OT_STRING:
                    sq_getstring(m_VM, -1, &s_);
                    LogInf("STRING [%s] : {%s}", name, s_);
                    break;
                case OT_TABLE:
                    LogInf("TABLE [%s] : ...", name);
                    break;
                case OT_ARRAY:
                    LogInf("ARRAY [%s] : ...", name);
                    break;
                case OT_CLOSURE:
                    LogInf("CLOSURE [%s] : ...", name);
                    break;
                case OT_NATIVECLOSURE:
                    LogInf("NATIVECLOSURE [%s] : ...", name);
                    break;
                case OT_GENERATOR:
                    LogInf("GENERATOR [%s] : ...", name);
                    break;
                case OT_USERDATA:
                    LogInf("USERDATA [%s] : ...", name);
                    break;
                case OT_THREAD:
                    LogInf("THREAD [%s] : ...", name);
                    break;
                case OT_CLASS:
                    LogInf("CLASS [%s] : ...", name);
                    break;
                case OT_INSTANCE:
                    LogInf("INSTANCE [%s] : ...", name);
                    break;
                case OT_WEAKREF:
                    LogInf("WEAKREF [%s] : ...", name);
                    break;
                case OT_BOOL:
                    sq_getinteger(m_VM, -1, &i_);
                    LogInf("BOOL [%s] : {%s}", name, i_ ? _SC("true") : _SC("false"));
                    break;
                default:
                    LogErr("UNKNOWN [%s] : ...", name);
                break;
            }
            sq_pop(m_VM, 1);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void Core::PrintFunc(HSQUIRRELVM vm, const SQChar * str, ...) noexcept
{
    // Prepare the arguments list
    va_list args;
    va_start(args, str);
    // Acquire a buffer from the buffer pool
    Core::Buffer vbuf = _Core->PullBuffer();
    // Attempt to process the specified format string
    SQInt32 fmt_ret = std::vsnprintf(vbuf.data(), vbuf.size(), str, args);
    // See if the formatting was successful
    if (fmt_ret < 0)
    {
        // Return the buffer back to the buffer pool
        _Core->PushBuffer(std::move(vbuf));
        LogErr("Format error");
        return;
    }
    // See if the buffer was big enough
    else if (_SCSZT(fmt_ret) > vbuf.size())
    {
        // Resize the buffer to accomodate the required size
        vbuf.resize(++fmt_ret);
        // Attempt to process the specified format string again
        fmt_ret = std::vsnprintf(vbuf.data(), vbuf.size(), str, args);
        // See if the formatting was successful
        if (fmt_ret < 0)
        {
            // Return the buffer back to the buffer pool
            _Core->PushBuffer(std::move(vbuf));
            LogErr("Format error");
            return;
        }
    }
    // Release the arguments list
    va_end(args);
    // Output the buffer content
    LogMsg("%s", vbuf.data());
    // Return the buffer back to the buffer pool
    _Core->PushBuffer(std::move(vbuf));
}

void Core::ErrorFunc(HSQUIRRELVM vm, const SQChar * str, ...) noexcept
{
    // Prepare the arguments list
    va_list args;
    va_start(args, str);
    // Acquire a buffer from the buffer pool
    Core::Buffer vbuf = _Core->PullBuffer();
    // Attempt to process the specified format string
    SQInt32 fmt_ret = std::vsnprintf(vbuf.data(), vbuf.size(), str, args);
    // See if the formatting was successful
    if (fmt_ret < 0)
    {
        // Return the buffer back to the buffer pool
        _Core->PushBuffer(std::move(vbuf));
        LogErr("Format error");
        return;
    }
    // See if the buffer was big enough
    else if (_SCSZT(fmt_ret) > vbuf.size())
    {
        // Resize the buffer to accomodate the required size
        vbuf.resize(++fmt_ret);
        // Attempt to process the specified format string again
        fmt_ret = std::vsnprintf(vbuf.data(), vbuf.size(), str, args);
        // See if the formatting was successful
        if (fmt_ret < 0)
        {
            // Return the buffer back to the buffer pool
            _Core->PushBuffer(std::move(vbuf));
            LogErr("Format error");
            return;
        }
    }
    // Release the arguments list
    va_end(args);
    // Output the buffer content
    LogErr("%s", vbuf.data());
    // Return the buffer back to the buffer pool
    _Core->PushBuffer(std::move(vbuf));
}

// ------------------------------------------------------------------------------------------------
SQInteger Core::RuntimeErrorHandler(HSQUIRRELVM vm) noexcept
{
    if (sq_gettop(vm) < 1)
    {
        return 0;
    }

    const SQChar * err_msg = NULL;

    if (SQ_SUCCEEDED(sq_getstring(vm, 2, &err_msg)))
    {
        _Core->m_ErrorMsg.assign(err_msg);
    }
    else
    {
        _Core->m_ErrorMsg.assign(_SC("An unknown runtime error has occurred"));
    }

    LogMsg("%s", CenterStr("ERROR", '*'));

    LogInf("[MESSAGE] : %s", _Core->m_ErrorMsg.c_str());

    if (_Log->GetVerbosity() > 0)
    {
        _Core->PrintCallstack();
    }

    LogMsg("%s", CenterStr("CONCLUDED", '*'));

    return SQ_OK;
}

void Core::CompilerErrorHandler(HSQUIRRELVM vm, const SQChar * desc, const SQChar * src, SQInteger line, SQInteger column) noexcept
{
    try
    {
        _Core->m_ErrorMsg.assign(fmt::format("{0:s} : {1:d}:{2:d} : {3:s}", src, line, column, desc).c_str());
    }
    catch (const std::exception & e)
    {
        LogErr("Compiler error: %s", e.what());
        _Core->m_ErrorMsg.assign(_SC("An unknown compiler error has occurred"));
    }

    LogErr("%s", _Core->m_ErrorMsg.c_str());
}

// ------------------------------------------------------------------------------------------------
Reference< CBlip > Core::CreateBlip(SQInt32 index, SQInt32 world, const Vector3 & pos, SQInt32 scale, \
                        const Color4 & color, SQInt32 sprite, SQInt32 header, SqObj & payload) noexcept
{
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateCoordBlip(index, world, pos.x, pos.y, pos.z, scale, color.GetRGBA(), sprite);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CBlip >::Activate(id, true, world, scale, sprite, pos, color))
    {
        // Trigger the specific event
        OnBlipCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CBlip> instance");
    }
    // Return the obtained reference as is
    return Reference< CBlip >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CCheckpoint > Core::CreateCheckpoint(const Reference< CPlayer > & player, SQInt32 world, const Vector3 & pos, \
                        const Color4 & color, SQFloat radius, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified player reference is valid
    if (!player)
    {
        LogWrn("Attempting to create a <Checkpoint> instance on an invalid player: %d", _SCI32(player));
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateCheckpoint(_SCI32(player), world, pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a, radius);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CCheckpoint >::Activate(id, true))
    {
        // Trigger the specific event
        OnCheckpointCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CCheckpoint> instance");
    }
    // Return the obtained reference as is
    return Reference< CCheckpoint >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CKeybind > Core::CreateKeybind(SQInt32 slot, bool release, SQInt32 primary, SQInt32 secondary, \
                        SQInt32 alternative, SQInt32 header, SqObj & payload) noexcept
{
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->RegisterKeyBind(slot, release, primary, secondary, alternative);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CKeybind >::Activate(id, true, primary, secondary, alternative, release))
    {
        // Trigger the specific event
        OnKeybindCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CKeybind> instance");
    }
    // Return the obtained reference as is
    return Reference< CKeybind >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CObject > Core::CreateObject(const CModel & model, SQInt32 world, const Vector3 & pos, SQInt32 alpha, \
                        SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified model is valid
    if (!model)
    {
        LogWrn("Attempting to create an <Object> instance with an invalid model: %d", _SCI32(model));
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateObject(_SCI32(model), world, pos.x, pos.y, pos.z, alpha);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CObject >::Activate(id, true))
    {
        // Trigger the specific event
        OnObjectCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CObject> instance");
    }
    // Return the obtained reference as is
    return Reference< CObject >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CPickup > Core::CreatePickup(const CModel & model, SQInt32 world, SQInt32 quantity, const Vector3 & pos, \
                        SQInt32 alpha, bool automatic, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified model is valid
    if (!model)
    {
        LogWrn("Attempting to create a <Pickup> instance with an invalid model: %d", _SCI32(model));
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreatePickup(_SCI32(model), world, quantity, pos.x, pos.y, pos.z, alpha, automatic);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CPickup >::Activate(id, true))
    {
        // Trigger the specific event
        OnPickupCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CPickup> instance");
    }
    // Return the obtained reference as is
    return Reference< CPickup >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CSphere > Core::CreateSphere(const Reference< CPlayer > & player, SQInt32 world, const Vector3 & pos, \
                        const Color3 & color, SQFloat radius, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified player reference is valid
    if (!player)
    {
        LogWrn("Attempting to create a <Sphere> instance on an invalid player: %d", _SCI32(player));
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateSphere(_SCI32(player), world, pos.x, pos.y, pos.z, color.r, color.g, color.b, radius);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CSphere >::Activate(id, true))
    {
        // Trigger the specific event
        OnSphereCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CSphere> instance");
    }
    // Return the obtained reference as is
    return Reference< CSphere >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CSprite > Core::CreateSprite(SQInt32 index, const String & file, const Vector2i & pos, const Vector2i & rot, \
                        SQFloat angle, SQInt32 alpha, bool rel, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified file path is valid
    if (file.empty())
    {
        LogWrn("Attempting to create a <Sprite> instance with an empty path: %d", file.size());
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateSprite(index, file.c_str(), pos.x, pos.y, rot.x, rot.y, angle, alpha, rel);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CSprite >::Activate(id, true, file))
    {
        // Trigger the specific event
        OnSpriteCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CSprite> instance");
    }
    // Return the obtained reference as is
    return Reference< CSprite >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CTextdraw > Core::CreateTextdraw(SQInt32 index, const String & text, const Vector2i & pos, \
                        const Color4 & color, bool rel, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified text is valid
    if (text.empty())
    {
        LogWrn("Attempting to create a <Textdraw> instance with an empty text: %d", text.size());
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateTextdraw(index, text.c_str(), pos.x, pos.y, color.GetRGBA(), rel);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CTextdraw >::Activate(id, true, text))
    {
        // Trigger the specific event
        OnTextdrawCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CTextdraw> instance");
    }
    // Return the obtained reference as is
    return Reference< CTextdraw >(id);
}

// ------------------------------------------------------------------------------------------------
Reference< CVehicle > Core::CreateVehicle(const CAutomobile & model, SQInt32 world, const Vector3 & pos, \
                        SQFloat angle, SQInt32 primary, SQInt32 secondary, SQInt32 header, SqObj & payload) noexcept
{
    // See if the specified model is valid
    if (!model)
    {
        LogWrn("Attempting to create an <Vehicle> instance with an invalid model: %d", _SCI32(model));
    }
    // Attempt to create an instance on the server and obtain it's identifier
    SQInt32 id = _Func->CreateVehicle(_SCI32(model), world, pos.x, pos.y, pos.z, angle, primary, secondary);
    // Attempt to activate the instance in the plugin at the received identifier
    if (EntMan< CVehicle >::Activate(id, true))
    {
        // Trigger the specific event
        OnVehicleCreated(id, header, payload);
    }
    else
    {
        LogErr("Unable to create a new <CVehicle> instance");
    }
    // Return the obtained reference as is
    return Reference< CVehicle >(id);
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyBlip(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{

    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyCheckpoint(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyKeybind(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyObject(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyPickup(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyPlayer(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroySphere(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroySprite(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyTextdraw(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
bool Core::DestroyVehicle(SQInt32 id, SQInt32 header, SqObj & payload) noexcept
{
    return true;
}

// ------------------------------------------------------------------------------------------------
void Core::OnBlipCreated(SQInt32 blip, SQInt32 header, SqObj & payload) noexcept
{
    BlipCreated.Emit(blip, header, payload);
    Reference< CBlip >::Get(blip).BlipCreated.Emit(blip, header, payload);
}

void Core::OnCheckpointCreated(SQInt32 checkpoint, SQInt32 header, SqObj & payload) noexcept
{
    CheckpointCreated.Emit(checkpoint, header, payload);
    Reference< CCheckpoint >::Get(checkpoint).CheckpointCreated.Emit(checkpoint, header, payload);
}

void Core::OnKeybindCreated(SQInt32 keybind, SQInt32 header, SqObj & payload) noexcept
{
    KeybindCreated.Emit(keybind, header, payload);
    Reference< CKeybind >::Get(keybind).KeybindCreated.Emit(keybind, header, payload);
}

void Core::OnObjectCreated(SQInt32 object, SQInt32 header, SqObj & payload) noexcept
{
    ObjectCreated.Emit(object, header, payload);
    Reference< CObject >::Get(object).ObjectCreated.Emit(object, header, payload);
}

void Core::OnPickupCreated(SQInt32 pickup, SQInt32 header, SqObj & payload) noexcept
{
    PickupCreated.Emit(pickup, header, payload);
    Reference< CPickup >::Get(pickup).PickupCreated.Emit(pickup, header, payload);
}

void Core::OnPlayerCreated(SQInt32 player, SQInt32 header, SqObj & payload) noexcept
{
    PlayerCreated.Emit(player, header, payload);
    Reference< CPlayer >::Get(player).PlayerCreated.Emit(player, header, payload);
}

void Core::OnSphereCreated(SQInt32 sphere, SQInt32 header, SqObj & payload) noexcept
{
    SphereCreated.Emit(sphere, header, payload);
    Reference< CSphere >::Get(sphere).SphereCreated.Emit(sphere, header, payload);
}

void Core::OnSpriteCreated(SQInt32 sprite, SQInt32 header, SqObj & payload) noexcept
{
    SpriteCreated.Emit(sprite, header, payload);
    Reference< CSprite >::Get(sprite).SpriteCreated.Emit(sprite, header, payload);
}

void Core::OnTextdrawCreated(SQInt32 textdraw, SQInt32 header, SqObj & payload) noexcept
{
    TextdrawCreated.Emit(textdraw, header, payload);
    Reference< CTextdraw >::Get(textdraw).TextdrawCreated.Emit(textdraw, header, payload);
}

void Core::OnVehicleCreated(SQInt32 vehicle, SQInt32 header, SqObj & payload) noexcept
{
    VehicleCreated.Emit(vehicle, header, payload);
    Reference< CVehicle >::Get(vehicle).VehicleCreated.Emit(vehicle, header, payload);
}

// ------------------------------------------------------------------------------------------------
void Core::OnBlipDestroyed(SQInt32 blip, SQInt32 header, SqObj & payload) noexcept
{
    BlipDestroyed.Emit(blip, header, payload);
    Reference< CBlip >::Get(blip).BlipDestroyed.Emit(blip, header, payload);
}

void Core::OnCheckpointDestroyed(SQInt32 checkpoint, SQInt32 header, SqObj & payload) noexcept
{
    CheckpointDestroyed.Emit(checkpoint, header, payload);
    Reference< CCheckpoint >::Get(checkpoint).CheckpointDestroyed.Emit(checkpoint, header, payload);
}

void Core::OnKeybindDestroyed(SQInt32 keybind, SQInt32 header, SqObj & payload) noexcept
{
    KeybindDestroyed.Emit(keybind, header, payload);
    Reference< CKeybind >::Get(keybind).KeybindDestroyed.Emit(keybind, header, payload);
}

void Core::OnObjectDestroyed(SQInt32 object, SQInt32 header, SqObj & payload) noexcept
{
    ObjectDestroyed.Emit(object, header, payload);
    Reference< CObject >::Get(object).ObjectDestroyed.Emit(object, header, payload);
}

void Core::OnPickupDestroyed(SQInt32 pickup, SQInt32 header, SqObj & payload) noexcept
{
    PickupDestroyed.Emit(pickup, header, payload);
    Reference< CPickup >::Get(pickup).PickupDestroyed.Emit(pickup, header, payload);
}

void Core::OnPlayerDestroyed(SQInt32 player, SQInt32 header, SqObj & payload) noexcept
{
    PlayerDestroyed.Emit(player, header, payload);
    Reference< CPlayer >::Get(player).PlayerDestroyed.Emit(player, header, payload);
}

void Core::OnSphereDestroyed(SQInt32 sphere, SQInt32 header, SqObj & payload) noexcept
{
    SphereDestroyed.Emit(sphere, header, payload);
    Reference< CSphere >::Get(sphere).SphereDestroyed.Emit(sphere, header, payload);
}

void Core::OnSpriteDestroyed(SQInt32 sprite, SQInt32 header, SqObj & payload) noexcept
{
    SpriteDestroyed.Emit(sprite, header, payload);
    Reference< CSprite >::Get(sprite).SpriteDestroyed.Emit(sprite, header, payload);
}

void Core::OnTextdrawDestroyed(SQInt32 textdraw, SQInt32 header, SqObj & payload) noexcept
{
    TextdrawDestroyed.Emit(textdraw, header, payload);
    Reference< CTextdraw >::Get(textdraw).TextdrawDestroyed.Emit(textdraw, header, payload);
}

void Core::OnVehicleDestroyed(SQInt32 vehicle, SQInt32 header, SqObj & payload) noexcept
{
    VehicleDestroyed.Emit(vehicle, header, payload);
    Reference< CVehicle >::Get(vehicle).VehicleDestroyed.Emit(vehicle, header, payload);
}

// ------------------------------------------------------------------------------------------------
void Core::OnBlipCustom(SQInt32 blip, SQInt32 header, SqObj & payload) noexcept
{
    BlipCustom.Emit(blip, header, payload);
    Reference< CBlip >::Get(blip).BlipCustom.Emit(blip, header, payload);
}

void Core::OnCheckpointCustom(SQInt32 checkpoint, SQInt32 header, SqObj & payload) noexcept
{
    CheckpointCustom.Emit(checkpoint, header, payload);
    Reference< CCheckpoint >::Get(checkpoint).CheckpointCustom.Emit(checkpoint, header, payload);
}

void Core::OnKeybindCustom(SQInt32 keybind, SQInt32 header, SqObj & payload) noexcept
{
    KeybindCustom.Emit(keybind, header, payload);
    Reference< CKeybind >::Get(keybind).KeybindCustom.Emit(keybind, header, payload);
}

void Core::OnObjectCustom(SQInt32 object, SQInt32 header, SqObj & payload) noexcept
{
    ObjectCustom.Emit(object, header, payload);
    Reference< CObject >::Get(object).ObjectCustom.Emit(object, header, payload);
}

void Core::OnPickupCustom(SQInt32 pickup, SQInt32 header, SqObj & payload) noexcept
{
    PickupCustom.Emit(pickup, header, payload);
    Reference< CPickup >::Get(pickup).PickupCustom.Emit(pickup, header, payload);
}

void Core::OnPlayerCustom(SQInt32 player, SQInt32 header, SqObj & payload) noexcept
{
    PlayerCustom.Emit(player, header, payload);
    Reference< CPlayer >::Get(player).PlayerCustom.Emit(player, header, payload);
}

void Core::OnSphereCustom(SQInt32 sphere, SQInt32 header, SqObj & payload) noexcept
{
    SphereCustom.Emit(sphere, header, payload);
    Reference< CSphere >::Get(sphere).SphereCustom.Emit(sphere, header, payload);
}

void Core::OnSpriteCustom(SQInt32 sprite, SQInt32 header, SqObj & payload) noexcept
{
    SpriteCustom.Emit(sprite, header, payload);
    Reference< CSprite >::Get(sprite).SpriteCustom.Emit(sprite, header, payload);
}

void Core::OnTextdrawCustom(SQInt32 textdraw, SQInt32 header, SqObj & payload) noexcept
{
    TextdrawCustom.Emit(textdraw, header, payload);
    Reference< CTextdraw >::Get(textdraw).TextdrawCustom.Emit(textdraw, header, payload);
}

void Core::OnVehicleCustom(SQInt32 vehicle, SQInt32 header, SqObj & payload) noexcept
{
    VehicleCustom.Emit(vehicle, header, payload);
    Reference< CVehicle >::Get(vehicle).VehicleCustom.Emit(vehicle, header, payload);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerAway(SQInt32 player, bool status) noexcept
{
    PlayerAway.Emit(player, status);
    Reference< CPlayer >::Get(player).PlayerAway.Emit(player, status);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerGameKeys(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept
{
    PlayerGameKeys.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerGameKeys.Emit(player, previous, current);
}

void Core::OnPlayerName(SQInt32 player, const SQChar * previous, const SQChar * current) noexcept
{
    PlayerRename.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerRename.Emit(player, previous, current);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerRequestClass(SQInt32 player, SQInt32 offset) noexcept
{
    PlayerRequestClass.Emit(player, offset);
    Reference< CPlayer >::Get(player).PlayerRequestClass.Emit(player, offset);
}

void Core::OnPlayerRequestSpawn(SQInt32 player) noexcept
{
    PlayerRequestSpawn.Emit(player);
    Reference< CPlayer >::Get(player).PlayerRequestSpawn.Emit(player);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerSpawn(SQInt32 player) noexcept
{
    PlayerSpawn.Emit(player);
    Reference< CPlayer >::Get(player).PlayerSpawn.Emit(player);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerStartTyping(SQInt32 player) noexcept
{
    PlayerStartTyping.Emit(player);
    Reference< CPlayer >::Get(player).PlayerStartTyping.Emit(player);
}

void Core::OnPlayerStopTyping(SQInt32 player) noexcept
{
    PlayerStopTyping.Emit(player);
    Reference< CPlayer >::Get(player).PlayerStopTyping.Emit(player);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerChat(SQInt32 player, const SQChar * message) noexcept
{
    PlayerChat.Emit(player, message);
    Reference< CPlayer >::Get(player).PlayerChat.Emit(player, message);
}

void Core::OnPlayerCommand(SQInt32 player, const SQChar * command) noexcept
{
    PlayerCommand.Emit(player, command);
    Reference< CPlayer >::Get(player).PlayerCommand.Emit(player, command);
}

void Core::OnPlayerMessage(SQInt32 player, SQInt32 receiver, const SQChar * message) noexcept
{
    PlayerMessage.Emit(player, receiver, message);
    Reference< CPlayer >::Get(player).PlayerMessage.Emit(player, receiver, message);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerHealth(SQInt32 player, SQFloat previous, SQFloat current) noexcept
{
    PlayerHealth.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerHealth.Emit(player, previous, current);
}

void Core::OnPlayerArmour(SQInt32 player, SQFloat previous, SQFloat current) noexcept
{
    PlayerArmour.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerArmour.Emit(player, previous, current);
}

void Core::OnPlayerWeapon(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept
{
    PlayerWeapon.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerWeapon.Emit(player, previous, current);
}

void Core::OnPlayerMove(SQInt32 player, const Vector3 & previous, const Vector3 & current) noexcept
{
    PlayerMove.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerMove.Emit(player, previous, current);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerWasted(SQInt32 player, SQInt32 reason) noexcept
{
    PlayerWasted.Emit(player, reason);
    Reference< CPlayer >::Get(player).PlayerWasted.Emit(player, reason);
}

void Core::OnPlayerKilled(SQInt32 player, SQInt32 killer, SQInt32 reason, SQInt32 body_part) noexcept
{
    PlayerKilled.Emit(player, killer, reason, body_part);
    Reference< CPlayer >::Get(player).PlayerKilled.Emit(player, killer, reason, body_part);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerSpectate(SQInt32 player, SQInt32 target) noexcept
{
    PlayerSpectate.Emit(player, target);
    Reference< CPlayer >::Get(player).PlayerSpectate.Emit(player, target);
}

void Core::OnPlayerCrashreport(SQInt32 player, const SQChar * report) noexcept
{
    PlayerCrashreport.Emit(player, report);
    Reference< CPlayer >::Get(player).PlayerCrashreport.Emit(player, report);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerBurning(SQInt32 player, bool state) noexcept
{
    PlayerBurning.Emit(player, state);
    Reference< CPlayer >::Get(player).PlayerBurning.Emit(player, state);
}

void Core::OnPlayerCrouching(SQInt32 player, bool state) noexcept
{
    PlayerCrouching.Emit(player, state);
    Reference< CPlayer >::Get(player).PlayerCrouching.Emit(player, state);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerState(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept
{
    PlayerState.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerState.Emit(player, previous, current);
}

void Core::OnPlayerAction(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept
{
    PlayerAction.Emit(player, previous, current);
    Reference< CPlayer >::Get(player).PlayerAction.Emit(player, previous, current);
}

// ------------------------------------------------------------------------------------------------
void Core::OnStateNone(SQInt32 player, SQInt32 previous) noexcept
{
    StateNone.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateNone.Emit(player, previous);
}

void Core::OnStateNormal(SQInt32 player, SQInt32 previous) noexcept
{
    StateNormal.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateNormal.Emit(player, previous);
}

void Core::OnStateShooting(SQInt32 player, SQInt32 previous) noexcept
{
    StateShooting.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateShooting.Emit(player, previous);
}

void Core::OnStateDriver(SQInt32 player, SQInt32 previous) noexcept
{
    StateDriver.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateDriver.Emit(player, previous);
}

void Core::OnStatePassenger(SQInt32 player, SQInt32 previous) noexcept
{
    StatePassenger.Emit(player, previous);
    Reference< CPlayer >::Get(player).StatePassenger.Emit(player, previous);
}

void Core::OnStateEnterDriver(SQInt32 player, SQInt32 previous) noexcept
{
    StateEnterDriver.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateEnterDriver.Emit(player, previous);
}

void Core::OnStateEnterPassenger(SQInt32 player, SQInt32 previous) noexcept
{
    StateEnterPassenger.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateEnterPassenger.Emit(player, previous);
}

void Core::OnStateExitVehicle(SQInt32 player, SQInt32 previous) noexcept
{
    StateExitVehicle.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateExitVehicle.Emit(player, previous);
}

void Core::OnStateUnspawned(SQInt32 player, SQInt32 previous) noexcept
{
    StateUnspawned.Emit(player, previous);
    Reference< CPlayer >::Get(player).StateUnspawned.Emit(player, previous);
}

// ------------------------------------------------------------------------------------------------
void Core::OnActionNone(SQInt32 player, SQInt32 previous) noexcept
{
    ActionNone.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionNone.Emit(player, previous);
}

void Core::OnActionNormal(SQInt32 player, SQInt32 previous) noexcept
{
    ActionNormal.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionNormal.Emit(player, previous);
}

void Core::OnActionAiming(SQInt32 player, SQInt32 previous) noexcept
{
    ActionAiming.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionAiming.Emit(player, previous);
}

void Core::OnActionShooting(SQInt32 player, SQInt32 previous) noexcept
{
    ActionShooting.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionShooting.Emit(player, previous);
}

void Core::OnActionJumping(SQInt32 player, SQInt32 previous) noexcept
{
    ActionJumping.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionJumping.Emit(player, previous);
}

void Core::OnActionLieDown(SQInt32 player, SQInt32 previous) noexcept
{
    ActionLieDown.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionLieDown.Emit(player, previous);
}

void Core::OnActionGettingUp(SQInt32 player, SQInt32 previous) noexcept
{
    ActionGettingUp.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionGettingUp.Emit(player, previous);
}

void Core::OnActionJumpVehicle(SQInt32 player, SQInt32 previous) noexcept
{
    ActionJumpVehicle.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionJumpVehicle.Emit(player, previous);
}

void Core::OnActionDriving(SQInt32 player, SQInt32 previous) noexcept
{
    ActionDriving.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionDriving.Emit(player, previous);
}

void Core::OnActionDying(SQInt32 player, SQInt32 previous) noexcept
{
    ActionDying.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionDying.Emit(player, previous);
}

void Core::OnActionWasted(SQInt32 player, SQInt32 previous) noexcept
{
    ActionWasted.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionWasted.Emit(player, previous);
}

void Core::OnActionEmbarking(SQInt32 player, SQInt32 previous) noexcept
{
    ActionEmbarking.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionEmbarking.Emit(player, previous);
}

void Core::OnActionDisembarking(SQInt32 player, SQInt32 previous) noexcept
{
    ActionDisembarking.Emit(player, previous);
    Reference< CPlayer >::Get(player).ActionDisembarking.Emit(player, previous);
}

// ------------------------------------------------------------------------------------------------
void Core::OnVehicleRespawn(SQInt32 vehicle) noexcept
{
    VehicleRespawn.Emit(vehicle);
    Reference< CVehicle >::Get(vehicle).VehicleRespawn.Emit(vehicle);
}

void Core::OnVehicleExplode(SQInt32 vehicle) noexcept
{
    VehicleExplode.Emit(vehicle);
    Reference< CVehicle >::Get(vehicle).VehicleExplode.Emit(vehicle);
}

// ------------------------------------------------------------------------------------------------
void Core::OnVehicleHealth(SQInt32 vehicle, SQFloat previous, SQFloat current) noexcept
{
    VehicleHealth.Emit(vehicle, previous, current);
    Reference< CVehicle >::Get(vehicle).VehicleHealth.Emit(vehicle, previous, current);
}

void Core::OnVehicleMove(SQInt32 vehicle, const Vector3 & previous, const Vector3 & current) noexcept
{
    VehicleMove.Emit(vehicle, previous, current);
    Reference< CVehicle >::Get(vehicle).VehicleMove.Emit(vehicle, previous, current);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPickupRespawn(SQInt32 pickup) noexcept
{
    PickupRespawn.Emit(pickup);
    Reference< CPickup >::Get(pickup).PickupRespawn.Emit(pickup);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerKeyPress(SQInt32 player, SQInt32 keybind) noexcept
{
    KeybindKeyPress.Emit(player, keybind);
    Reference< CKeybind >::Get(keybind).KeybindKeyPress.Emit(player, keybind);
    Reference< CPlayer >::Get(player).KeybindKeyPress.Emit(player, keybind);
}

void Core::OnPlayerKeyRelease(SQInt32 player, SQInt32 keybind) noexcept
{
    KeybindKeyRelease.Emit(player, keybind);
    Reference< CKeybind >::Get(keybind).KeybindKeyRelease.Emit(player, keybind);
    Reference< CPlayer >::Get(player).KeybindKeyRelease.Emit(player, keybind);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerEmbarking(SQInt32 player, SQInt32 vehicle, SQInt32 slot) noexcept
{
    VehicleEmbarking.Emit(player, vehicle, slot);
    Reference< CVehicle >::Get(vehicle).VehicleEmbarking.Emit(player, vehicle, slot);
    Reference< CPlayer >::Get(player).VehicleEmbarking.Emit(player, vehicle, slot);
}

void Core::OnPlayerEmbarked(SQInt32 player, SQInt32 vehicle, SQInt32 slot) noexcept
{
    VehicleEmbarked.Emit(player, vehicle, slot);
    Reference< CVehicle >::Get(vehicle).VehicleEmbarked.Emit(player, vehicle, slot);
    Reference< CPlayer >::Get(player).VehicleEmbarked.Emit(player, vehicle, slot);
}

void Core::OnPlayerDisembark(SQInt32 player, SQInt32 vehicle) noexcept
{
    VehicleDisembark.Emit(player, vehicle);
    Reference< CVehicle >::Get(vehicle).VehicleDisembark.Emit(player, vehicle);
    Reference< CPlayer >::Get(player).VehicleDisembark.Emit(player, vehicle);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPickupClaimed(SQInt32 player, SQInt32 pickup) noexcept
{
    PickupClaimed.Emit(player, pickup);
    Reference< CPickup >::Get(pickup).PickupClaimed.Emit(player, pickup);
    Reference< CPlayer >::Get(player).PickupClaimed.Emit(player, pickup);
}

void Core::OnPickupCollected(SQInt32 player, SQInt32 pickup) noexcept
{
    PickupClaimed.Emit(player, pickup);
    Reference< CPickup >::Get(pickup).PickupClaimed.Emit(player, pickup);
    Reference< CPlayer >::Get(player).PickupClaimed.Emit(player, pickup);
}

// ------------------------------------------------------------------------------------------------
void Core::OnObjectShot(SQInt32 player, SQInt32 object, SQInt32 weapon) noexcept
{
    ObjectShot.Emit(player, object, weapon);
    Reference< CObject >::Get(object).ObjectShot.Emit(player, object, weapon);
    Reference< CPlayer >::Get(player).ObjectShot.Emit(player, object, weapon);
}

void Core::OnObjectBump(SQInt32 player, SQInt32 object) noexcept
{
    ObjectBump.Emit(player, object);
    Reference< CObject >::Get(object).ObjectBump.Emit(player, object);
    Reference< CPlayer >::Get(player).ObjectBump.Emit(player, object);
}

// ------------------------------------------------------------------------------------------------
void Core::OnCheckpointEntered(SQInt32 player, SQInt32 checkpoint) noexcept
{
    CheckpointEntered.Emit(player, checkpoint);
    Reference< CCheckpoint >::Get(checkpoint).CheckpointEntered.Emit(player, checkpoint);
    Reference< CPlayer >::Get(player).CheckpointEntered.Emit(player, checkpoint);
}

void Core::OnCheckpointExited(SQInt32 player, SQInt32 checkpoint) noexcept
{
    CheckpointExited.Emit(player, checkpoint);
    Reference< CCheckpoint >::Get(checkpoint).CheckpointExited.Emit(player, checkpoint);
    Reference< CPlayer >::Get(player).CheckpointExited.Emit(player, checkpoint);
}

// ------------------------------------------------------------------------------------------------
void Core::OnSphereEntered(SQInt32 player, SQInt32 sphere) noexcept
{
    SphereEntered.Emit(player, sphere);
    Reference< CSphere >::Get(sphere).SphereEntered.Emit(player, sphere);
    Reference< CPlayer >::Get(player).SphereEntered.Emit(player, sphere);
}

void Core::OnSphereExited(SQInt32 player, SQInt32 sphere) noexcept
{
    SphereExited.Emit(player, sphere);
    Reference< CSphere >::Get(sphere).SphereExited.Emit(player, sphere);
    Reference< CPlayer >::Get(player).SphereExited.Emit(player, sphere);
}

// ------------------------------------------------------------------------------------------------
void Core::OnServerFrame(SQFloat delta) noexcept
{
    ServerFrame.Emit(delta);
}

// ------------------------------------------------------------------------------------------------
void Core::OnServerStartup() noexcept
{
    ServerStartup.Emit();
}

void Core::OnServerShutdown() noexcept
{
    ServerShutdown.Emit();
}

// ------------------------------------------------------------------------------------------------
void Core::OnInternalCommand(SQInt32 type, const SQChar * text) noexcept
{
    InternalCommand.Emit(type, text);
}

void Core::OnLoginAttempt(const SQChar * name, const SQChar * passwd, const SQChar * ip) noexcept
{
    LoginAttempt.Emit(name, passwd, ip);
}

// ------------------------------------------------------------------------------------------------
void Core::OnCustomEvent(SQInt32 group, SQInt32 header, SqObj & payload) noexcept
{
    CustomEvent.Emit(group, header, payload);
}

// ------------------------------------------------------------------------------------------------
void Core::OnWorldOption(SQInt32 option, SqObj & value) noexcept
{
    WorldOption.Emit(option, value);
}

void Core::OnWorldToggle(SQInt32 option, bool value) noexcept
{
    WorldToggle.Emit(option, value);
}

// ------------------------------------------------------------------------------------------------
void Core::OnScriptReload(SQInt32 header, SqObj & payload) noexcept
{
    ScriptReload.Emit(header, payload);
}

// ------------------------------------------------------------------------------------------------
void Core::OnLogMessage(SQInt32 type, const SQChar * message) noexcept
{
    LogMessage.Emit(type, message);
}

// ------------------------------------------------------------------------------------------------
void Core::OnPlayerUpdate(SQInt32 player, SQInt32 type) noexcept
{
    Vector3 pos;
    _Func->GetPlayerPos(player, &pos.x, &pos.y, &pos.z);

    if (m_PlayerTrack[player].Fresh)
    {
        m_PlayerTrack[player].Position =  pos;
        m_PlayerTrack[player].Health = _Func->GetPlayerHealth(player);
        m_PlayerTrack[player].Armour = _Func->GetPlayerArmour(player);
        m_PlayerTrack[player].Weapon = _Func->GetPlayerWeapon(player);
        m_PlayerTrack[player].Fresh = false;
        return;
    }

    if (pos != m_PlayerTrack[player].Position)
    {
        PlayerMove.Emit(player, m_PlayerTrack[player].Position, pos);
        m_PlayerTrack[player].Position = pos;
    }

    SQFloat health = _Func->GetPlayerHealth(player);

    if (!EpsEq(health, m_PlayerTrack[player].Health))
    {
        PlayerHealth.Emit(player, m_PlayerTrack[player].Health, health);
        m_PlayerTrack[player].Health = health;
    }

    SQFloat armour = _Func->GetPlayerArmour(player);

    if (!EpsEq(armour, m_PlayerTrack[player].Armour))
    {
        PlayerArmour.Emit(player, m_PlayerTrack[player].Armour, armour);
        m_PlayerTrack[player].Armour = armour;
    }

    SQInteger wep = _Func->GetPlayerWeapon(player);

    if (wep != m_PlayerTrack[player].Weapon)
    {
        PlayerWeapon.Emit(player, m_PlayerTrack[player].Weapon, wep);
        m_PlayerTrack[player].Weapon = wep;
    }
}

void Core::OnVehicleUpdate(SQInt32 vehicle, SQInt32 type) noexcept
{
    Vector3 pos;
    _Func->GetVehiclePos(vehicle, &pos.x, &pos.y, &pos.z);

    if (m_VehicleTrack[vehicle].Fresh)
    {
        m_VehicleTrack[vehicle].Position =  pos;
        m_VehicleTrack[vehicle].Health = _Func->GetVehicleHealth(vehicle);
        m_VehicleTrack[vehicle].Fresh = false;
        return;
    }

    if (pos != m_VehicleTrack[vehicle].Position)
    {
        VehicleMove.Emit(vehicle, m_VehicleTrack[vehicle].Position, pos);
        m_VehicleTrack[vehicle].Position = pos;
    }

    SQFloat health = _Func->GetVehicleHealth(vehicle);

    if (!EpsEq(health, m_VehicleTrack[vehicle].Health))
    {
        VehicleHealth.Emit(vehicle, m_VehicleTrack[vehicle].Health, health);
        m_VehicleTrack[vehicle].Health = health;
    }
}

void Core::OnEntityPool(SQInt32 type, SQInt32 id, bool deleted) noexcept
{
    static SqObj payload;
    payload.Release();
    switch (type)
    {
        case SQMOD_ENTITY_POOL_VEHICLE:
            if (deleted)
            {
                DestroyVehicle(id, SQMOD_DESTROY_POOL, payload);
            }
            else if (EntMan< CVehicle >::Activate(id, false))
            {
                OnVehicleCreated(id, SQMOD_CREATE_POOL, payload);
            }
        break;
        case SQMOD_ENTITY_POOL_OBJECT:
            if (deleted)
            {
                DestroyObject(id, SQMOD_DESTROY_POOL, payload);
            }
            else if (EntMan< CObject >::Activate(id, false))
            {
                OnObjectCreated(id, SQMOD_CREATE_POOL, payload);
            }
        break;
        case SQMOD_ENTITY_POOL_PICKUP:
            if (deleted)
            {
                DestroyPickup(id, SQMOD_DESTROY_POOL, payload);
            }
            else if (EntMan< CPickup >::Activate(id, false))
            {
                OnPickupCreated(id, SQMOD_CREATE_POOL, payload);
            }
        break;
        case SQMOD_ENTITY_POOL_RADIO:
            // @TODO Implement...
        break;
        case SQMOD_ENTITY_POOL_SPRITE:
            if (deleted)
            {
                DestroySprite(id, SQMOD_DESTROY_POOL, payload);
            }
            else if (EntMan< CSprite >::Activate(id, false, ""))
            {
                OnSpriteCreated(id, SQMOD_CREATE_POOL, payload);
            }
        break;
        case SQMOD_ENTITY_POOL_TEXTDRAW:
            if (deleted)
            {
                DestroyTextdraw(id, SQMOD_DESTROY_POOL, payload);
            }
            else if (EntMan< CTextdraw >::Activate(id, false, ""))
            {
                OnTextdrawCreated(id, SQMOD_CREATE_POOL, payload);
            }
        break;
        case SQMOD_ENTITY_POOL_BLIP:
            if (deleted)
            {
                DestroyBlip(id, SQMOD_DESTROY_POOL, payload);
            }
            else
            {
                SQInt32 world, scale, sprite;
                SQUint32 pcolor;
                Vector3 pos;
                Color4 color;
                // Get the blip information from the server
                _Func->GetCoordBlipInfo(id, &world, &pos.x, &pos.y, &pos.z, &scale, &pcolor, &sprite);
                // Unpack the retrieved color
                color.SetRGBA(pcolor);

                if (EntMan< CBlip >::Activate(id, false, world, scale, sprite, pos, color))
                {
                    OnBlipCreated(id, SQMOD_CREATE_POOL, payload);
                }
            }
        break;
        default:
            LogErr("Unknown change in the entity pool of type: %d", type);
    }

}


} // Namespace:: SqMod
