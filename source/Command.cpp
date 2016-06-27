// ------------------------------------------------------------------------------------------------
#include "Command.hpp"
#include "Core.hpp"
#include "Entity/Player.hpp"

// ------------------------------------------------------------------------------------------------
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <algorithm>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
CmdManager CmdManager::s_Inst;

// ------------------------------------------------------------------------------------------------
SQInteger CmdListener::Typename(HSQUIRRELVM vm)
{
    static const SQChar name[] = _SC("SqCmdListener");
    sq_pushstring(vm, name, sizeof(name));
    return 1;
}

// ------------------------------------------------------------------------------------------------
static void ValidateName(CSStr name)
{
    // Is the name empty?
    if (!name || *name == '\0')
    {
        STHROWF("Invalid or empty command name");
    }
    // Inspect name characters
    while (*name != '\0')
    {
        // Does it contain spaces?
        if (std::isspace(*name) != 0)
        {
            STHROWF("Command names cannot contain spaces");
        }
        // Move to the next character
        ++name;
    }
}

// ------------------------------------------------------------------------------------------------
CSStr CmdArgSpecToStr(Uint8 spec)
{
    switch (spec)
    {
        case CMDARG_ANY:        return _SC("any");
        case CMDARG_INTEGER:    return _SC("integer");
        case CMDARG_FLOAT:      return _SC("float");
        case CMDARG_BOOLEAN:    return _SC("boolean");
        case CMDARG_STRING:
        case CMDARG_LOWER:
        case CMDARG_UPPER:
        case CMDARG_GREEDY:     return _SC("string");
        default:                return _SC("unknown");
    }
}

/* ------------------------------------------------------------------------------------------------
 * Forward the call to initialize the command manager.
*/
void InitializeCmdManager()
{
    CmdManager::Get().Initialize();
}

/* ------------------------------------------------------------------------------------------------
 * Forward the call to terminate the command manager.
*/
void TerminateCmdManager()
{
    CmdManager::Get().Deinitialize();
}

// ------------------------------------------------------------------------------------------------
CmdManager::Context::Context(Int32 invoker)
    : mBuffer(512)
    , mInvoker(invoker)
    , mCommand()
    , mArgument()
    , mInstance(nullptr)
    , mObject()
    , mArgv()
    , mArgc(0)
{
    // Reserve enough space upfront
    mCommand.reserve(64);
    mArgument.reserve(512);
}

// ------------------------------------------------------------------------------------------------
CmdManager::Guard::Guard(const CtxRef & ctx)
    : mPrevious(CmdManager::Get().m_Context), mCurrent(ctx)
{
    CmdManager::Get().m_Context = mCurrent;
}

// ------------------------------------------------------------------------------------------------
CmdManager::Guard::~Guard()
{
    if (CmdManager::Get().m_Context == mCurrent)
    {
        CmdManager::Get().m_Context = mPrevious;
    }
}

// ------------------------------------------------------------------------------------------------
Object & CmdManager::Attach(const String & name, CmdListener * ptr, bool autorel)
{
    // Obtain the unique identifier of the specified name
    const std::size_t hash = std::hash< String >()(name);
    // Make sure the command doesn't already exist
    for (const auto & cmd : m_Commands)
    {
        // Are the hashes identical?
        if (cmd.mHash == hash)
        {
            // Do we have to release this listener instance our self?
            if (autorel)
            {
                delete ptr; // Let's avoid memory leaks!
            }
            // Now it's safe to throw the exception
            // (include information necessary to help identify hash collisions!)
            STHROWF("Command '%s' already exists as '%s' for hash (%zu)",
                        name.c_str(), cmd.mName.c_str(), hash);
        }
    }
    // Obtain the initial stack size
    const StackGuard sg;
    // Push this instance on the stack
    ClassType< CmdListener >::PushInstance(DefaultVM::Get(), ptr);
    // Attempt to insert the command
    m_Commands.emplace_back(hash, name, Var< Object >(DefaultVM::Get(), -1).value);
    // Return the script object of the listener
    return m_Commands.back().mObj;
}

// ------------------------------------------------------------------------------------------------
void CmdManager::Detach(const String & name)
{
    // Obtain the unique identifier of the specified name
    const std::size_t hash = std::hash< String >()(name);
    // Iterator to the found command, if any
    CmdList::const_iterator itr = m_Commands.cbegin();
    // Attempt to find the specified command
    for (; itr != m_Commands.cend(); ++itr)
    {
        // Are the hashes identical?
        if (itr->mHash == hash)
        {
            break; // We found our command!
        }
    }
    // Make sure the command exist before attempting to remove it
    if (itr != m_Commands.end())
    {
        m_Commands.erase(itr);
    }
}

// ------------------------------------------------------------------------------------------------
void CmdManager::Detach(CmdListener * ptr)
{
    // Iterator to the found command, if any
    CmdList::const_iterator itr = m_Commands.cbegin();
    // Attempt to find the specified command
    for (; itr != m_Commands.cend(); ++itr)
    {
        // Are the instances identical?
        if (itr->mPtr == ptr)
        {
            break; // We found our command!
        }
    }
    // Make sure the command exists before attempting to remove it
    if (itr != m_Commands.end())
    {
        m_Commands.erase(itr);
    }
}

// ------------------------------------------------------------------------------------------------
bool CmdManager::Attached(const String & name) const
{
    // Obtain the unique identifier of the specified name
    const std::size_t hash = std::hash< String >()(name);
    // Attempt to find the specified command
    for (const auto & cmd : m_Commands)
    {
        // Are the hashes identical?
        if (cmd.mHash == hash)
        {
            return true; // We found our command!
        }
    }
    // No such command exists
    return false;
}

// ------------------------------------------------------------------------------------------------
bool CmdManager::Attached(const CmdListener * ptr) const
{
    // Attempt to find the specified command
    for (const auto & cmd : m_Commands)
    {
        // Are the instances identical?
        if (cmd.mPtr == ptr)
        {
            return true; // We found our command!
        }
    }
    // No such command exists
    return false;
}

// ------------------------------------------------------------------------------------------------
CmdManager::CmdManager()
    : m_Commands()
    , m_Context()
    , m_OnFail()
    , m_OnAuth()
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
CmdManager::~CmdManager()
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
void CmdManager::Initialize()
{

}

// ------------------------------------------------------------------------------------------------
void CmdManager::Deinitialize()
{
    // Release the script resources from command instances
    for (const auto & cmd : m_Commands)
    {
        if (cmd.mPtr)
        {
            // Release the local command callbacks
            cmd.mPtr->m_OnExec.ReleaseGently();
            cmd.mPtr->m_OnAuth.ReleaseGently();
            cmd.mPtr->m_OnPost.ReleaseGently();
            cmd.mPtr->m_OnFail.ReleaseGently();
        }
    }
    // Clear the command list and release all references
    m_Commands.clear();
	// Release the current context if any
	m_Context.Reset();
    // Release the global callbacks
    m_OnFail.ReleaseGently();
    m_OnAuth.ReleaseGently();
}

// ------------------------------------------------------------------------------------------------
void CmdManager::Sort()
{
    std::sort(m_Commands.begin(), m_Commands.end(),
        [](CmdList::const_reference a, CmdList::const_reference b) -> bool {
            return (b.mName < a.mName);
        });
}

