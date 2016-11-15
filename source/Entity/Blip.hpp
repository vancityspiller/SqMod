#ifndef _ENTITY_BLIP_HPP_
#define _ENTITY_BLIP_HPP_

// ------------------------------------------------------------------------------------------------
#include "Base/Shared.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

/* ------------------------------------------------------------------------------------------------
 * Manages a single blip entity.
*/
class CBlip
{
    // --------------------------------------------------------------------------------------------
    friend class Core;

private:

    /* --------------------------------------------------------------------------------------------
     * Identifier of the managed entity.
    */
    Int32   m_ID;

    /* --------------------------------------------------------------------------------------------
     * User tag associated with this instance.
    */
    String  m_Tag;

    /* --------------------------------------------------------------------------------------------
     * User data associated with this instance.
    */
    Object  m_Data;

    /* --------------------------------------------------------------------------------------------
     * Base constructor.
    */
    CBlip(Int32 id);

public:

    /* --------------------------------------------------------------------------------------------
     * Maximum possible number that could represent an identifier for this entity type.
    */
    static const Int32 Max;

    /* --------------------------------------------------------------------------------------------
     * Copy constructor. (disabled)
    */
    CBlip(const CBlip &) = delete;

    /* --------------------------------------------------------------------------------------------
     * Move constructor. (disabled)
    */
    CBlip(CBlip &&) = delete;

    /* --------------------------------------------------------------------------------------------
     * Destructor.
    */
    ~CBlip();

    /* --------------------------------------------------------------------------------------------
     * Copy assignment operator. (disabled)
    */
    CBlip & operator = (const CBlip &) = delete;

    /* --------------------------------------------------------------------------------------------
     * Move assignment operator. (disabled)
    */
    CBlip & operator = (CBlip &&) = delete;

    /* --------------------------------------------------------------------------------------------
     * See whether this instance manages a valid entity otherwise throw an exception.
    */
    void Validate() const
    {
        if (INVALID_ENTITY(m_ID))
        {
            STHROWF("Invalid blip reference [%s]", m_Tag.c_str());
        }
    }

    /* --------------------------------------------------------------------------------------------
     * Used by the script engine to compare two instances of this type.
    */
    Int32 Cmp(const CBlip & o) const;

    /* --------------------------------------------------------------------------------------------
     * Used by the script engine to convert an instance of this type to a string.
    */
    const String & ToString() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the associated null entity instance.
    */
    static SQInteger SqGetNull(HSQUIRRELVM vm);

    /* --------------------------------------------------------------------------------------------
     * Retrieve the associated null entity instance.
    */
    static Object & GetNull();

    /* --------------------------------------------------------------------------------------------
     * Retrieve the identifier of the entity managed by this instance.
    */
    Int32 GetID() const
    {
        return m_ID;
    }

    /* --------------------------------------------------------------------------------------------
     * Check whether this instance manages a valid entity.
    */
    bool IsActive() const
    {
        return VALID_ENTITY(m_ID);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the associated user tag.
    */
    const String & GetTag() const;

    /* --------------------------------------------------------------------------------------------
     * Modify the associated user tag.
    */
    void SetTag(CSStr tag);

    /* --------------------------------------------------------------------------------------------
     * Retrieve the associated user data.
    */
    Object & GetData();

    /* --------------------------------------------------------------------------------------------
     * Modify the associated user data.
    */
    void SetData(Object & data);

    /* --------------------------------------------------------------------------------------------
     * Destroy the managed blip entity.
    */
    bool Destroy()
    {
        return Destroy(0, NullObject());
    }

    /* --------------------------------------------------------------------------------------------
     * Destroy the managed blip entity.
    */
    bool Destroy(Int32 header)
    {
        return Destroy(header, NullObject());
    }

    /* --------------------------------------------------------------------------------------------
     * Destroy the managed blip entity.
    */
    bool Destroy(Int32 header, Object & payload);

    /* --------------------------------------------------------------------------------------------
     * Bind to an event supported by this entity type.
    */
    void BindEvent(Int32 evid, Object & env, Function & func) const;

    /* --------------------------------------------------------------------------------------------
     * Emit a custom event for the managed entity
    */
    void CustomEvent(Int32 header, Object & payload) const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the world in which the referenced blip entity exists.
    */
    Int32 GetWorld() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the scale of the managed blip entity.
    */
    Int32 GetScale() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the position of the managed blip entity.
    */
    const Vector3 & GetPosition() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the color of the managed blip entity.
    */
    const Color4 & GetColor() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the identifier of the sprite used by the managed blip entity.
    */
    Int32 GetSprID() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the position on the x axis of the managed blip entity.
    */
    Float32 GetPositionX() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the position on the y axis of the managed blip entity.
    */
    Float32 GetPositionY() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the position on the z axis of the managed blip entity.
    */
    Float32 GetPositionZ() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the red color of the managed blip entity.
    */
    Int32 GetColorR() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the green color of the managed blip entity.
    */
    Int32 GetColorG() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the blue color of the managed blip entity.
    */
    Int32 GetColorB() const;

    /* --------------------------------------------------------------------------------------------
     * Retrieve the alpha transparency of the managed blip entity.
    */
    Int32 GetColorA() const;
};

} // Namespace:: SqMod

#endif // _ENTITY_BLIP_HPP_