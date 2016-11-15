// ------------------------------------------------------------------------------------------------
#include "Entity/Blip.hpp"
#include "Core.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
SQMODE_DECL_TYPENAME(Typename, _SC("SqBlip"))

// ------------------------------------------------------------------------------------------------
const Int32 CBlip::Max = SQMOD_BLIP_POOL;

// ------------------------------------------------------------------------------------------------
SQInteger CBlip::SqGetNull(HSQUIRRELVM vm)
{
    sq_pushobject(vm, Core::Get().GetNullBlip().GetObject());
    return 1;
}

// ------------------------------------------------------------------------------------------------
Object & CBlip::GetNull()
{
    return Core::Get().GetNullBlip();
}

// ------------------------------------------------------------------------------------------------
CBlip::CBlip(Int32 id)
    : m_ID(VALID_ENTITYGETEX(id, SQMOD_BLIP_POOL))
    , m_Tag(ToStrF("%d", id))
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
CBlip::~CBlip()
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
const String & CBlip::ToString() const
{
    return m_Tag;
}

// ------------------------------------------------------------------------------------------------
const String & CBlip::GetTag() const
{
    return m_Tag;
}

// ------------------------------------------------------------------------------------------------
void CBlip::SetTag(CSStr tag)
{
    m_Tag.assign(tag);
}

// ------------------------------------------------------------------------------------------------
Object & CBlip::GetData()
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return m_Data;
}

// ------------------------------------------------------------------------------------------------
void CBlip::SetData(Object & data)
{
    // Validate the managed identifier
    Validate();
    // Apply the specified value
    m_Data = data;
}

// ------------------------------------------------------------------------------------------------
bool CBlip::Destroy(Int32 header, Object & payload)
{
    // Validate the managed identifier
    Validate();
    // Perform the requested operation
    return Core::Get().DelBlip(m_ID, header, payload);
}

// ------------------------------------------------------------------------------------------------
void CBlip::BindEvent(Int32 evid, Object & env, Function & func) const
{
    // Validate the managed identifier
    Validate();
    // Obtain the function instance called for this event
    Function & event = Core::Get().GetBlipEvent(m_ID, evid);
    // Is the specified callback function null?
    if (func.IsNull())
    {
        event.ReleaseGently(); // Then release the current callback
    }
    // Does this function need a custom environment?
    else if (env.IsNull())
    {
        event = func;
    }
    // Assign the specified environment and function
    else
    {
        event = Function(env.GetVM(), env, func.GetFunc());
    }
}

// ------------------------------------------------------------------------------------------------
void CBlip::CustomEvent(Int32 header, Object & payload) const
{
    // Validate the managed identifier
    Validate();
    // Perfrom the requested action
    Core::Get().EmitBlipCustom(m_ID, header, payload);
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetWorld() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mWorld;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetScale() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mScale;
}

// ------------------------------------------------------------------------------------------------
const Vector3 & CBlip::GetPosition() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mPosition;
}

// ------------------------------------------------------------------------------------------------
const Color4 & CBlip::GetColor() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mColor;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetSprID() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mSprID;
}

// ------------------------------------------------------------------------------------------------
Float32 CBlip::GetPositionX() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mPosition.x;
}

// ------------------------------------------------------------------------------------------------
Float32 CBlip::GetPositionY() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mPosition.y;
}

// ------------------------------------------------------------------------------------------------
Float32 CBlip::GetPositionZ() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mPosition.z;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetColorR() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mColor.r;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetColorG() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mColor.g;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetColorB() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mColor.b;
}

// ------------------------------------------------------------------------------------------------
Int32 CBlip::GetColorA() const
{
    // Validate the managed identifier
    Validate();
    // Return the requested information
    return Core::Get().GetBlip(m_ID).mColor.a;
}

// ------------------------------------------------------------------------------------------------
static Object & Blip_CreateEx(Int32 world, Float32 x, Float32 y, Float32 z, Int32 scale,
                            Uint8 r, Uint8 g, Uint8 b, Uint8 a, Int32 sprid)
{
    return Core::Get().NewBlip(-1, world, x, y, z, scale, SQMOD_PACK_RGBA(r, g, b, a), sprid,
                            SQMOD_CREATE_DEFAULT, NullObject());
}

static Object & Blip_CreateEx(Int32 world, Float32 x, Float32 y, Float32 z, Int32 scale,
                            Uint8 r, Uint8 g, Uint8 b, Uint8 a, Int32 sprid,
                            Int32 header, Object & payload)
{
    return Core::Get().NewBlip(-1, world, x, y, z, scale, SQMOD_PACK_RGBA(r, g, b, a), sprid,
                            header, payload);
}

// ------------------------------------------------------------------------------------------------
static Object & Blip_CreateEx(Int32 index, Int32 world, Float32 x, Float32 y, Float32 z,
                            Int32 scale, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Int32 sprid)
{
    return Core::Get().NewBlip(index, world, x, y, z, scale, SQMOD_PACK_RGBA(r, g, b, a), sprid,
                            SQMOD_CREATE_DEFAULT, NullObject());
}

static Object & Blip_CreateEx(Int32 index, Int32 world, Float32 x, Float32 y, Float32 z, Int32 scale,
                            Uint8 r, Uint8 g, Uint8 b, Uint8 a, Int32 sprid,
                            Int32 header, Object & payload)
{
    return Core::Get().NewBlip(index, world, x, y, z, scale, SQMOD_PACK_RGBA(r, g, b, a), sprid,
                            header, payload);
}