// ------------------------------------------------------------------------------------------------
const Object & CmdManager::FindByName(const String & name)
{
    // Obtain the unique identifier of the specified name
    const std::size_t hash = std::hash< String >()(name);
    // Attempt to find the specified command
    for (const auto & cmd : m_Commands)
    {
        // Are the hashes identical?
        if (cmd.mHash == hash)
        {
            return cmd.mObj; // We found our command!
        }
    }
    // No such command exist
    return NullObject();
}

// ------------------------------------------------------------------------------------------------
Int32 CmdManager::Run(Int32 invoker, CCStr command)
{
    // Validate the string command
    if (!command || *command == '\0')
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EMPTY_COMMAND, _SC("Invalid or empty command name"), invoker);
        // Execution failed!
        return -1;
    }
    // Create the command execution context
    CtxRef ctx_ref(new Context(invoker));
    // Grab a direct reference to the context instance
    Context & ctx = *ctx_ref;
    // Skip white-space until the command name
    while (std::isspace(*command))
    {
        ++command;
    }
    // Anything left to process?
    if (*command == '\0')
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EMPTY_COMMAND, _SC("Invalid or empty command name"), invoker);
        // Execution failed!
        return -1;
    }
    // Where the name ends and argument begins
    CCStr split = command;
    // Find where the command name ends
    while (*split != '\0' && !std::isspace(*split))
    {
        ++split;
    }
    // Are there any arguments specified?
    if (split != '\0')
    {
        // Save the command name
        ctx.mCommand.assign(command, (split - command));
        // Skip white space after command name
        while (std::isspace(*split))
        {
            ++split;
        }
        // Save the command argument
        ctx.mArgument.assign(split);
    }
    // No arguments specified
    else
    {
        // Save the command name
        ctx.mCommand.assign(command);
        // Leave argument empty
        ctx.mArgument.assign(_SC(""));
    }
    // Do we have a valid command name?
    try
    {
        ValidateName(ctx.mCommand.c_str());
    }
    catch (const Sqrat::Exception & e)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_INVALID_COMMAND, ToStrF("%s", e.Message().c_str()), invoker);
        // Execution failed!
        return -1;
    }
    catch (...)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_INVALID_COMMAND, _SC("Cannot execute invalid command name"), invoker);
        // Execution failed!
        return -1;
    }
    // We're about to enter execution land, so let's be safe
    const Guard g(ctx_ref);
    // Attempt to find the specified command
    ctx.mObject = FindByName(ctx.mCommand);
    // Have we found anything?
    if (ctx.mObject.IsNull())
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_UNKNOWN_COMMAND, _SC("Unable to find the specified command"), ctx.mCommand);
        // Execution failed!
        return -1;
    }
    // Save the command instance
    ctx.mInstance = ctx.mObject.Cast< CmdListener * >();
    // Is the command instance valid? (just in case)
    if (!ctx.mInstance)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_UNKNOWN_COMMAND, _SC("Unable to find the specified command"), ctx.mCommand);
        // Execution failed!
        return -1;
    }
    // Attempt to execute the command
    try
    {
        return Exec(ctx);
    }
    catch (...)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EXECUTION_FAILED, _SC("Exceptions occurred during execution"), ctx.mInvoker);
    }
    // Execution failed
    return -1;
}

// ------------------------------------------------------------------------------------------------
Int32 CmdManager::Exec(Context & ctx)
{
    // Clear previous arguments
    ctx.mArgv.clear();
    // Reset the argument counter
    ctx.mArgc = 0;
    // Make sure the invoker has enough authority to execute this command
    if (!ctx.mInstance->AuthCheckID(ctx.mInvoker))
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_INSUFFICIENT_AUTH, _SC("Insufficient authority to execute command"), ctx.mInvoker);
        // Execution failed!
        return -1;
    }
    // Make sure an executer was specified
    else if (ctx.mInstance->GetOnExec().IsNull())
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_MISSING_EXECUTER, _SC("No executer was specified for this command"), ctx.mInvoker);
        // Execution failed!
        return -1;
    }
    // See if there are any arguments to parse
    else if (!ctx.mArgument.empty() && !Parse(ctx))
    {
        // The error message was reported while parsing
        return -1;
    }
    // Make sure we have enough arguments specified
    else if (ctx.mInstance->GetMinArgC() > ctx.mArgc)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_INCOMPLETE_ARGS, _SC("Incomplete command arguments"), ctx.mInstance->GetMinArgC());
        // Execution failed!
        return -1;
    }
    // The check during the parsing may omit the last argument
    else if (ctx.mInstance->GetMaxArgC() < ctx.mArgc)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EXTRANEOUS_ARGS, _SC("Extraneous command arguments"), ctx.mInstance->GetMaxArgC());
        // Execution failed!
        return -1;
    }
    // Check argument types against the command specifiers
    for (Uint32 arg = 0; arg < ctx.mArgc; ++arg)
    {
        if (!ctx.mInstance->ArgCheck(arg, ctx.mArgv[arg].first))
        {
            // Tell the script callback to deal with the error
            SqError(CMDERR_UNSUPPORTED_ARG, _SC("Unsupported command argument"), arg);
            // Execution failed!
            return -1;
        }
    }
    // Result of the command execution
    SQInteger result = -1;
    // Clear any data from the buffer to make room for the error message
    ctx.mBuffer.At(0) = '\0';
    // Whether the command execution failed
    bool failed = false;
    // Do we have to call the command with an associative container?
    if (ctx.mInstance->m_Associate)
    {
        // Create the associative container
        Table args(DefaultVM::Get());
        // Copy the arguments into the table
        for (Uint32 arg = 0; arg < ctx.mArgc; ++arg)
        {
            // Do we have use the argument index as the key?
            if (ctx.mInstance->m_ArgTags[arg].empty())
            {
                args.SetValue(SQInteger(arg), ctx.mArgv[arg].second);
            }
            // Nope, we have a name for this argument!
            else
            {
                args.SetValue(ctx.mInstance->m_ArgTags[arg].c_str(), ctx.mArgv[arg].second);
            }
        }
        // Attempt to execute the command with the specified arguments
        try
        {
            result = ctx.mInstance->Execute(Core::Get().GetPlayer(ctx.mInvoker).mObj, args);
        }
        catch (const Sqrat::Exception & e)
        {
            // Let's store the exception message
            ctx.mBuffer.Write(0, e.Message().c_str(), e.Message().size());
            // Specify that the command execution failed
            failed = true;
        }
        catch (const std::exception & e)
        {
            // Let's store the exception message
            ctx.mBuffer.WriteF(0, "Application exception occurred [%s]", e.what());
            // Specify that the command execution failed
            failed = true;
        }
    }
    else
    {
        // Reserve an array for the extracted arguments
        Array args(DefaultVM::Get(), ctx.mArgc);
        // Copy the arguments into the array
        for (Uint32 arg = 0; arg < ctx.mArgc; ++arg)
        {
            args.Bind(SQInteger(arg), ctx.mArgv[arg].second);
        }
        // Attempt to execute the command with the specified arguments
        try
        {
            result = ctx.mInstance->Execute(Core::Get().GetPlayer(ctx.mInvoker).mObj, args);
        }
        catch (const Sqrat::Exception & e)
        {
            // Let's store the exception message
            ctx.mBuffer.Write(0, e.Message().c_str(), e.Message().size());
            // Specify that the command execution failed
            failed = true;
        }
        catch (const std::exception & e)
        {
            // Let's store the exception message
            ctx.mBuffer.WriteF(0, "Application exception occurred [%s]", e.what());
            // Specify that the command execution failed
            failed = true;
        }
    }
    // Was there a runtime exception during the execution?
    if (failed)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EXECUTION_FAILED, _SC("Command execution failed"), ctx.mBuffer.Data());
        // Is there a script callback that handles failures?
        if (!ctx.mInstance->m_OnFail.IsNull())
        {
            // Then attempt to relay the result to that function
            try
            {
                ctx.mInstance->m_OnFail.Execute(Core::Get().GetPlayer(ctx.mInvoker).mObj, result);
            }
            catch (const Sqrat::Exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_UNRESOLVED_FAILURE, _SC("Unable to resolve command failure"), e.Message());
            }
            catch (const std::exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_UNRESOLVED_FAILURE, _SC("Unable to resolve command failure"), e.what());
            }
        }
        // Result is invalid at this point
        result = -1;
    }
    // Was the command aborted explicitly?
    else if (!result)
    {
        // Tell the script callback to deal with the error
        SqError(CMDERR_EXECUTION_ABORTED, _SC("Command execution aborted"), result);
        // Is there a script callback that handles failures?
        if (!ctx.mInstance->m_OnFail.IsNull())
        {
            // Then attempt to relay the result to that function
            try
            {
                ctx.mInstance->m_OnFail.Execute(Core::Get().GetPlayer(ctx.mInvoker).mObj, result);
            }
            catch (const Sqrat::Exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_UNRESOLVED_FAILURE, _SC("Unable to resolve command failure"), e.Message());
            }
            catch (const std::exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_UNRESOLVED_FAILURE, _SC("Unable to resolve command failure"), e.what());
            }
        }
    }
    // Is there a callback that must be executed after a successful execution?
    else if (!ctx.mInstance->m_OnPost.IsNull())
    {
            // Then attempt to relay the result to that function
            try
            {
                ctx.mInstance->m_OnPost.Execute(Core::Get().GetPlayer(ctx.mInvoker).mObj, result);
            }
            catch (const Sqrat::Exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_POST_PROCESSING_FAILED, _SC("Unable to complete command post processing"), e.Message());
            }
            catch (const std::exception & e)
            {
                // Tell the script callback to deal with the error
                SqError(CMDERR_POST_PROCESSING_FAILED, _SC("Unable to complete command post processing"), e.what());
            }
    }
    // Return the result
    return ConvTo< Int32 >::From(result);
}

