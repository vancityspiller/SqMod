#pragma once

// ------------------------------------------------------------------------------------------------
#include "Core/Utility.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
struct RoutineBuilder;

/* ------------------------------------------------------------------------------------------------
 * Execute callbacks after specific intervals of time.
*/
class Routine
{
    friend struct RoutineBuilder;
public:

    /* --------------------------------------------------------------------------------------------
     * Simplify future changes to a single point of change.
    */
    typedef int64_t                                     Time;
    typedef SQInteger                                   Interval;
    typedef uint32_t                                    Iterator;
    typedef LightObj                                    Argument;

private:

    /* --------------------------------------------------------------------------------------------
     * Structure that represents an active routine and keeps track of the routine information.
    */
    struct Instance
    {
        // ----------------------------------------------------------------------------------------
        LightObj    mEnv; // A reference to the managed environment object.
        LightObj    mFunc; // A reference to the managed function object.
        LightObj    mInst; // Reference to the routine associated with this instance.
        LightObj    mData; // A reference to the arbitrary data associated with this instance.
        String      mTag; // An arbitrary string which represents the tag.
        Iterator    mIterations; // Number of iterations before self destruct.
        Interval    mInterval; // Interval between routine invocations.
        bool        mSuspended; // Whether this instance is allowed to receive calls.
        bool        mQuiet; // Whether this instance is allowed to handle errors.
        bool        mEndure; // Whether this instance is allowed to terminate itself on errors.
        bool        mExecuting; // Whether this instance is currently being executed.
        uint8_t     mArgc; // The number of arguments that the routine must forward.
        Argument    mArgv[14]; // The arguments that the routine must forward.

        /* ----------------------------------------------------------------------------------------
         * Default constructor.
        */
        Instance() noexcept
            : mEnv()
            , mFunc()
            , mInst()
            , mData()
            , mTag()
            , mIterations(0)
            , mInterval(0)
            , mSuspended(false)
            , mQuiet(GetSilenced())
            , mEndure(false)
            , mExecuting(false)
            , mArgc(0)
            , mArgv()
        {
            /* ... */
        }

        /* ----------------------------------------------------------------------------------------
         * Copy constructor. (disabled)
        */
        Instance(const Instance & o) = delete;

        /* ----------------------------------------------------------------------------------------
         * Move constructor. (disabled)
        */
        Instance(Instance && o) = delete;

        /* ----------------------------------------------------------------------------------------
         * Destructor.
        */
        ~Instance()
        {
            Terminate();
        }

        /* ----------------------------------------------------------------------------------------
         * Copy assignment operator. (disabled)
        */
        Instance & operator = (const Instance & o) = delete;

        /* ----------------------------------------------------------------------------------------
         * Move assignment operator. (disabled)
        */
        Instance & operator = (Instance && o) = delete;

        /* ----------------------------------------------------------------------------------------
         * Initializes the routine parameters. (assumes previous values are already released)
        */
        void Init(HSQOBJECT & env, HSQOBJECT & func, HSQOBJECT & inst, Interval intrv, Iterator itr)
        {
            // Initialize the callback objects
            mEnv = LightObj{env};
            mFunc = LightObj{func};
            // Associate with the routine instance
            mInst = LightObj{inst};
            // Initialize the routine options
            mIterations = itr;
            mInterval = intrv;
            // This can't be true now
            mExecuting = false;
        }

        /* ----------------------------------------------------------------------------------------
         * Release managed script resources.
        */
        void Release()
        {
            mEnv.Release();
            mFunc.Release();
            mInst.Release();
            mData.Release();
            mIterations = 0;
            mInterval = 0;
            mTag.clear();
        }

