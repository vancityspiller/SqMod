#pragma once

// ------------------------------------------------------------------------------------------------
#include "Core/Utility.hpp"
#include "Base/Color4.hpp"
#include "Base/Vector3.hpp"
#include "Base/Quaternion.hpp"

// ------------------------------------------------------------------------------------------------
#include <vector>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// --------------------------------------------------------------------------------------------
typedef std::vector< std::pair< Area *, LightObj > > AreaList; // List of collided areas.

// --------------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    struct LgCheckpoint;
    struct LgObject;
    struct LgPickup;
    struct LgPlayer;
    struct LgVehicle;
#endif

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a blip entity instance on the server.
*/
struct BlipInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    BlipInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    BlipInst(const BlipInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    BlipInst(BlipInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~BlipInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    BlipInst & operator = (const BlipInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    BlipInst & operator = (BlipInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CBlip *         mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    int32_t         mWorld{-1}; // The identifier of the world in which this blip was created.
    int32_t         mScale{-1}; // The scale of the blip.

    // ----------------------------------------------------------------------------------------
    int32_t         mSprID{-1};

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
    Vector3         mPosition{};
    Color4          mColor{};

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a checkpoint entity instance on the server.
*/
struct CheckpointInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    CheckpointInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    CheckpointInst(const CheckpointInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    CheckpointInst(CheckpointInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~CheckpointInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    CheckpointInst & operator = (const CheckpointInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    CheckpointInst & operator = (CheckpointInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CCheckpoint *   mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    LgCheckpoint *  mLgInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mLgObj{}; // Script object of the instance used to interact this entity.
#endif

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnStream{};
#endif
    // ----------------------------------------------------------------------------------------
    SignalPair      mOnEntered{};
    SignalPair      mOnExited{};
    SignalPair      mOnWorld{};
    SignalPair      mOnRadius{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a key-bind entity instance on the server.
*/
struct KeyBindInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    KeyBindInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    KeyBindInst(const KeyBindInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    KeyBindInst(KeyBindInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~KeyBindInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    KeyBindInst & operator = (const KeyBindInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    KeyBindInst & operator = (KeyBindInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CKeyBind *      mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    int32_t         mFirst{-1}; // Key-code of the first button from the triggering combination.
    int32_t         mSecond{-1}; // Key-code of the second button from the triggering combination.
    int32_t         mThird{-1}; // Key-code of the third button from the triggering combination.
    int32_t         mRelease{-1}; // Whether the key-bind reacts to button press or release.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnKeyPress{};
    SignalPair      mOnKeyRelease{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify an object entity instance on the server.
*/
struct ObjectInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    ObjectInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    ObjectInst(const ObjectInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    ObjectInst(ObjectInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~ObjectInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    ObjectInst & operator = (const ObjectInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    ObjectInst & operator = (ObjectInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CObject *       mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    LgObject *      mLgInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mLgObj{}; // Script object of the instance used to interact this entity.
#endif

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnStream{};
#endif
    // ----------------------------------------------------------------------------------------
    SignalPair      mOnShot{};
    SignalPair      mOnTouched{};
    SignalPair      mOnWorld{};
    SignalPair      mOnAlpha{};
    SignalPair      mOnReport{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a pickup entity instance on the server.
*/
struct PickupInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    PickupInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    PickupInst(const PickupInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    PickupInst(PickupInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~PickupInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    PickupInst & operator = (const PickupInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    PickupInst & operator = (PickupInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CPickup *       mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    LgPickup *      mLgInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mLgObj{}; // Script object of the instance used to interact this entity.
#endif

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnStream{};
#endif
    // ----------------------------------------------------------------------------------------
    SignalPair      mOnRespawn{};
    SignalPair      mOnClaimed{};
    SignalPair      mOnCollected{};
    SignalPair      mOnWorld{};
    SignalPair      mOnAlpha{};
    SignalPair      mOnAutomatic{};
    SignalPair      mOnAutoTimer{};
    SignalPair      mOnOption{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a player entity instance on the server.
*/
struct PlayerInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    PlayerInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    PlayerInst(const PlayerInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    PlayerInst(PlayerInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~PlayerInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    PlayerInst & operator = (const PlayerInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    PlayerInst & operator = (PlayerInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CPlayer *       mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    AreaList        mAreas{}; // Areas the player is currently in.
    double          mDistance{0}; // Distance traveled while tracking was enabled.

    // ----------------------------------------------------------------------------------------
    SQInteger       mTrackPosition{0}; // The number of times to track position changes.
    SQInteger       mTrackHeading{0}; // The number of times to track heading changes.

    // ----------------------------------------------------------------------------------------
    int32_t         mTrackPositionHeader{0}; // Header to send when triggering position callback.
    LightObj        mTrackPositionPayload{}; // Payload to send when triggering position callback.

    // ----------------------------------------------------------------------------------------
    int32_t         mKickBanHeader{0}; // Header to send when triggering kick/ban callback.
    LightObj        mKickBanPayload{}; // Payload to send when triggering kick/ban callback.

    // ----------------------------------------------------------------------------------------
    int32_t         mLastWeapon{-1}; // Last known weapon of the player entity.
    float           mLastHealth{0}; // Last known health of the player entity.
    float           mLastArmour{0}; // Last known armor of the player entity.
    float           mLastHeading{0}; // Last known heading of the player entity.
    Vector3         mLastPosition{}; // Last known position of the player entity.

    // ----------------------------------------------------------------------------------------
    int32_t         mAuthority{0}; // The authority level of the managed player.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    LgPlayer *      mLgInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mLgObj{}; // Script object of the instance used to interact this entity.
#endif

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnStream{};
#endif
    // ----------------------------------------------------------------------------------------
    SignalPair      mOnRequestClass{};
    SignalPair      mOnRequestSpawn{};
    SignalPair      mOnSpawn{};
    SignalPair      mOnWasted{};
    SignalPair      mOnKilled{};
    SignalPair      mOnEmbarking{};
    SignalPair      mOnEmbarked{};
    SignalPair      mOnDisembark{};
    SignalPair      mOnRename{};
    SignalPair      mOnState{};
    SignalPair      mOnStateNone{};
    SignalPair      mOnStateNormal{};
    SignalPair      mOnStateAim{};
    SignalPair      mOnStateDriver{};
    SignalPair      mOnStatePassenger{};
    SignalPair      mOnStateEnterDriver{};
    SignalPair      mOnStateEnterPassenger{};
    SignalPair      mOnStateExit{};
    SignalPair      mOnStateUnspawned{};
    SignalPair      mOnAction{};
    SignalPair      mOnActionNone{};
    SignalPair      mOnActionNormal{};
    SignalPair      mOnActionAiming{};
    SignalPair      mOnActionShooting{};
    SignalPair      mOnActionJumping{};
    SignalPair      mOnActionLieDown{};
    SignalPair      mOnActionGettingUp{};
    SignalPair      mOnActionJumpVehicle{};
    SignalPair      mOnActionDriving{};
    SignalPair      mOnActionDying{};
    SignalPair      mOnActionWasted{};
    SignalPair      mOnActionEmbarking{};
    SignalPair      mOnActionDisembarking{};
    SignalPair      mOnBurning{};
    SignalPair      mOnCrouching{};
    SignalPair      mOnGameKeys{};
    SignalPair      mOnStartTyping{};
    SignalPair      mOnStopTyping{};
    SignalPair      mOnAway{};
    SignalPair      mOnMessage{};
    SignalPair      mOnCommand{};
    SignalPair      mOnPrivateMessage{};
    SignalPair      mOnKeyPress{};
    SignalPair      mOnKeyRelease{};
    SignalPair      mOnSpectate{};
    SignalPair      mOnUnspectate{};
    SignalPair      mOnCrashReport{};
    SignalPair      mOnModuleList{};
    SignalPair      mOnObjectShot{};
    SignalPair      mOnObjectTouched{};
    SignalPair      mOnPickupClaimed{};
    SignalPair      mOnPickupCollected{};
    SignalPair      mOnCheckpointEntered{};
    SignalPair      mOnCheckpointExited{};
    SignalPair      mOnClientScriptData{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnEntityStream{};
#endif
    SignalPair      mOnUpdate{};
    SignalPair      mOnHealth{};
    SignalPair      mOnArmour{};
    SignalPair      mOnWeapon{};
    SignalPair      mOnHeading{};
    SignalPair      mOnPosition{};
    SignalPair      mOnOption{};
    SignalPair      mOnAdmin{};
    SignalPair      mOnWorld{};
    SignalPair      mOnTeam{};
    SignalPair      mOnSkin{};
    SignalPair      mOnMoney{};
    SignalPair      mOnScore{};
    SignalPair      mOnWantedLevel{};
    SignalPair      mOnImmunity{};
    SignalPair      mOnAlpha{};
    SignalPair      mOnEnterArea{};
    SignalPair      mOnLeaveArea{};
};

/* --------------------------------------------------------------------------------------------
 * Helper structure used to identify a vehicle entity instance on the server.
*/
struct VehicleInst
{
    /* ----------------------------------------------------------------------------------------
     * Default constructor.
    */
    VehicleInst() = default;

    /* ----------------------------------------------------------------------------------------
     * Copy constructor (disabled).
    */
    VehicleInst(const VehicleInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move constructor.
    */
    VehicleInst(VehicleInst && o) noexcept = default;

    /* ----------------------------------------------------------------------------------------
     * Destructor.
    */
    ~VehicleInst();

    /* ----------------------------------------------------------------------------------------
     * Copy assignment operator (disabled).
    */
    VehicleInst & operator = (const VehicleInst & o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Move assignment operator (disabled).
    */
    VehicleInst & operator = (VehicleInst && o) = delete;

    /* ----------------------------------------------------------------------------------------
     * Destroy the entity instance from the server, if necessary.
    */
    void Destroy(bool destroy, int32_t header, LightObj & payload);

    /* ----------------------------------------------------------------------------------------
     * Reset the instance to the default values.
    */
    void ResetInstance();

    /* ----------------------------------------------------------------------------------------
     * Create the associated signals.
    */
    void InitEvents();

    /* ----------------------------------------------------------------------------------------
     * Clear the associated signals.
    */
    void DropEvents();

    // ----------------------------------------------------------------------------------------
    int32_t         mID{-1}; // The unique number that identifies this entity on the server.
    uint32_t        mFlags{ENF_DEFAULT}; // Various options and states that can be toggled on the instance.
    CVehicle *      mInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mObj{}; // Script object of the instance used to interact this entity.

    // ----------------------------------------------------------------------------------------
    AreaList        mAreas{}; // Areas the vehicle is currently in.
    double          mDistance{0}; // Distance traveled while tracking was enabled.

    // ----------------------------------------------------------------------------------------
    SQInteger       mTrackPosition{0}; // The number of times to track position changes.
    SQInteger       mTrackRotation{0}; // The number of times to track rotation changes.

    // ----------------------------------------------------------------------------------------
    int32_t         mLastPrimaryColor{-1}; // Last known secondary-color of the vehicle entity.
    int32_t         mLastSecondaryColor{-1}; // Last known primary-color of the vehicle entity.
    float           mLastHealth{0}; // Last known health of the vehicle entity.
    Vector3         mLastPosition{}; // Last known position of the vehicle entity.
    Quaternion      mLastRotation{}; // Last known rotation of the vehicle entity.

    // ----------------------------------------------------------------------------------------
    LightObj        mEvents{}; // Table containing the emitted entity events.

    // ----------------------------------------------------------------------------------------
#ifdef VCMP_ENABLE_OFFICIAL
    LgVehicle *     mLgInst{nullptr}; // Pointer to the actual instance used to interact this entity.
    LightObj        mLgObj{}; // Script object of the instance used to interact this entity.
#endif

    // ----------------------------------------------------------------------------------------
    SignalPair      mOnDestroyed{};
    SignalPair      mOnCustom{};
#if SQMOD_SDK_LEAST(2, 1)
    SignalPair      mOnStream{};
#endif
    // ----------------------------------------------------------------------------------------
    SignalPair      mOnEmbarking{};
    SignalPair      mOnEmbarked{};
    SignalPair      mOnDisembark{};
    SignalPair      mOnExplode{};
    SignalPair      mOnRespawn{};
    SignalPair      mOnUpdate{};
    SignalPair      mOnColor{};
    SignalPair      mOnHealth{};
    SignalPair      mOnPosition{};
    SignalPair      mOnRotation{};
    SignalPair      mOnOption{};
    SignalPair      mOnWorld{};
    SignalPair      mOnImmunity{};
    SignalPair      mOnPartStatus{};
    SignalPair      mOnTyreStatus{};
    SignalPair      mOnDamageData{};
    SignalPair      mOnRadio{};
    SignalPair      mOnHandlingRule{};
    SignalPair      mOnEnterArea{};
    SignalPair      mOnLeaveArea{};
};

} // Namespace:: SqMod