// ------------------------------------------------------------------------------------------------
bool CmdManager::Parse(Context & ctx)
{
    // Is there anything to parse?
    if (ctx.mArgument.empty())
    {
        return true; // Done parsing!
    }
    // Obtain the flags of the currently processed argument
    Uint8 arg_flags = ctx.mInstance->m_ArgSpec[ctx.mArgc];
    // Adjust the internal buffer if necessary (mostly never)
    ctx.mBuffer.Adjust(ctx.mArgument.size());
    // The iterator to the currently processed character
    String::const_iterator itr = ctx.mArgument.cbegin();
    // Previous and currently processed character
    String::value_type prev = 0, elem = 0;
    // Maximum arguments allowed to be processed
    const Uint8 max_arg = ctx.mInstance->m_MaxArgc;
    // Process loop result
    bool good = true;
    // Process the specified command text
    while (good)
    {
        // Extract the current characters before advancing
        prev = elem, elem = *itr;
        // See if we have anything left to parse or we have what we need already
        if (elem == '\0' || ctx.mArgc >= max_arg)
        {
            break; // We only parse what we need or what we have!
        }
        // Is this a greedy argument?
        else if (arg_flags & CMDARG_GREEDY)
        {
            // Remember the current stack size
            const StackGuard sg;
            // Skip white-space characters
            itr = std::find_if(itr, ctx.mArgument.cend(), IsNotCType(std::isspace));
            // Anything left to copy to the argument?
            if (itr != ctx.mArgument.end())
            {
                // Transform it into a script object
                sq_pushstring(DefaultVM::Get(), &(*itr), std::distance(itr, ctx.mArgument.cend()));
            }
            // Just push an empty string
            else
            {
                sq_pushstring(DefaultVM::Get(), _SC(""), 0);
            }
            // Get the object from the stack and add it to the argument list along with it's type
            ctx.mArgv.emplace_back(CMDARG_STRING, Var< Object >(DefaultVM::Get(), -1).value);
            // Include this argument into the count
            ++ctx.mArgc;
            // Nothing left to parse
            break;
        }
        // Do we have to extract a string argument?
        else if ((elem == '\'' || elem == '"') && prev != '\\')
        {
            // Obtain the beginning and ending of the internal buffer
            SStr str = ctx.mBuffer.Begin< SQChar >();
            SStr end = (ctx.mBuffer.End< SQChar >() - 1); // + null terminator
            // Save the closing quote type
            const SQChar close = elem;
            // Skip the opening quote
            ++itr;
            // Attempt to consume the string argument
            while (good)
            {
                // Extract the current characters before advancing
                prev = elem, elem = *itr;
                // See if there's anything left to parse
                if (elem == '\0')
                {
                    // Tell the script callback to deal with the error
                    SqError(CMDERR_SYNTAX_ERROR, _SC("String argument not closed properly"), ctx.mArgc);
                    // Parsing aborted
                    good = false;
                    // Stop parsing
                    break;
                }
                // First un-escaped matching quote character ends the argument
                else if (elem == close)
                {
                    // Was this not escaped?
                    if (prev != '\\')
                    {
                        // Terminate the string value in the internal buffer
                        *str = '\0';
                        // Stop parsing
                        break;
                    }
                    // Overwrite last character when replicating
                    else
                    {
                        --str;
                    }
                }
                // See if the internal buffer needs to scale
                else if  (str >= end)
                {
                    // We should already have a buffer as big as the entire command!
                    SqError(CMDERR_BUFFER_OVERFLOW, _SC("Command buffer was exceeded unexpectedly"), ctx.mInvoker);
                    // Parsing aborted
                    good = false;
                    // Stop parsing
                    break;
                }
                // Simply replicate the character to the internal buffer
                *str = elem;
                // Advance to the next character
                ++str, ++itr;
            }
            // See if the argument was valid
            if (!good)
            {
                break; // Propagate the failure!
            }
            // Swap the beginning and ending of the extracted string
            end = str, str = ctx.mBuffer.Begin();
            // Make sure the string is null terminated
            *end = '\0';
            // Do we have to make the string lowercase?
            if (arg_flags & CMDARG_LOWER)
            {
                for (CStr chr = str; chr < end; ++chr)
                {
                    *chr = static_cast< SQChar >(std::tolower(*chr));
                }
            }
            // Do we have to make the string uppercase?
            else if (arg_flags & CMDARG_UPPER)
            {
                for (CStr chr = str; chr < end; ++chr)
                {
                    *chr = static_cast< SQChar >(std::toupper(*chr));
                }
            }
            // Remember the current stack size
            const StackGuard sg;
            // Was the specified string empty?
            if (str >= end)
            {
                // Just push an empty string
                sq_pushstring(DefaultVM::Get(), _SC(""), 0);
            }
            // Add it to the argument list along with it's type
            else
            {
                // Transform it into a script object
                sq_pushstring(DefaultVM::Get(), str, end - str - 1);
            }
            // Get the object from the stack and add it to the argument list along with it's type
            ctx.mArgv.emplace_back(CMDARG_STRING, Var< Object >(DefaultVM::Get(), -1).value);
            // Advance to the next argument and obtain its flags
            arg_flags = ctx.mInstance->m_ArgSpec[++ctx.mArgc];
        }
        // Ignore white-space characters until another valid character is found
        else if (!std::isspace(elem) && (std::isspace(prev) || prev == '\0'))
        {
            // Find the first space character that marks the end of the argument
            String::const_iterator pos = std::find_if(itr, ctx.mArgument.cend(), IsCType(std::isspace));
            // Obtain both ends of the argument string
            CCStr str = &(*itr), end = &(*pos);
            // Compute the argument string size
            const Uint32 sz = std::distance(itr, pos);
            // Update the main iterator position
            itr = pos;
            // Update the currently processed character
            elem = *itr;
            // Used to exclude all other checks when a valid type was identified
            bool identified = false;
            // Attempt to treat the value as an integer number if possible
            if (!identified && (arg_flags & CMDARG_INTEGER))
            {
                // Let's us know if the whole argument was part of the resulted value
                CStr next = nullptr;
                // Attempt to extract the integer value from the string
                const Int64 value = std::strtoll(str, &next, 10);
                // See if this whole string was indeed an integer
                if (next == end)
                {
                    // Remember the current stack size
                    const StackGuard sg;
                    // Transform it into a script object
                    sq_pushinteger(DefaultVM::Get(), ConvTo< SQInteger >::From(value));
                    // Get the object from the stack and add it to the argument list along with it's type
                    ctx.mArgv.emplace_back(CMDARG_INTEGER, Var< Object >(DefaultVM::Get(), -1).value);
                    // We've identified the correct value type
                    identified = false;
                }
            }
            // Attempt to treat the value as an floating point number if possible
            if (!identified && (arg_flags & CMDARG_FLOAT))
            {
                // Let's us know if the whole argument was part of the resulted value
                CStr next = nullptr;
                // Attempt to extract the integer value from the string
#ifdef SQUSEDOUBLE
                const Float64 value = std::strtod(str, &next);
#else
                const Float32 value = std::strtof(str, &next);
#endif // SQUSEDOUBLE
                // See if this whole string was indeed an integer
                if (next == end)
                {
                    // Remember the current stack size
                    const StackGuard sg;
                    // Transform it into a script object
                    sq_pushfloat(DefaultVM::Get(), ConvTo< SQFloat >::From(value));
                    // Get the object from the stack and add it to the argument list along with it's type
                    ctx.mArgv.emplace_back(CMDARG_FLOAT, Var< Object >(DefaultVM::Get(), -1).value);
                    // We've identified the correct value type
                    identified = false;
                }

            }
            // Attempt to treat the value as a boolean if possible
            if (!identified && (arg_flags & CMDARG_BOOLEAN) && sz <= 5)
            {
                // Allocate memory for enough data to form a boolean value
                CharT lc[6];
                // Fill the temporary buffer with data from the internal buffer
                std::snprintf(lc, 6, "%.5s", str);
                // Convert all characters to lowercase
                for (Uint32 i = 0; i < 5; ++i)
                {
                    lc[i] = std::tolower(lc[i]);
                }
                // Remember the current stack size
                const StackGuard sg;
                // Is this a boolean true value?
                if (std::strcmp(lc, "true") == 0 || std::strcmp(lc, "on") == 0)
                {
                    // Transform it into a script object
                    sq_pushbool(DefaultVM::Get(), true);
                    // We've identified the correct value type
                    identified = true;
                }
                // Is this a boolean false value?
                else if (std::strcmp(lc, "false") == 0 || std::strcmp(lc, "off") == 0)
                {
                    // Transform it into a script object
                    sq_pushbool(DefaultVM::Get(), false);
                    // We've identified the correct value type
                    identified = true;
                }
                // Could the value inside the string be interpreted as a boolean?
                if (identified)
                {
                    // Get the object from the stack and add it to the argument list along with it's type
                    ctx.mArgv.emplace_back(CMDARG_BOOLEAN, Var< Object >(DefaultVM::Get(), -1).value);
                }
            }
            // If everything else failed then simply treat the value as a string
            if (!identified)
            {
                // Remember the current stack size
                const StackGuard sg;
                // Do we have to make the string lowercase?
                if (arg_flags & CMDARG_LOWER)
                {
                    // Convert all characters from the argument string into the buffer
                    for (CStr chr = ctx.mBuffer.Data(); str < end; ++str, ++chr)
                    {
                        *chr = static_cast< CharT >(std::tolower(*str));
                    }
                    // Transform it into a script object
                    sq_pushstring(DefaultVM::Get(), ctx.mBuffer.Get< SQChar >(), sz);
                }
                // Do we have to make the string uppercase?
                else if (arg_flags & CMDARG_UPPER)
                {
                    // Convert all characters from the argument string into the buffer
                    for (CStr chr = ctx.mBuffer.Data(); str < end; ++str, ++chr)
                    {
                        *chr = static_cast< CharT >(std::toupper(*str));
                    }
                    // Transform it into a script object
                    sq_pushstring(DefaultVM::Get(), ctx.mBuffer.Get< SQChar >(), sz);
                }
                else
                {
                    // Transform it into a script object
                    sq_pushstring(DefaultVM::Get(), str, sz);
                }
                // Get the object from the stack and add it to the argument list along with it's type
                ctx.mArgv.emplace_back(CMDARG_STRING, Var< Object >(DefaultVM::Get(), -1).value);
            }
            // Advance to the next argument and obtain its flags
            arg_flags = ctx.mInstance->m_ArgSpec[++ctx.mArgc];
        }
        // Is there anything left to parse?
        if (itr >= ctx.mArgument.end())
        {
            break;
        }
        // Advance to the next character
        ++itr;
    }
    // Return whether the parsing was successful
    return good;
}