        /* ----------------------------------------------------------------------------------------
         * Execute the managed routine.
        */
        Interval Execute()
        {
            // Is this even a valid routine?
            if (mInst.IsNull())
            {
                return 0; // Dunno how we got here but it ends now
            }
            // Are we allowed to forward calls?
            else if (!mSuspended)
            {
                // Grab the virtual machine once
                HSQUIRRELVM vm = SqVM();
                // Push the function on the stack
                sq_pushobject(vm, mFunc);
                // Push the environment on the stack
                if (!mEnv.IsNull())
                {
                    sq_pushobject(vm, mEnv); // Push object
                }
                else
                {
                    sq_pushobject(vm, mInst); // Push self
                }
                // Push function parameters, if any
                for (uint32_t n = 0; n < mArgc; ++n)
                {
                    sq_pushobject(vm, mArgv[n].mObj);
                }
                // This routine is currently executing
                mExecuting = true;
                // Make the function call and store the result
                const SQRESULT res = sq_call(vm, mArgc + 1, static_cast< SQBool >(false), static_cast< SQBool >(!mQuiet));
                // This routine has finished executing
                mExecuting = false;
                // Pop the callback object from the stack
                sq_pop(vm, 1);
                // Validate the result
                if (SQ_FAILED(res))
                {
                    // Should we endure the errors?
                    if (!mEndure)
                    {
                        Terminate(); // Destroy our self on error
                    }
                }
            }
            // Decrease the number of iterations if necessary
            if (mIterations && (--mIterations) == 0)
            {
                Terminate(); // This routine reached the end of it's life
            }
            // Return the current interval
            return mInterval;
        }

        /* ----------------------------------------------------------------------------------------
         * Clear the arguments.
        */
        void Clear()
        {
            // Now release the arguments
            for (auto & a : mArgv)
            {
                a.Release();
            }
            // Reset the counter
            mArgc = 0;
        }

        /* ----------------------------------------------------------------------------------------
         * Terminate the routine.
        */
        void Terminate()
        {
            Release();
            Clear();
        }
    };

private:

    // --------------------------------------------------------------------------------------------
    static Time         s_Last; // Last time point.
    static Time         s_Prev; // Previous time point.
    static Interval     s_Intervals[SQMOD_MAX_ROUTINES]; // List of intervals to be processed.
    static Instance     s_Instances[SQMOD_MAX_ROUTINES]; // List of routines to be executed.
    static bool         s_Silenced; // Error reporting independent from global setting.

private:

    /* --------------------------------------------------------------------------------------------
     * The index of the slot in the pool of active routines.
    */
    uint32_t   m_Slot;

protected:

    /* --------------------------------------------------------------------------------------------
     * Default constructor.
    */
    Routine()
        : m_Slot(SQMOD_MAX_ROUTINES)
    {
        /* ... */
    }

    /* --------------------------------------------------------------------------------------------
     * Default constructor.
    */
    explicit Routine(uint32_t slot)
        : m_Slot(slot)
    {
        /* ... */
    }

    /* --------------------------------------------------------------------------------------------
     * Find an unoccupied routine slot.
    */
    static SQInteger FindUnused()
    {
        for (const auto & r : s_Instances)
        {
            // Either not used or not currently being executing
            if (r.mInst.IsNull() && !(r.mExecuting))
            {
                return (&r - s_Instances); // Return the index of this element
            }
        }
        // No available slot
        return -1;
    }

public:

    /* --------------------------------------------------------------------------------------------
     * Default constructor.
    */
    ~Routine()
    {
        if (m_Slot < SQMOD_MAX_ROUTINES)
        {
            Terminate();
        }
    }

    /* --------------------------------------------------------------------------------------------
     * Copy constructor. (disabled)
    */
    Routine(const Routine & o) = delete;

    /* --------------------------------------------------------------------------------------------
     * Move constructor. (disabled)
    */
    Routine(Routine && o) = delete;

    /* --------------------------------------------------------------------------------------------
     * Copy assignment operator. (disabled)
    */
    Routine & operator = (const Routine & o) = delete;

