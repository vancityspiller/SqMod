// ------------------------------------------------------------------------------------------------
#include "Base/Vector2.hpp"
#include "Base/Vector2i.hpp"
#include "Base/Shared.hpp"
#include "Library/Random.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
const Vector2 Vector2::NIL = Vector2(0);
const Vector2 Vector2::MIN = Vector2(NumLimit< Vector2::Value >::Min);
const Vector2 Vector2::MAX = Vector2(NumLimit< Vector2::Value >::Max);

// ------------------------------------------------------------------------------------------------
SQChar Vector2::Delim = ',';

// ------------------------------------------------------------------------------------------------
Vector2::Vector2()
    : x(0.0), y(0.0)
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
Vector2::Vector2(Value sv)
    : x(sv), y(sv)
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
Vector2::Vector2(Value xv, Value yv)
    : x(xv), y(yv)
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
Vector2::Vector2(const Vector2 & o)
    : x(o.x), y(o.y)
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
Vector2::~Vector2()
{
    /* ... */
}

// ------------------------------------------------------------------------------------------------
Vector2 & Vector2::operator = (const Vector2 & o)
{
    x = o.x;
    y = o.y;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Vector2 & Vector2::operator = (Value s)
{
    x = s;
    y = s;
    return *this;
}

Vector2 & Vector2::operator = (CSStr values)
{
    Set(GetVector2(values, Delim));
    return *this;
}

Vector2 & Vector2::operator = (const Vector2i & v)
{
    x = static_cast<Value>(v.x);
    y = static_cast<Value>(v.y);
    return *this;
}

// ------------------------------------------------------------------------------------------------
Vector2 & Vector2::operator += (const Vector2 & v)
{
    x += v.x;
    y += v.y;
    return *this;
}

Vector2 & Vector2::operator -= (const Vector2 & v)
{
    x -= v.x;
    y -= v.y;
    return *this;
}

Vector2 & Vector2::operator *= (const Vector2 & v)
{
    x *= v.x;
    y *= v.y;
    return *this;
}

Vector2 & Vector2::operator /= (const Vector2 & v)
{
    x /= v.x;
    y /= v.y;
    return *this;
}

Vector2 & Vector2::operator %= (const Vector2 & v)
{
    x = fmod(x, v.x);
    y = fmod(y, v.y);
    return *this;
}

// ------------------------------------------------------------------------------------------------
Vector2 & Vector2::operator += (Value s)
{
    x += s;
    y += s;
    return *this;
}

Vector2 & Vector2::operator -= (Value s)
{
    x -= s;
    y -= s;
    return *this;
}

Vector2 & Vector2::operator *= (Value s)
{
    x *= s;
    y *= s;
    return *this;
}

Vector2 & Vector2::operator /= (Value s)
{
    x /= s;
    y /= s;
    return *this;
}

Vector2 & Vector2::operator %= (Value s)
{
    x = fmod(x, s);
    y = fmod(y, s);
    return *this;
}

// ------------------------------------------------------------------------------------------------
Vector2 & Vector2::operator ++ ()
{
    ++x;
    ++y;
    return *this;
}

Vector2 & Vector2::operator -- ()
{
    --x;
    --y;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Vector2 Vector2::operator ++ (int)
{
    Vector2 state(*this);
    ++x;
    ++y;
    return state;
}

Vector2 Vector2::operator -- (int)
{
    Vector2 state(*this);
    --x;
    --y;
    return state;
}

// ------------------------------------------------------------------------------------------------
Vector2 Vector2::operator + (const Vector2 & v) const
{
    return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator - (const Vector2 & v) const
{
    return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator * (const Vector2 & v) const
{
    return Vector2(x * v.x, y * v.y);
}

Vector2 Vector2::operator / (const Vector2 & v) const
{
    return Vector2(x / v.x, y / v.y);
}

Vector2 Vector2::operator % (const Vector2 & v) const
{
    return Vector2(fmod(x, v.x), fmod(y, v.y));
}

// ------------------------------------------------------------------------------------------------
Vector2 Vector2::operator + (Value s) const
{
    return Vector2(x + s, y + s);
}

Vector2 Vector2::operator - (Value s) const
{
    return Vector2(x - s, y - s);
}

Vector2 Vector2::operator * (Value s) const
{
    return Vector2(x * s, y * s);
}

Vector2 Vector2::operator / (Value s) const
{
    return Vector2(x / s, y / s);
}

Vector2 Vector2::operator % (Value s) const
{
    return Vector2(fmod(x, s), fmod(y, s));
}

// ------------------------------------------------------------------------------------------------
Vector2 Vector2::operator + () const
{
    return Vector2(fabs(x), fabs(y));
}

Vector2 Vector2::operator - () const
{
    return Vector2(-x, -y);
}

// ------------------------------------------------------------------------------------------------
bool Vector2::operator == (const Vector2 & v) const
{
    return EpsEq(x, v.x) && EpsEq(y, v.y);
}

bool Vector2::operator != (const Vector2 & v) const
{
    return !EpsEq(x, v.x) && !EpsEq(y, v.y);
}

bool Vector2::operator < (const Vector2 & v) const
{
    return EpsLt(x, v.x) && EpsLt(y, v.y);
}

bool Vector2::operator > (const Vector2 & v) const
{
    return EpsGt(x, v.x) && EpsGt(y, v.y);
}

bool Vector2::operator <= (const Vector2 & v) const
{
    return EpsLtEq(x, v.x) && EpsLtEq(y, v.y);
}

bool Vector2::operator >= (const Vector2 & v) const
{
    return EpsGtEq(x, v.x) && EpsGtEq(y, v.y);
}

// ------------------------------------------------------------------------------------------------
Int32 Vector2::Cmp(const Vector2 & o) const
{
    if (*this == o)
        return 0;
    else if (*this > o)
        return 1;
    else
        return -1;
}

// ------------------------------------------------------------------------------------------------
CSStr Vector2::ToString() const
{
    return ToStringF("%f,%f", x, y);
}

// ------------------------------------------------------------------------------------------------
void Vector2::Set(Value ns)
{
    x = ns;
    y = ns;
}

void Vector2::Set(Value nx, Value ny)
{
    x = nx;
    y = ny;
}

// ------------------------------------------------------------------------------------------------
void Vector2::Set(const Vector2 & v)
{
    x = v.x;
    y = v.y;
}

void Vector2::Set(const Vector2i & v)
{
    x = static_cast<Value>(v.x);
    y = static_cast<Value>(v.y);
}

// ------------------------------------------------------------------------------------------------
void Vector2::Set(CSStr values, SQChar delim)
{
    Set(GetVector2(values, delim));
}

// ------------------------------------------------------------------------------------------------
void Vector2::Generate()
{
    x = GetRandomFloat32();
    y = GetRandomFloat32();
}

void Vector2::Generate(Value min, Value max)
{
    if (EpsLt(max, min))
    {
        SqThrow("max value is lower than min value");
    }
    else
    {
        x = GetRandomFloat32(min, max);
        y = GetRandomFloat32(min, max);
    }
}

void Vector2::Generate(Value xmin, Value xmax, Value ymin, Value ymax)
{
    if (EpsLt(xmax, xmin) || EpsLt(ymax, ymin))
    {
        SqThrow("max value is lower than min value");
    }
    else
    {
        x = GetRandomFloat32(ymin, ymax);
        y = GetRandomFloat32(xmin, xmax);
    }
}

// ------------------------------------------------------------------------------------------------
Vector2 Vector2::Abs() const
{
    return Vector2(fabs(x), fabs(y));
}

// ================================================================================================
void Register_Vector2(HSQUIRRELVM vm)
{
    typedef Vector2::Value Val;

    RootTable(vm).Bind(_SC("Vector2"), Class< Vector2 >(vm, _SC("Vector2"))
        /* Constructors */
        .Ctor()
        .Ctor< Val >()
        .Ctor< Val, Val >()
        /* Static Members */
        .SetStaticValue(_SC("Delim"), &Vector2::Delim)
        /* Member Variables */
        .Var(_SC("x"), &Vector2::x)
        .Var(_SC("y"), &Vector2::y)
        /* Properties */
        .Prop(_SC("abs"), &Vector2::Abs)
        /* Core Metamethods */
        .Func(_SC("_tostring"), &Vector2::ToString)
        .Func(_SC("_cmp"), &Vector2::Cmp)
        /* Metamethods */
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("_add"), &Vector2::operator +)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("_sub"), &Vector2::operator -)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("_mul"), &Vector2::operator *)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("_div"), &Vector2::operator /)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("_modulo"), &Vector2::operator %)
        .Func<Vector2 (Vector2::*)(void) const>(_SC("_unm"), &Vector2::operator -)
        /* Setters */
        .Overload<void (Vector2::*)(Val)>(_SC("Set"), &Vector2::Set)
        .Overload<void (Vector2::*)(Val, Val)>(_SC("Set"), &Vector2::Set)
        .Overload<void (Vector2::*)(const Vector2 &)>(_SC("SetVec2"), &Vector2::Set)
        .Overload<void (Vector2::*)(const Vector2i &)>(_SC("SetVec2i"), &Vector2::Set)
        .Overload<void (Vector2::*)(CSStr, SQChar)>(_SC("SetStr"), &Vector2::Set)
        /* Random Generators */
        .Overload<void (Vector2::*)(void)>(_SC("Generate"), &Vector2::Generate)
        .Overload<void (Vector2::*)(Val, Val)>(_SC("Generate"), &Vector2::Generate)
        .Overload<void (Vector2::*)(Val, Val, Val, Val)>(_SC("Generate"), &Vector2::Generate)
        /* Utility Methods */
        .Func(_SC("Clear"), &Vector2::Clear)
        /* Operator Exposure */
        .Func<Vector2 & (Vector2::*)(const Vector2 &)>(_SC("opAddAssign"), &Vector2::operator +=)
        .Func<Vector2 & (Vector2::*)(const Vector2 &)>(_SC("opSubAssign"), &Vector2::operator -=)
        .Func<Vector2 & (Vector2::*)(const Vector2 &)>(_SC("opMulAssign"), &Vector2::operator *=)
        .Func<Vector2 & (Vector2::*)(const Vector2 &)>(_SC("opDivAssign"), &Vector2::operator /=)
        .Func<Vector2 & (Vector2::*)(const Vector2 &)>(_SC("opModAssign"), &Vector2::operator %=)

        .Func<Vector2 & (Vector2::*)(Vector2::Value)>(_SC("opAddAssignS"), &Vector2::operator +=)
        .Func<Vector2 & (Vector2::*)(Vector2::Value)>(_SC("opSubAssignS"), &Vector2::operator -=)
        .Func<Vector2 & (Vector2::*)(Vector2::Value)>(_SC("opMulAssignS"), &Vector2::operator *=)
        .Func<Vector2 & (Vector2::*)(Vector2::Value)>(_SC("opDivAssignS"), &Vector2::operator /=)
        .Func<Vector2 & (Vector2::*)(Vector2::Value)>(_SC("opModAssignS"), &Vector2::operator %=)

        .Func<Vector2 & (Vector2::*)(void)>(_SC("opPreInc"), &Vector2::operator ++)
        .Func<Vector2 & (Vector2::*)(void)>(_SC("opPreDec"), &Vector2::operator --)
        .Func<Vector2 (Vector2::*)(int)>(_SC("opPostInc"), &Vector2::operator ++)
        .Func<Vector2 (Vector2::*)(int)>(_SC("opPostDec"), &Vector2::operator --)

        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("opAdd"), &Vector2::operator +)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("opSub"), &Vector2::operator -)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("opMul"), &Vector2::operator *)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("opDiv"), &Vector2::operator /)
        .Func<Vector2 (Vector2::*)(const Vector2 &) const>(_SC("opMod"), &Vector2::operator %)

        .Func<Vector2 (Vector2::*)(Vector2::Value) const>(_SC("opAddS"), &Vector2::operator +)
        .Func<Vector2 (Vector2::*)(Vector2::Value) const>(_SC("opSubS"), &Vector2::operator -)
        .Func<Vector2 (Vector2::*)(Vector2::Value) const>(_SC("opMulS"), &Vector2::operator *)
        .Func<Vector2 (Vector2::*)(Vector2::Value) const>(_SC("opDivS"), &Vector2::operator /)
        .Func<Vector2 (Vector2::*)(Vector2::Value) const>(_SC("opModS"), &Vector2::operator %)

        .Func<Vector2 (Vector2::*)(void) const>(_SC("opUnPlus"), &Vector2::operator +)
        .Func<Vector2 (Vector2::*)(void) const>(_SC("opUnMinus"), &Vector2::operator -)

        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opEqual"), &Vector2::operator ==)
        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opNotEqual"), &Vector2::operator !=)
        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opLessThan"), &Vector2::operator <)
        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opGreaterThan"), &Vector2::operator >)
        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opLessEqual"), &Vector2::operator <=)
        .Func<bool (Vector2::*)(const Vector2 &) const>(_SC("opGreaterEqual"), &Vector2::operator >=)
    );
}

} // Namespace:: SqMod