// ------------------------------------------------------------------------------------------------
Object & CmdManager::Create(CSStr name)
{
    return Attach(name, new CmdListener(name), true);
}

Object & CmdManager::Create(CSStr name, CSStr spec)
{
    return Attach(name, new CmdListener(name, spec), true);
}

Object & CmdManager::Create(CSStr name, CSStr spec, Array & tags)
{
    return Attach(name, new CmdListener(name, spec, tags), true);
}

Object & CmdManager::Create(CSStr name, CSStr spec, Uint8 min, Uint8 max)
{
    return Attach(name, new CmdListener(name, spec, min, max), true);
}

Object & CmdManager::Create(CSStr name, CSStr spec, Array & tags, Uint8 min, Uint8 max)
{
    return Attach(name, new CmdListener(name, spec, tags, min, max), true);
}

// ------------------------------------------------------------------------------------------------
void CmdListener::Init(CSStr name, CSStr spec, Array & tags, Uint8 min, Uint8 max)
{
    m_Name.assign("");
    // Initialize the specifiers and tags to default values
    for (Uint8 n = 0; n < SQMOD_MAX_CMD_ARGS; ++n)
    {
        m_ArgSpec[n] = CMDARG_ANY;
        m_ArgTags[n].assign("");
    }
    // Default to minimum and maximum arguments
    m_MinArgc = 0;
    m_MaxArgc = (SQMOD_MAX_CMD_ARGS - 1);
    // Default to no specifiers, help or informational message
    m_Spec.assign("");
    m_Help.assign("");
    m_Info.assign("");
    // Default to no authority check
    m_Authority = -1;
    // Default to unprotected command
    m_Protected = false;
    // Default to unsuspended command
    m_Suspended = false;
    // Default to non-associative arguments
    m_Associate = false;
    // Set the specified minimum and maximum allowed arguments
    SetMinArgC(min);
    SetMaxArgC(max);
    // Extract the specified argument tags
    SetArgTags(tags);
    // Bind to the specified command name
    SetName(name);
    // Apply the specified argument rules
    SetSpec(spec);
}