// ------------------------------------------------------------------------------------------------
static Object & Blip_Create(Int32 world, const Vector3 & pos, Int32 scale, const Color4 & color,
                            Int32 sprid)
{
    return Core::Get().NewBlip(-1, world, pos.x, pos.y, pos.z, scale, color.GetRGBA(), sprid,
                            SQMOD_CREATE_DEFAULT, NullObject());
}

static Object & Blip_Create(Int32 world, const Vector3 & pos, Int32 scale, const Color4 & color,
                            Int32 sprid, Int32 header, Object & payload)
{
    return Core::Get().NewBlip(-1, world, pos.x, pos.y, pos.z, scale, color.GetRGBA(), sprid,
                            header, payload);
}

// ------------------------------------------------------------------------------------------------
static Object & Blip_Create(Int32 index, Int32 world, const Vector3 & pos, Int32 scale,
                            const Color4 & color, Int32 sprid)
{
    return Core::Get().NewBlip(index, world, pos.x, pos.y, pos.z, scale, color.GetRGBA(), sprid,
                            SQMOD_CREATE_DEFAULT, NullObject());
}

static Object & Blip_Create(Int32 index, Int32 world, const Vector3 & pos, Int32 scale,
                            const Color4 & color, Int32 sprid, Int32 header, Object & payload)
{
    return Core::Get().NewBlip(index, world, pos.x, pos.y, pos.z, scale, color.GetRGBA(), sprid,
                            header, payload);
}

// ================================================================================================
void Register_CBlip(HSQUIRRELVM vm)
{
    RootTable(vm).Bind(Typename::Str,
        Class< CBlip, NoConstructor< CBlip > >(vm, Typename::Str)
        // Meta-methods
        .SquirrelFunc(_SC("_typename"), &Typename::Fn)
        .Func(_SC("_tostring"), &CBlip::ToString)
        // Static Values
        .SetStaticValue(_SC("MaxID"), CBlip::Max)
        // Core Properties
        .Prop(_SC("ID"), &CBlip::GetID)
        .Prop(_SC("Tag"), &CBlip::GetTag, &CBlip::SetTag)
        .Prop(_SC("Data"), &CBlip::GetData, &CBlip::SetData)
        .Prop(_SC("Active"), &CBlip::IsActive)
        // Core Methods
        .Func(_SC("Bind"), &CBlip::BindEvent)
        .Func(_SC("CustomEvent"), &CBlip::CustomEvent)
        // Core Overloads
        .Overload< bool (CBlip::*)(void) >(_SC("Destroy"), &CBlip::Destroy)
        .Overload< bool (CBlip::*)(Int32) >(_SC("Destroy"), &CBlip::Destroy)
        .Overload< bool (CBlip::*)(Int32, Object &) >(_SC("Destroy"), &CBlip::Destroy)
        // Properties
        .Prop(_SC("World"), &CBlip::GetWorld)
        .Prop(_SC("Scale"), &CBlip::GetScale)
        .Prop(_SC("Pos"), &CBlip::GetPosition)
        .Prop(_SC("Position"), &CBlip::GetPosition)
        .Prop(_SC("Color"), &CBlip::GetColor)
        .Prop(_SC("Colour"), &CBlip::GetColor)
        .Prop(_SC("SprID"), &CBlip::GetSprID)
        .Prop(_SC("PosX"), &CBlip::GetPositionX)
        .Prop(_SC("PosY"), &CBlip::GetPositionY)
        .Prop(_SC("PosZ"), &CBlip::GetPositionZ)
        .Prop(_SC("Red"), &CBlip::GetColorR)
        .Prop(_SC("Green"), &CBlip::GetColorG)
        .Prop(_SC("Blue"), &CBlip::GetColorB)
        .Prop(_SC("Alpha"), &CBlip::GetColorA)
        // Static Overloads
        .StaticOverload< Object & (*)(Int32, Float32, Float32, Float32, Int32, Uint8, Uint8, Uint8, Uint8, Int32) >
            (_SC("CreateEx"), &Blip_CreateEx)
        .StaticOverload< Object & (*)(Int32, Float32, Float32, Float32, Int32, Uint8, Uint8, Uint8, Uint8, Int32, Int32, Object &) >
            (_SC("CreateEx"), &Blip_CreateEx)
        .StaticOverload< Object & (*)(Int32, Int32, Float32, Float32, Float32, Int32, Uint8, Uint8, Uint8, Uint8, Int32) >
            (_SC("CreateEx"), &Blip_CreateEx)
        .StaticOverload< Object & (*)(Int32, Int32, Float32, Float32, Float32, Int32, Uint8, Uint8, Uint8, Uint8, Int32, Int32, Object &) >
            (_SC("CreateEx"), &Blip_CreateEx)
        .StaticOverload< Object & (*)(Int32, const Vector3 &, Int32, const Color4 &, Int32) >
            (_SC("Create"), &Blip_Create)
        .StaticOverload< Object & (*)(Int32, const Vector3 &, Int32, const Color4 &, Int32, Int32, Object &) >
            (_SC("Create"), &Blip_Create)
        .StaticOverload< Object & (*)(Int32, Int32, const Vector3 &, Int32, const Color4 &, Int32) >
            (_SC("Create"), &Blip_Create)
        .StaticOverload< Object & (*)(Int32, Int32, const Vector3 &, Int32, const Color4 &, Int32, Int32, Object &) >
            (_SC("Create"), &Blip_Create)
        // Raw Squirrel Methods
        .SquirrelFunc(_SC("NullInst"), &CBlip::SqGetNull)
    );
}

} // Namespace:: SqMod