    /* --------------------------------------------------------------------------------------------
     * Move assignment operator. (disabled)
    */
    Routine & operator = (Routine && o) = delete;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the number of used routine slots.
    */
    static SQInteger GetUsed()
    {
        SQInteger n = 0;
        // Iterate routine list
        for (const auto & r : s_Instances)
        {
            if (!r.mInst.IsNull())
            {
                ++n;
            }
        }
        // Return the final count
        return n;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the number of used routine slots.
    */
    static const LightObj & FindByTag(StackStrF & tag)
    {
        // Is the specified tag valid?
        if (!tag.mPtr)
        {
            STHROWF("Invalid routine tag");
        }
        // Iterate routine list
        for (const auto & r : s_Instances)
        {
            if (!r.mInst.IsNull() && r.mTag == tag.mPtr)
            {
                return r.mInst; // Return this routine instance
            }
        }
        // Unable to find such routine
        STHROWF("Unable to find a routine with tag ({})", tag.mPtr);
        SQ_UNREACHABLE;
        // Should not reach this point but if it did, we have to return something
#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Warray-bounds"
#endif
        return s_Instances[SQMOD_MAX_ROUTINES].mInst; // Intentional Buffer overflow!
#ifdef __clang__
	#pragma clang diagnostic pop
#endif
    }
    /* --------------------------------------------------------------------------------------------
     * Check if a routine with a certain tag exists.
    */
    static bool IsWithTag(StackStrF & tag);
    /* --------------------------------------------------------------------------------------------
     * Check if a routine with a certain tag exists.
    */
    static bool TerminateWithTag(StackStrF & tag);
    /* --------------------------------------------------------------------------------------------
     * Process all active routines and update elapsed time.
    */
    static void Process();

    /* --------------------------------------------------------------------------------------------
     * Initialize all resources and prepare for startup.
    */
    static void Initialize();

    /* --------------------------------------------------------------------------------------------
     * Release all resources and prepare for shutdown.
    */
    static void Deinitialize();

    /* --------------------------------------------------------------------------------------------
     * Create a routine with the specified parameters.
    */
    static SQInteger Create(HSQUIRRELVM vm);
#ifdef VCMP_ENABLE_OFFICIAL
    /* --------------------------------------------------------------------------------------------
     * Create a routine with the specified parameters using the official compatibility layer.
    */
    static SQInteger CreateOfficial(HSQUIRRELVM vm);
#endif
protected:

    /* --------------------------------------------------------------------------------------------
     * See whether this routine is valid otherwise throw an exception.
    */
    void Validate() const
    {
        if (m_Slot >= SQMOD_MAX_ROUTINES)
        {
            STHROWF("This instance does not reference a valid routine");
        }
    }

    /* --------------------------------------------------------------------------------------------
     * See whether this routine is valid otherwise throw an exception.
    */
    SQMOD_NODISCARD Instance & GetValid() const
    {
        if (m_Slot >= SQMOD_MAX_ROUTINES)
        {
            STHROWF("This instance does not reference a valid routine");
        }
        // We know it's valid so let's return it
        return s_Instances[m_Slot];
    }

public:

    /* --------------------------------------------------------------------------------------------
     * Used by the script engine to convert an instance of this type to a string.
    */
    SQMOD_NODISCARD const String & ToString() const
    {
        return (m_Slot >= SQMOD_MAX_ROUTINES) ? NullString() : s_Instances[m_Slot].mTag;
    }

    /* --------------------------------------------------------------------------------------------
     * Terminate the routine.
    */
    void Terminate()
    {
        GetValid().Terminate();
        s_Intervals[m_Slot] = 0;
        m_Slot = SQMOD_MAX_ROUTINES;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the associated user tag.
    */
    SQMOD_NODISCARD const String & GetTag() const
    {
        return GetValid().mTag;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the associated user tag.
    */
    void SetTag(StackStrF & tag)
    {
        GetValid().mTag.assign(tag.mPtr, static_cast< size_t >(ClampMin(tag.mLen, 0)));
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the associated user tag.
    */
    Routine & ApplyTag(StackStrF & tag)
    {
        SetTag(tag);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the environment object.
    */
    SQMOD_NODISCARD const LightObj & GetEnv() const
    {
        return GetValid().mEnv;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the environment object.
    */
    void SetEnv(const LightObj & env)
    {
        GetValid().mEnv = env.IsNull() ? LightObj(RootTable{}.GetObj()) : env;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the function object.
    */
    SQMOD_NODISCARD const LightObj & GetFunc() const
    {
        return GetValid().mFunc;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the function object.
    */
    void SetFunc(const Function & func)
    {
        // Validate the specified
        if (!sq_isclosure(func.GetFunc()) && !sq_isnativeclosure(func.GetFunc()))
        {
            STHROWF("Invalid callback type {}", SqTypeName(GetValid().mFunc.GetType()));
        }
        // Store the function without the environment
        GetValid().mFunc = LightObj(func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the arbitrary user data object.
    */
    SQMOD_NODISCARD const LightObj & GetData() const
    {
        return GetValid().mData;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the arbitrary user data object.
    */
    void SetData(const LightObj & data)
    {
        GetValid().mData = data;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the arbitrary user data object.
    */
    Routine & ApplyData(const LightObj & data)
    {
        SetData(data);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the execution interval.
    */
    SQMOD_NODISCARD SQInteger GetInterval() const
    {
        return ConvTo< SQInteger >::From(GetValid().mInterval);
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the execution interval.
    */
    void SetInterval(SQInteger itr)
    {
        GetValid().mInterval = ClampMin(ConvTo< Interval >::From(itr), static_cast< Interval >(0));
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the execution interval.
    */
    Routine & ApplyInterval(SQInteger itr)
    {
        SetInterval(itr);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the number of iterations.
    */
    SQMOD_NODISCARD SQInteger GetIterations() const
    {
        return ConvTo< SQInteger >::From(GetValid().mIterations);
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the number of iterations.
    */
    void SetIterations(SQInteger itr)
    {
        GetValid().mIterations = ConvTo< Iterator >::From(itr);
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the number of iterations.
    */
    Routine & ApplyIterations(SQInteger itr)
    {
        SetIterations(itr);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * See whether the routine is suspended.
    */
    SQMOD_NODISCARD bool GetSuspended() const
    {
        return GetValid().mSuspended;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should be suspended.
    */
    void SetSuspended(bool toggle)
    {
        GetValid().mSuspended = toggle;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should be suspended.
    */
    Routine & ApplySuspended(bool toggle)
    {
        SetSuspended(toggle);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * See whether the routine is quite.
    */
    SQMOD_NODISCARD bool GetQuiet() const
    {
        return GetValid().mQuiet;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should be quiet.
    */
    void SetQuiet(bool toggle)
    {
        GetValid().mQuiet = toggle;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should be quiet.
    */
    Routine & ApplyQuiet(bool toggle)
    {
        SetQuiet(toggle);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * See whether the routine endures.
    */
    SQMOD_NODISCARD bool GetEndure() const
    {
        return GetValid().mEndure;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should endure.
    */
    void SetEndure(bool toggle)
    {
        GetValid().mEndure = toggle;
    }

    /* --------------------------------------------------------------------------------------------
     * Set whether the routine should endure.
    */
    Routine & ApplyEndure(bool toggle)
    {
        SetEndure(toggle);
        return *this;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the number of arguments to be forwarded.
    */
    SQMOD_NODISCARD SQInteger GetArguments() const
    {
        return ConvTo< SQInteger >::From(GetValid().mArgc);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve a certain argument.
    */
    SQMOD_NODISCARD const Argument & GetArgument(SQInteger arg) const
    {
        // Cast the index to the proper value
        uint8_t idx = ConvTo< uint8_t >::From(arg);
        // Validate the specified index
        if (idx >= 14)
        {
            STHROWF("The specified index is out of range: {} >= {}", idx, 14);
        }
        // Return the requested argument
        return GetValid().mArgv[idx];
    }

    /* --------------------------------------------------------------------------------------------
     * Release the environment object and default to self.
    */
    void DropEnv()
    {
        GetValid().mEnv.Release();
    }

    /* --------------------------------------------------------------------------------------------
     * See if error reporting is enabled for all newly created routines.
    */
    static bool GetSilenced()
    {
        return s_Silenced;
    }

    /* --------------------------------------------------------------------------------------------
     * Set if error reporting should be enabled for all newly created routines.
    */
    static void SetSilenced(bool toggle)
    {
        s_Silenced = toggle;
    }
};

} // Namespace:: SqMod