// ------------------------------------------------------------------------------------------------
CmdListener::CmdListener(CSStr name)
{
    Init(name, _SC(""), NullArray(), 0, SQMOD_MAX_CMD_ARGS-1);
}

CmdListener::CmdListener(CSStr name, CSStr spec)
{
    Init(name, spec, NullArray(), 0, SQMOD_MAX_CMD_ARGS-1);
}

CmdListener::CmdListener(CSStr name, CSStr spec, Array & tags)
{
    Init(name, spec, tags, 0, SQMOD_MAX_CMD_ARGS-1);
}

CmdListener::CmdListener(CSStr name, CSStr spec, Uint8 min, Uint8 max)
{
    Init(name, spec, NullArray(), min, max);
}

CmdListener::CmdListener(CSStr name, CSStr spec, Array & tags, Uint8 min, Uint8 max)
{
    Init(name, spec, tags, min, max);
}

// ------------------------------------------------------------------------------------------------
CmdListener::~CmdListener()
{
    // Detach this command (shouldn't be necessary!)
    CmdManager::Get().Detach(this);
    // Release callbacks
    m_OnExec.ReleaseGently();
    m_OnAuth.ReleaseGently();
    m_OnPost.ReleaseGently();
    m_OnFail.ReleaseGently();
}

// ------------------------------------------------------------------------------------------------
Int32 CmdListener::Cmp(const CmdListener & o) const
{
    if (m_Name == o.m_Name)
    {
        return 0;
    }
    else if (m_Name.size() > o.m_Name.size())
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::ToString() const
{
    return m_Name.c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::Attach()
{
    // Is the associated name even valid?
    if (m_Name.empty())
    {
        STHROWF("Invalid or empty command name");
    }
    // Are we already attached?
    else if (CmdManager::Get().Attached(this))
    {
        STHROWF("Command is already attached");
    }
    // Attempt to attach this command
    CmdManager::Get().Attach(m_Name, this, false);
}

// ------------------------------------------------------------------------------------------------
void CmdListener::Detach()
{
    // Detach this command
    CmdManager::Get().Detach(this);
}

// ------------------------------------------------------------------------------------------------
Uint8 CmdListener::GetArgFlags(Uint32 idx) const
{
    if (idx < SQMOD_MAX_CMD_ARGS)
        return m_ArgSpec[idx];
    return CMDARG_ANY;
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::GetName() const
{
    return m_Name.c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetName(CSStr name)
{
    // Validate the specified name
    ValidateName(name);
    // Is this command already attached to a name?
    if (CmdManager::Get().Attached(this))
    {
        // Detach from the current name if necessary
        CmdManager::Get().Detach(this);
        // Now it's safe to assign the new name
        m_Name.assign(name);
        // We know the new name is valid
        CmdManager::Get().Attach(m_Name, this, false);
    }
    else
    {
        // Just assign the name
        m_Name.assign(name);
    }
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::GetSpec() const
{
    return m_Spec.c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetSpec(CSStr spec)
{
    // Attempt to process the specified string
    ProcSpec(spec);
    // At this point there were no errors
    m_Spec.assign(spec);
}

// ------------------------------------------------------------------------------------------------
Array CmdListener::GetArgTags() const
{
    // Allocate an array to encapsulate all tags
    Array arr(DefaultVM::Get(), SQMOD_MAX_CMD_ARGS);
    // Put the tags to the allocated array
    for (Uint32 arg = 0; arg < SQMOD_MAX_CMD_ARGS; ++arg)
    {
        arr.SetValue(arg, m_ArgTags[arg]);
    }
    // Return the array with the tags
    return arr;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetArgTags(Array & tags)
{
    // Attempt to retrieve the number of specified tags
    const Uint32 max = static_cast< Uint32 >(tags.Length());
    // If no tags were specified then clear current tags
    if (tags.IsNull() || max == 0)
    {
        for (Uint8 n = 0; n < SQMOD_MAX_CMD_ARGS; ++n)
        {
            m_ArgTags[n].assign("");
        }
    }
    // See if we're in range
    else if (max < SQMOD_MAX_CMD_ARGS)
    {
        // Attempt to get all arguments in one go
        tags.GetArray(m_ArgTags, max);
    }
    else
    {
        STHROWF("Argument tag (%u) is out of range (%u)", max, SQMOD_MAX_CMD_ARGS);
    }
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::Attached() const
{
    return CmdManager::Get().Attached(this);
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::GetHelp() const
{
    return m_Help.c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetHelp(CSStr help)
{
    m_Help.assign(help);
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::GetInfo() const
{
    return m_Info.c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetInfo(CSStr info)
{
    m_Info.assign(info);
}

// ------------------------------------------------------------------------------------------------
Int32 CmdListener::GetAuthority() const
{
    return m_Authority;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetAuthority(Int32 level)
{
    m_Authority = level;
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::GetProtected() const
{
    return m_Protected;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetProtected(bool toggle)
{
    m_Protected = toggle;
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::GetSuspended() const
{
    return m_Suspended;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetSuspended(bool toggle)
{
    m_Suspended = toggle;
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::GetAssociate() const
{
    return m_Associate;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetAssociate(bool toggle)
{
    m_Associate = toggle;
}

// ------------------------------------------------------------------------------------------------
Uint8 CmdListener::GetMinArgC() const
{
    return m_MinArgc;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetMinArgC(Uint8 val)
{
    // Perform a range check on the specified argument index
    if (val >= SQMOD_MAX_CMD_ARGS)
    {
        STHROWF("Argument (%u) is out of total range (%u)", val, SQMOD_MAX_CMD_ARGS);
    }
    else if (val > m_MaxArgc)
    {
        STHROWF("Minimum argument (%u) exceeds maximum (%u)", val, m_MaxArgc);
    }
    // Apply the specified value
    m_MinArgc = val;
}

// ------------------------------------------------------------------------------------------------
Uint8 CmdListener::GetMaxArgC() const
{
    return m_MaxArgc;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetMaxArgC(Uint8 val)
{
    // Perform a range check on the specified argument index
    if (val >= SQMOD_MAX_CMD_ARGS)
    {
        STHROWF("Argument (%u) is out of total range (%u)", val, SQMOD_MAX_CMD_ARGS);
    }
    else if (val < m_MinArgc)
    {
        STHROWF("Minimum argument (%u) exceeds maximum (%u)", m_MinArgc, val);
    }
    // Apply the specified value
    m_MaxArgc = val;
}

// ------------------------------------------------------------------------------------------------
Function & CmdListener::GetOnExec()
{
    return m_OnExec;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetOnExec(Object & env, Function & func)
{
    // Make sure that we are allowed to store script resources
    if (m_Name.empty())
    {
        STHROWF("Invalid commands cannot store script resources");
    }
    // Apply the specified information
    m_OnExec = Function(env.GetVM(), env.GetObject(), func.GetFunc());
}

// ------------------------------------------------------------------------------------------------
Function & CmdListener::GetOnAuth()
{
    return m_OnAuth;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetOnAuth(Object & env, Function & func)
{
    // Make sure that we are allowed to store script resources
    if (m_Name.empty())
    {
        STHROWF("Invalid commands cannot store script resources");
    }
    // Apply the specified information
    m_OnAuth = Function(env.GetVM(), env.GetObject(), func.GetFunc());
}

// ------------------------------------------------------------------------------------------------
Function & CmdListener::GetOnPost()
{
    return m_OnPost;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetOnPost(Object & env, Function & func)
{
    // Make sure that we are allowed to store script resources
    if (m_Name.empty())
    {
        STHROWF("Invalid commands cannot store script resources");
    }
    // Apply the specified information
    m_OnPost = Function(env.GetVM(), env.GetObject(), func.GetFunc());
}

// ------------------------------------------------------------------------------------------------
Function & CmdListener::GetOnFail()
{
    return m_OnFail;
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetOnFail(Object & env, Function & func)
{
    // Make sure that we are allowed to store script resources
    if (m_Name.empty())
    {
        STHROWF("Invalid commands cannot store script resources");
    }
    // Apply the specified information
    m_OnFail = Function(env.GetVM(), env.GetObject(), func.GetFunc());
}

// ------------------------------------------------------------------------------------------------
CSStr CmdListener::GetArgTag(Uint32 arg) const
{
    // Perform a range check on the specified argument index
    if (arg >= SQMOD_MAX_CMD_ARGS)
    {
        STHROWF("Argument (%u) is out of total range (%u)", arg, SQMOD_MAX_CMD_ARGS);
    }
    // Return the requested information
    return m_ArgTags[arg].c_str();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::SetArgTag(Uint32 arg, CSStr name)
{
    // Perform a range check on the specified argument index
    if (arg >= SQMOD_MAX_CMD_ARGS)
    {
        STHROWF("Argument (%u) is out of total range (%u)", arg, SQMOD_MAX_CMD_ARGS);
    }
    // The string type doesn't appreciate null values
    else if (name)
    {
        m_ArgTags[arg].assign(name);
    }
    // Clear previous name in this case
    else
        m_ArgTags[arg].clear();
}

// ------------------------------------------------------------------------------------------------
void CmdListener::GenerateInfo(bool full)
{
    // Clear any previously generated informational message
    m_Info.clear();
    // Process each supported command argument
    for (Uint32 arg = 0; arg < m_MaxArgc; ++arg)
    {
        // If this is not a full command request then see if we must stop
        if (!full)
        {
            // Default to stop if criteria are not meet
            bool stop = true;
            // Check all arguments after this and see if there's any left
            for (Uint32 idx = arg; idx < m_MaxArgc; ++idx)
            {
                // If the argument has a name or a type specifier then it's valid
                if (!m_ArgTags[idx].empty() || m_ArgSpec[idx] != CMDARG_ANY)
                {
                    // We have more arguments that need to be parsed
                    stop = false;
                    // Go back to the main loop
                    break;
                }
            }
            // Is there any argument left?
            if (stop)
            {
                break; // Stop the main loop as well
            }
        }
        // Begin the argument block
        m_Info.push_back('<');
        // If the current argument is beyond minimum then mark it as optional
        if (arg >= m_MinArgc)
        {
            m_Info.push_back('*');
        }
        // If the argument has a tag/name associated then add it as well
        if (!m_ArgTags[arg].empty())
        {
            // Add the name first
            m_Info.append(m_ArgTags[arg]);
            // Separate the name from the specifiers
            m_Info.push_back(':');
        }
        // Obtain the argument specifier
        const Uint8 spec = m_ArgSpec[arg];
        // Is this a greedy argument?
        if (spec & CMDARG_GREEDY)
        {
            m_Info.append("...");
        }
        // If the argument has any explicit types specified
        else if (spec != CMDARG_ANY)
        {
            // Does it support integers?
            if (spec & CMDARG_INTEGER)
            {
                m_Info.append("integer");
            }
            // Does it support floats?
            if (spec & CMDARG_FLOAT)
            {
                // Add a separator if this is not the first enabled type!
                if (m_Info.back() != ':' && m_Info.back() != '<')
                {
                    m_Info.push_back(',');
                }
                // Now add the type name
                m_Info.append("float");
            }
            // Does it support booleans?
            if (spec & CMDARG_BOOLEAN)
            {
                // Add a separator if this is not the first enabled type!
                if (m_Info.back() != ':' && m_Info.back() != '<')
                {
                    m_Info.push_back(',');
                }
                // Now add the type name
                m_Info.append("boolean");
            }
            // Does it support strings?
            if (spec & CMDARG_STRING)
            {
                // Add a separator if this is not the first enabled type?
                if (m_Info.back() != ':' && m_Info.back() != '<')
                {
                    m_Info.push_back(',');
                }
                // Now add the type name
                m_Info.append("string");
            }
        }
        // Any kind of value is supported by this argument
        else
        {
            m_Info.append("any");
        }
        // Terminate the argument block
        m_Info.push_back('>');
        // Don't process anything after greedy arguments
        if (spec & CMDARG_GREEDY)
        {
            break;
        }
        // If this is not the last argument then add a separator
        else if (arg+1 != m_MaxArgc)
        {
            m_Info.push_back(' ');
        }
    }
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::ArgCheck(Uint32 arg, Uint8 flag) const
{
    // Perform a range check on the specified argument index
    if (arg >= SQMOD_MAX_CMD_ARGS)
    {
        STHROWF("Argument (%u) is out of total range (%u)", arg, SQMOD_MAX_CMD_ARGS);
    }
    // Retrieve the argument flags
    const Uint8 f = m_ArgSpec[arg];
    // Perform the requested check
    return  (f == CMDARG_ANY) || /* Requires check? */
            (f & flag) || /* Exact match? */
            (f & CMDARG_GREEDY && flag & CMDARG_STRING);
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::AuthCheck(CPlayer & player)
{
    return AuthCheckID(player.GetID());
}

// ------------------------------------------------------------------------------------------------
bool CmdListener::AuthCheckID(Int32 id)
{
    // Do we need explicit authority verification?
    if (!m_Protected)
    {
        return true;
    }
    // Allow execution by default
    bool allow = true;
    // Was there a custom authority inspector specified?
    if (!m_OnAuth.IsNull())
    {
        // Ask the specified authority inspector if this execution should be allowed
        SharedPtr< bool > ret = m_OnAuth.Evaluate< bool, Object & >(Core::Get().GetPlayer(id).mObj);
        // See what the custom authority inspector said or default to disallow
        allow = (!ret ? false : *ret);
    }
    // Was there a global authority inspector specified?
    else if (!CmdManager::Get().GetOnAuth().IsNull())
    {
        // Ask the specified authority inspector if this execution should be allowed
        SharedPtr< bool > ret = CmdManager::Get().GetOnAuth().Evaluate< bool, Object & >(Core::Get().GetPlayer(id).mObj);
        // See what the custom authority inspector said or default to disallow
        allow = (!ret ? false : *ret);
    }
    // Can we use the default authority system?
    else if (m_Authority >= 0)
    {
        allow = (Core::Get().GetPlayer(id).mAuthority >= m_Authority);
    }
    // Return result
    return allow;
}

// ------------------------------------------------------------------------------------------------
SQInteger CmdListener::Execute(Object & invoker, Array & args)
{
    // Attempt to evaluate the specified executer knowing the manager did the validations
    SharedPtr< SQInteger > ret = m_OnExec.Evaluate< SQInteger, Object &, Array & >(invoker, args);
    // See if the executer succeeded and return the result or default to failed
    return (!ret ? 0 : *ret);
}

// ------------------------------------------------------------------------------------------------
SQInteger CmdListener::Execute(Object & invoker, Table & args)
{
    // Attempt to evaluate the specified executer knowing the manager did the validations
    SharedPtr< SQInteger > ret = m_OnExec.Evaluate< SQInteger, Object &, Table & >(invoker, args);
    // See if the executer succeeded and return the result or default to failed
    return (!ret ? 0 : *ret);
}

// ------------------------------------------------------------------------------------------------
void CmdListener::ProcSpec(CSStr str)
{
    // Reset current argument specifiers
    memset(m_ArgSpec, CMDARG_ANY, sizeof(m_ArgSpec));
    // Make sure we have anything to parse
    if (!str || *str == '\0')
    {
        return;
    }
    // Currently processed argument
    Uint32 idx = 0;
    // Try to apply the specified type specifiers
    try
    {
        // Process until null terminator or an error occurs
        while (*str != 0)
        {
            // See if we need to move to the next argument
            if (*str == '|')
            {
                if (idx >= SQMOD_MAX_CMD_ARGS)
                {
                    STHROWF("Extraneous type specifiers: %d >= %d", idx, SQMOD_MAX_CMD_ARGS);
                }
                // Move to the next character
                ++str;
                // Advance to the next argument
                ++idx;
            }
            // Simply ignore a type specifier delimiter
            else if (*str != ',')
            {
                // Ignore non-alphabetic characters
                while (*str != 0 &&  !isalpha(*str))
                {
                    ++str;
                }
                // Apply the type specifier
                switch(*str++)
                {
                    // Did we reached the end of the string?
                    case '\0':
                        break;
                    // Is this a greedy argument?
                    case 'g':
                    {
                        m_ArgSpec[idx] = CMDARG_GREEDY;
                    } break;
                    // Is this a integer type
                    case 'i':
                    {
                        m_ArgSpec[idx] |= CMDARG_INTEGER;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Is this a float type
                    case 'f':
                    {
                        m_ArgSpec[idx] |= CMDARG_FLOAT;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Is this a boolean type
                    case 'b':
                    {
                        m_ArgSpec[idx] |= CMDARG_BOOLEAN;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Is this a string type
                    case 's':
                    {
                        m_ArgSpec[idx] |= CMDARG_STRING;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Is this a lowercase string?
                    case 'l':
                    {
                        m_ArgSpec[idx] |= CMDARG_STRING;
                        m_ArgSpec[idx] |= CMDARG_LOWER;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Is this a uppercase string?
                    case 'u':
                    {
                        m_ArgSpec[idx] |= CMDARG_STRING;
                        m_ArgSpec[idx] |= CMDARG_UPPER;
                        // Disable greedy argument flag if set
                        if (m_ArgSpec[idx] & CMDARG_GREEDY)
                        {
                            m_ArgSpec[idx] ^= CMDARG_GREEDY;
                        }
                    } break;
                    // Unknown type!
                    default: STHROWF("Unknown type specifier (%c) at argument: %u", *str, idx);
                }
            }
        }
    }
    catch (const Sqrat::Exception & e)
    {
        // Reset all argument specifiers if failed
        memset(m_ArgSpec, CMDARG_ANY, sizeof(m_ArgSpec));
        // Propagate the exception back to the caller
        throw e;
    }
    // Attempt to generate an informational message
    GenerateInfo(false);
}

/* ------------------------------------------------------------------------------------------------
 * Forward the call to run a command.
*/
static Int32 Cmd_Run(Int32 invoker, CSStr command)
{
    return CmdManager::Get().Run(invoker, command);
}

// ------------------------------------------------------------------------------------------------
static void Cmd_Sort()
{
    CmdManager::Get().Sort();
}

// ------------------------------------------------------------------------------------------------
static Uint32 Cmd_Count()
{
    return CmdManager::Get().Count();
}

// ------------------------------------------------------------------------------------------------
static const Object & Cmd_FindByName(CSStr name)
{
    // Validate the specified name
    ValidateName(name);
    // Now perform the requested search
    return CmdManager::Get().FindByName(name);
}

// ------------------------------------------------------------------------------------------------
static Function & Cmd_GetOnFail()
{
    return CmdManager::Get().GetOnFail();
}

// ------------------------------------------------------------------------------------------------
static void Cmd_SetOnFail(Object & env, Function & func)
{
    CmdManager::Get().SetOnFail(env, func);
}

// ------------------------------------------------------------------------------------------------
static Function & Cmd_GetOnAuth()
{
    return CmdManager::Get().GetOnAuth();
}

// ------------------------------------------------------------------------------------------------
static void Cmd_SetOnAuth(Object & env, Function & func)
{
    CmdManager::Get().SetOnAuth(env, func);
}

// ------------------------------------------------------------------------------------------------
static bool Cmd_IsContext()
{
    return CmdManager::Get().IsContext();
}

// ------------------------------------------------------------------------------------------------
static Object & Cmd_GetInvoker()
{
    return Core::Get().GetPlayer(CmdManager::Get().GetInvoker()).mObj;
}

// ------------------------------------------------------------------------------------------------
static Int32 Cmd_GetInvokerID()
{
    return CmdManager::Get().GetInvoker();
}

// ------------------------------------------------------------------------------------------------
static const Object & Cmd_GetObject()
{
    return CmdManager::Get().GetObject();
}

// ------------------------------------------------------------------------------------------------
static const String & Cmd_GetCommand()
{
    return CmdManager::Get().GetCommand();
}

// ------------------------------------------------------------------------------------------------
static const String & Cmd_GetArgument()
{
    return CmdManager::Get().GetArgument();
}

// ------------------------------------------------------------------------------------------------
Object & Cmd_Create(CSStr name)
{
    return CmdManager::Get().Create(name);
}

Object & Cmd_Create(CSStr name, CSStr spec)
{
    return CmdManager::Get().Create(name, spec);
}

Object & Cmd_Create(CSStr name, CSStr spec, Array & tags)
{
    return CmdManager::Get().Create(name, spec, tags);
}

Object & Cmd_Create(CSStr name, CSStr spec, Uint8 min, Uint8 max)
{
    return CmdManager::Get().Create(name,spec, min, max);
}

Object & Cmd_Create(CSStr name, CSStr spec, Array & tags, Uint8 min, Uint8 max)
{
    return CmdManager::Get().Create(name, spec, tags, min, max);
}

// ================================================================================================
void Register_Command(HSQUIRRELVM vm)
{
    Table cmdns(vm);

    cmdns.Bind(_SC("Listener"), Class< CmdListener, NoConstructor< CmdListener > >(vm, _SC("SqCmdListener"))
        // Meta-methods
        .Func(_SC("_cmp"), &CmdListener::Cmp)
        .SquirrelFunc(_SC("_typename"), &CmdListener::Typename)
        .Func(_SC("_tostring"), &CmdListener::ToString)
        // Member Properties
        .Prop(_SC("Attached"), &CmdListener::Attached)
        .Prop(_SC("Name"), &CmdListener::GetName, &CmdListener::SetName)
        .Prop(_SC("Spec"), &CmdListener::GetSpec, &CmdListener::SetSpec)
        .Prop(_SC("Specifier"), &CmdListener::GetSpec, &CmdListener::SetSpec)
        .Prop(_SC("Tags"), &CmdListener::GetArgTags, &CmdListener::SetArgTags)
        .Prop(_SC("Help"), &CmdListener::GetHelp, &CmdListener::SetHelp)
        .Prop(_SC("Info"), &CmdListener::GetInfo, &CmdListener::SetInfo)
        .Prop(_SC("Authority"), &CmdListener::GetAuthority, &CmdListener::SetAuthority)
        .Prop(_SC("Protected"), &CmdListener::GetProtected, &CmdListener::SetProtected)
        .Prop(_SC("Suspended"), &CmdListener::GetSuspended, &CmdListener::SetSuspended)
        .Prop(_SC("Associate"), &CmdListener::GetAssociate, &CmdListener::SetAssociate)
        .Prop(_SC("MinArgs"), &CmdListener::GetMinArgC, &CmdListener::SetMinArgC)
        .Prop(_SC("MaxArgs"), &CmdListener::GetMaxArgC, &CmdListener::SetMaxArgC)
        .Prop(_SC("OnExec"), &CmdListener::GetOnExec)
        .Prop(_SC("OnAuth"), &CmdListener::GetOnAuth)
        .Prop(_SC("OnPost"), &CmdListener::GetOnPost)
        .Prop(_SC("OnFail"), &CmdListener::GetOnFail)
        // Member Methods
        .Func(_SC("Attach"), &CmdListener::Attach)
        .Func(_SC("Detach"), &CmdListener::Detach)
        .Func(_SC("BindExec"), &CmdListener::SetOnExec)
        .Func(_SC("BindAuth"), &CmdListener::SetOnAuth)
        .Func(_SC("BindPost"), &CmdListener::SetOnPost)
        .Func(_SC("BindFail"), &CmdListener::SetOnFail)
        .Func(_SC("GetArgTag"), &CmdListener::GetArgTag)
        .Func(_SC("SetArgTag"), &CmdListener::SetArgTag)
        .Func(_SC("GenerateInfo"), &CmdListener::GenerateInfo)
        .Func(_SC("ArgCheck"), &CmdListener::ArgCheck)
        .Func(_SC("AuthCheck"), &CmdListener::AuthCheck)
        .Func(_SC("AuthCheckID"), &CmdListener::AuthCheckID)
    );

    cmdns.Func(_SC("Run"), &Cmd_Run);
    cmdns.Func(_SC("Sort"), &Cmd_Sort);
    cmdns.Func(_SC("Count"), &Cmd_Count);
    cmdns.Func(_SC("FindByName"), &Cmd_FindByName);
    cmdns.Func(_SC("GetOnFail"), &Cmd_GetOnFail);
    cmdns.Func(_SC("SetOnFail"), &Cmd_SetOnFail);
    cmdns.Func(_SC("BindFail"), &Cmd_SetOnFail);
    cmdns.Func(_SC("GetOnAuth"), &Cmd_GetOnAuth);
    cmdns.Func(_SC("SetOnAuth"), &Cmd_SetOnAuth);
    cmdns.Func(_SC("BindAuth"), &Cmd_SetOnAuth);
    cmdns.Func(_SC("Context"), &Cmd_IsContext);
    cmdns.Func(_SC("Invoker"), &Cmd_GetInvoker);
    cmdns.Func(_SC("InvokerID"), &Cmd_GetInvokerID);
    cmdns.Func(_SC("Instance"), &Cmd_GetObject);
    cmdns.Func(_SC("Name"), &Cmd_GetCommand);
    cmdns.Func(_SC("Command"), &Cmd_GetCommand);
    cmdns.Func(_SC("Text"), &Cmd_GetArgument);
    cmdns.Func(_SC("Argument"), &Cmd_GetArgument);
    cmdns.Overload< Object & (CSStr) >(_SC("Create"), &Cmd_Create);
    cmdns.Overload< Object & (CSStr, CSStr) >(_SC("Create"), &Cmd_Create);
    cmdns.Overload< Object & (CSStr, CSStr, Array &) >(_SC("Create"), &Cmd_Create);
    cmdns.Overload< Object & (CSStr, CSStr, Uint8, Uint8) >(_SC("Create"), &Cmd_Create);
    cmdns.Overload< Object & (CSStr, CSStr, Array &, Uint8, Uint8) >(_SC("Create"), &Cmd_Create);

    RootTable(vm).Bind(_SC("SqCmd"), cmdns);

    ConstTable(vm).Enum(_SC("CmdArg"), Enumeration(vm)
        .Const(_SC("Any"),                  CMDARG_ANY)
        .Const(_SC("Integer"),              CMDARG_INTEGER)
        .Const(_SC("Float"),                CMDARG_FLOAT)
        .Const(_SC("Boolean"),              CMDARG_BOOLEAN)
        .Const(_SC("String"),               CMDARG_STRING)
        .Const(_SC("Lower"),                CMDARG_LOWER)
        .Const(_SC("Upper"),                CMDARG_UPPER)
        .Const(_SC("Greedy"),               CMDARG_GREEDY)
    );

    ConstTable(vm).Enum(_SC("CmdErr"), Enumeration(vm)
        .Const(_SC("Unknown"),              CMDERR_UNKNOWN)
        .Const(_SC("EmptyCommand"),         CMDERR_EMPTY_COMMAND)
        .Const(_SC("InvalidCommand"),       CMDERR_INVALID_COMMAND)
        .Const(_SC("SyntaxError"),          CMDERR_SYNTAX_ERROR)
        .Const(_SC("UnknownCommand"),       CMDERR_UNKNOWN_COMMAND)
        .Const(_SC("MissingExecuter"),      CMDERR_MISSING_EXECUTER)
        .Const(_SC("InsufficientAuth"),     CMDERR_INSUFFICIENT_AUTH)
        .Const(_SC("IncompleteArgs"),       CMDERR_INCOMPLETE_ARGS)
        .Const(_SC("ExtraneousArgs"),       CMDERR_EXTRANEOUS_ARGS)
        .Const(_SC("UnsupportedArg"),       CMDERR_UNSUPPORTED_ARG)
        .Const(_SC("BufferOverflow"),       CMDERR_BUFFER_OVERFLOW)
        .Const(_SC("ExecutionFailed"),      CMDERR_EXECUTION_FAILED)
        .Const(_SC("ExecutionAborted"),     CMDERR_EXECUTION_ABORTED)
        .Const(_SC("PostProcessingFailed"), CMDERR_POST_PROCESSING_FAILED)
        .Const(_SC("UnresolvedFailure"),    CMDERR_UNRESOLVED_FAILURE)
        .Const(_SC("Max"),                  CMDERR_MAX)
    );
}

} // Namespace:: SqMod
