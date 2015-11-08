#include "Base/Color4.hpp"
#include "Base/Color3.hpp"
#include "Base/Shared.hpp"
#include "Register.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
const Color4 Color4::NIL = Color4();
const Color4 Color4::MIN = Color4(std::numeric_limits<Color4::Value>::min());
const Color4 Color4::MAX = Color4(std::numeric_limits<Color4::Value>::max());

// ------------------------------------------------------------------------------------------------
SQChar Color4::Delim = ',';

// ------------------------------------------------------------------------------------------------
Color4::Color4()
    : r(0), g(0), b(0), a(0)
{

}

Color4::Color4(Value s)
    : r(s), g(s), b(s), a(s)
{

}

Color4::Color4(Value rv, Value gv, Value bv)
    : r(rv), g(gv), b(bv), a(0)
{

}

Color4::Color4(Value rv, Value gv, Value bv, Value av)
    : r(rv), g(gv), b(bv), a(av)
{

}

// ------------------------------------------------------------------------------------------------
Color4::Color4(const Color3 & c)
    : r(c.r), g(c.g), b(c.b), a(0)
{

}

// ------------------------------------------------------------------------------------------------
Color4::Color4(const SQChar * name)
    : Color4(GetColor(name))
{

}

Color4::Color4(const SQChar * str, SQChar delim)
    : Color4(GetColor4(str, delim))
{

}

// ------------------------------------------------------------------------------------------------
Color4::Color4(const Color4 & c)
    : r(c.r), g(c.g), b(c.b), a(c.a)
{

}

Color4::Color4(Color4 && c)
    : r(c.r), g(c.g), b(c.b), a(c.a)
{

}

// ------------------------------------------------------------------------------------------------
Color4::~Color4()
{

}

// ------------------------------------------------------------------------------------------------
Color4 & Color4::operator = (const Color4 & c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return *this;
}

Color4 & Color4::operator = (Color4 && c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Color4 & Color4::operator = (Value s)
{
    r = s;
    g = s;
    b = s;
    a = s;
    return *this;
}

Color4 & Color4::operator = (const SQChar * name)
{
    Set(GetColor(name));
    return *this;
}

Color4 & Color4::operator = (const Color3 & c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Color4 & Color4::operator += (const Color4 & c)
{
    r += c.r;
    g += c.g;
    b += c.b;
    a += c.a;
    return *this;
}

Color4 & Color4::operator -= (const Color4 & c)
{
    r -= c.r;
    g -= c.g;
    b -= c.b;
    a -= c.a;
    return *this;
}

Color4 & Color4::operator *= (const Color4 & c)
{
    r *= c.r;
    g *= c.g;
    b *= c.b;
    a *= c.a;
    return *this;
}

Color4 & Color4::operator /= (const Color4 & c)
{
    r /= c.r;
    g /= c.g;
    b /= c.b;
    a /= c.a;
    return *this;
}

Color4 & Color4::operator %= (const Color4 & c)
{
    r %= c.r;
    g %= c.g;
    b %= c.b;
    a %= c.a;
    return *this;
}

Color4 & Color4::operator &= (const Color4 & c)
{
    r &= c.r;
    g &= c.g;
    b &= c.b;
    a &= c.a;
    return *this;
}

Color4 & Color4::operator |= (const Color4 & c)
{
    r |= c.r;
    g |= c.g;
    b |= c.b;
    a |= c.a;
    return *this;
}

Color4 & Color4::operator ^= (const Color4 & c)
{
    r ^= c.r;
    g ^= c.g;
    b ^= c.b;
    a ^= c.a;
    return *this;
}

Color4 & Color4::operator <<= (const Color4 & c)
{
    r <<= c.r;
    g <<= c.g;
    b <<= c.b;
    a <<= c.a;
    return *this;
}

Color4 & Color4::operator >>= (const Color4 & c)
{
    r >>= c.r;
    g >>= c.g;
    b >>= c.b;
    a >>= c.a;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Color4 & Color4::operator += (Value s)
{
    r += s;
    g += s;
    b += s;
    a += s;
    return *this;
}

Color4 & Color4::operator -= (Value s)
{
    r -= s;
    g -= s;
    b -= s;
    a -= s;
    return *this;
}

Color4 & Color4::operator *= (Value s)
{
    r *= s;
    g *= s;
    b *= s;
    a *= s;
    return *this;
}

Color4 & Color4::operator /= (Value s)
{
    r /= s;
    g /= s;
    b /= s;
    a /= s;
    return *this;
}

Color4 & Color4::operator %= (Value s)
{
    r %= s;
    g %= s;
    b %= s;
    a %= s;
    return *this;
}

Color4 & Color4::operator &= (Value s)
{
    r &= s;
    g &= s;
    b &= s;
    a &= s;
    return *this;
}

Color4 & Color4::operator |= (Value s)
{
    r |= s;
    g |= s;
    b |= s;
    a |= s;
    return *this;
}

Color4 & Color4::operator ^= (Value s)
{
    r ^= s;
    g ^= s;
    b ^= s;
    a ^= s;
    return *this;
}

Color4 & Color4::operator <<= (Value s)
{
    r <<= s;
    g <<= s;
    b <<= s;
    a <<= s;
    return *this;
}

Color4 & Color4::operator >>= (Value s)
{
    r >>= s;
    g >>= s;
    b >>= s;
    a >>= s;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Color4 & Color4::operator ++ ()
{
    ++r;
    ++g;
    ++b;
    ++a;
    return *this;
}

Color4 & Color4::operator -- ()
{
    --r;
    --g;
    --b;
    --a;
    return *this;
}

// ------------------------------------------------------------------------------------------------
Color4 Color4::operator ++ (int)
{
    Color4 state(*this);
    ++r;
    ++g;
    ++b;
    ++a;
    return state;
}

Color4 Color4::operator -- (int)
{
    Color4 state(*this);
    --r;
    --g;
    --b;
    --a;
    return state;
}

// ------------------------------------------------------------------------------------------------
Color4 Color4::operator + (const Color4 & c) const
{
    return Color4(r + c.r, g + c.g, b + c.b, a + c.a);
}

Color4 Color4::operator - (const Color4 & c) const
{
    return Color4(r - c.r, g - c.g, b - c.b, a - c.a);
}

Color4 Color4::operator * (const Color4 & c) const
{
    return Color4(r * c.r, g * c.g, b * c.b, a * c.a);
}

Color4 Color4::operator / (const Color4 & c) const
{
    return Color4(r / c.r, g / c.g, b / c.b, a / c.a);
}

Color4 Color4::operator % (const Color4 & c) const
{
    return Color4(r % c.r, g % c.g, b % c.b, a % c.a);
}

Color4 Color4::operator & (const Color4 & c) const
{
    return Color4(r & c.r, g & c.g, b & c.b, a & c.a);
}

Color4 Color4::operator | (const Color4 & c) const
{
    return Color4(r | c.r, g | c.g, b | c.b, a | c.a);
}

Color4 Color4::operator ^ (const Color4 & c) const
{
    return Color4(r ^ c.r, g ^ c.g, b ^ c.b, a ^ c.a);
}

Color4 Color4::operator << (const Color4 & c) const
{
    return Color4(r << c.r, g << c.g, b << c.b, a << c.a);
}

Color4 Color4::operator >> (const Color4 & c) const
{
    return Color4(r >> c.r, g >> c.g, b >> c.b, a >> c.a);
}

// ------------------------------------------------------------------------------------------------
Color4 Color4::operator + (Value s) const
{
    return Color4(r + s, g + s, b + s, a + s);
}

Color4 Color4::operator - (Value s) const
{
    return Color4(r - s, g - s, b - s, a - s);
}

Color4 Color4::operator * (Value s) const
{
    return Color4(r * s, g * s, b * s, a * s);
}

Color4 Color4::operator / (Value s) const
{
    return Color4(r / s, g / s, b / s, a / s);
}

Color4 Color4::operator % (Value s) const
{
    return Color4(r % s, g % s, b % s, a % s);
}

Color4 Color4::operator & (Value s) const
{
    return Color4(r & s, g & s, b & s, a & s);
}

Color4 Color4::operator | (Value s) const
{
    return Color4(r | s, g | s, b | s, a | s);
}

Color4 Color4::operator ^ (Value s) const
{
    return Color4(r ^ s, g ^ s, b ^ s, a ^ s);
}

Color4 Color4::operator << (Value s) const
{
    return Color4(r << s, g << s, b << s, a << s);
}

Color4 Color4::operator >> (Value s) const
{
    return Color4(r >> s, g >> s, b >> s, a >> s);
}

// ------------------------------------------------------------------------------------------------
Color4 Color4::operator + () const
{
    return Color4(r, g, b, a);
}

Color4 Color4::operator - () const
{
    return Color4(0, 0, 0, 0);
}

// ------------------------------------------------------------------------------------------------
Color4 Color4::operator ~ () const
{
    return Color4(~r, ~g, ~b, ~a);
}

// ------------------------------------------------------------------------------------------------
bool Color4::operator == (const Color4 & c) const
{
    return (r == c.r) && (g == c.g) && (b == c.b) && (a == c.a);
}

bool Color4::operator != (const Color4 & c) const
{
    return (r != c.r) && (g != c.g) && (b != c.b) && (a != c.a);
}

bool Color4::operator < (const Color4 & c) const
{
    return (r < c.r) && (g < c.g) && (b < c.b) && (a < c.a);
}

bool Color4::operator > (const Color4 & c) const
{
    return (r > c.r) && (g > c.g) && (b > c.b) && (a > c.a);
}

bool Color4::operator <= (const Color4 & c) const
{
    return (r <= c.r) && (g <= c.g) && (b <= c.b) && (a <= c.a);
}

bool Color4::operator >= (const Color4 & c) const
{
    return (r >= c.r) && (g >= c.g) && (b >= c.b) && (a >= c.a);
}

// ------------------------------------------------------------------------------------------------
Color4::operator Color3 () const
{
    return Color3(r, g, b);
}

// ------------------------------------------------------------------------------------------------
SQInteger Color4::Cmp(const Color4 & c) const
{
    return *this == c ? 0 : (*this > c ? 1 : -1);
}

// ------------------------------------------------------------------------------------------------
const SQChar * Color4::ToString() const
{
    return ToStringF("%u,%u,%u,%u", r, g, b, a);
}

// ------------------------------------------------------------------------------------------------
void Color4::Set(Value ns)
{
    r = ns;
    g = ns;
    b = ns;
    a = ns;
}

void Color4::Set(Value nr, Value ng, Value nb)
{
    r = nr;
    g = ng;
    b = nb;
}

void Color4::Set(Value nr, Value ng, Value nb, Value na)
{
    r = nr;
    g = ng;
    b = nb;
    a = na;
}

// ------------------------------------------------------------------------------------------------
void Color4::Set(const Color4 & c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
}

void Color4::Set(const Color3 & c)
{
    r = c.r;
    g = c.g;
    b = c.b;
    a = 0;
}

// ------------------------------------------------------------------------------------------------
void Color4::Set(const SQChar * str, SQChar delim)
{
    Set(GetColor4(str, delim));
}

// ------------------------------------------------------------------------------------------------
void Color4::SetCol(const SQChar * name)
{
    Set(GetColor(name));
}

// ------------------------------------------------------------------------------------------------
SQUint32 Color4::GetRGB() const
{
    return static_cast<SQUint32>(r << 16 | g << 8 | b);
}

void Color4::SetRGB(SQUint32 p)
{
    r = static_cast<Value>((p >> 16) & 0xFF);
    g = static_cast<Value>((p >> 8) & 0xFF);
    b = static_cast<Value>((p) & 0xFF);
}

// ------------------------------------------------------------------------------------------------
SQUint32 Color4::GetRGBA() const
{
    return static_cast<SQUint32>(r << 24 | g << 16 | b << 8 | a);
}

void Color4::SetRGBA(SQUint32 p)
{
    r = static_cast<Value>((p >> 24) & 0xFF);
    g = static_cast<Value>((p >> 16) & 0xFF);
    b = static_cast<Value>((p >> 8) & 0xFF);
    a = static_cast<Value>((p) & 0xFF);
}

// ------------------------------------------------------------------------------------------------
SQUint32 Color4::GetARGB() const
{
    return static_cast<SQUint32>(a << 24 | r << 16 | g << 8 | b);
}

void Color4::SetARGB(SQUint32 p)
{
    a = static_cast<Value>((p >> 24) & 0xFF);
    r = static_cast<Value>((p >> 16) & 0xFF);
    g = static_cast<Value>((p >> 8) & 0xFF);
    b = static_cast<Value>((p) & 0xFF);
}

// ------------------------------------------------------------------------------------------------
void Color4::Generate()
{
    r = RandomVal<Value>::Get();
    g = RandomVal<Value>::Get();
    b = RandomVal<Value>::Get();
    a = RandomVal<Value>::Get();
}

void Color4::Generate(Value min, Value max)
{
    if (max < min)
    {
        LogErr("max value is lower than min value");
    }
    else
    {
        r = RandomVal<Value>::Get(min, max);
        g = RandomVal<Value>::Get(min, max);
        b = RandomVal<Value>::Get(min, max);
        a = RandomVal<Value>::Get(min, max);
    }
}

void Color4::Generate(Value rmin, Value rmax, Value gmin, Value gmax, Value bmin, Value bmax, Value amin, Value amax)
{
    if (rmax < rmin || gmax < gmin || bmax < bmin || amax < amin)
    {
        LogErr("max value is lower than min value");
    }
    else
    {
        r = RandomVal<Value>::Get(rmin, rmax);
        g = RandomVal<Value>::Get(gmin, gmax);
        b = RandomVal<Value>::Get(bmin, bmax);
        a = RandomVal<Value>::Get(bmin, bmax);
    }
}

// ------------------------------------------------------------------------------------------------
void Color4::Random()
{
    Set(GetRandomColor());
}

// ------------------------------------------------------------------------------------------------
void Color4::Inverse()
{
    r = static_cast<Value>(~r);
    g = static_cast<Value>(~g);
    b = static_cast<Value>(~b);
    a = static_cast<Value>(~a);
}

// ================================================================================================
bool Register_Color4(HSQUIRRELVM vm)
{
    LogDbg("Beginning registration of <Color4> type");

    typedef Color4::Value Val;

    Sqrat::RootTable(vm).Bind(_SC("Color4"), Sqrat::Class<Color4>(vm, _SC("Color4"))
        .Ctor()
        .Ctor<Val>()
        .Ctor<Val, Val, Val>()
        .Ctor<Val, Val, Val, Val>()
        .Ctor<const SQChar *, SQChar>()

        .SetStaticValue(_SC("delim"), &Color4::Delim)

        .Var(_SC("r"), &Color4::r)
        .Var(_SC("g"), &Color4::g)
        .Var(_SC("b"), &Color4::b)
        .Var(_SC("a"), &Color4::a)

        .Prop(_SC("rgb"), &Color4::GetRGB, &Color4::SetRGB)
        .Prop(_SC("rgba"), &Color4::GetRGBA, &Color4::SetRGBA)
        .Prop(_SC("argb"), &Color4::GetARGB, &Color4::SetARGB)
        .Prop(_SC("str"), &Color4::SetCol)

        .Func(_SC("_tostring"), &Color4::ToString)
        .Func(_SC("_cmp"), &Color4::Cmp)

        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("_add"), &Color4::operator +)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("_sub"), &Color4::operator -)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("_mul"), &Color4::operator *)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("_div"), &Color4::operator /)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("_modulo"), &Color4::operator %)
        .Func<Color4 (Color4::*)(void) const>(_SC("_unm"), &Color4::operator -)

        .Overload<void (Color4::*)(Val)>(_SC("set"), &Color4::Set)
        .Overload<void (Color4::*)(Val, Val, Val)>(_SC("set"), &Color4::Set)
        .Overload<void (Color4::*)(Val, Val, Val, Val)>(_SC("set"), &Color4::Set)
        .Overload<void (Color4::*)(const Color4 &)>(_SC("set_col4"), &Color4::Set)
        .Overload<void (Color4::*)(const Color3 &)>(_SC("set_col3"), &Color4::Set)
        .Overload<void (Color4::*)(const SQChar *, SQChar)>(_SC("set_str"), &Color4::Set)

        .Overload<void (Color4::*)(void)>(_SC("generate"), &Color4::Generate)
        .Overload<void (Color4::*)(Val, Val)>(_SC("generate"), &Color4::Generate)
        .Overload<void (Color4::*)(Val, Val, Val, Val, Val, Val, Val, Val)>(_SC("generate"), &Color4::Generate)

        .Func(_SC("clear"), &Color4::Clear)
        .Func(_SC("random"), &Color4::Random)
        .Func(_SC("inverse"), &Color4::Inverse)

        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opAddAssign"), &Color4::operator +=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opSubAssign"), &Color4::operator -=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opMulAssign"), &Color4::operator *=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opDivAssign"), &Color4::operator /=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opModAssign"), &Color4::operator %=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opAndAssign"), &Color4::operator &=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opOrAssign"), &Color4::operator |=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opXorAssign"), &Color4::operator ^=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opShlAssign"), &Color4::operator <<=)
        .Func<Color4 & (Color4::*)(const Color4 &)>(_SC("opShrAssign"), &Color4::operator >>=)

        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opAddAssignS"), &Color4::operator +=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opSubAssignS"), &Color4::operator -=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opMulAssignS"), &Color4::operator *=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opDivAssignS"), &Color4::operator /=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opModAssignS"), &Color4::operator %=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opAndAssignS"), &Color4::operator &=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opOrAssignS"), &Color4::operator |=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opXorAssignS"), &Color4::operator ^=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opShlAssignS"), &Color4::operator <<=)
        .Func<Color4 & (Color4::*)(Color4::Value)>(_SC("opShrAssignS"), &Color4::operator >>=)

        .Func<Color4 & (Color4::*)(void)>(_SC("opPreInc"), &Color4::operator ++)
        .Func<Color4 & (Color4::*)(void)>(_SC("opPreDec"), &Color4::operator --)
        .Func<Color4 (Color4::*)(int)>(_SC("opPostInc"), &Color4::operator ++)
        .Func<Color4 (Color4::*)(int)>(_SC("opPostDec"), &Color4::operator --)

        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opAdd"), &Color4::operator +)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opSub"), &Color4::operator -)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opMul"), &Color4::operator *)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opDiv"), &Color4::operator /)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opMod"), &Color4::operator %)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opAnd"), &Color4::operator &)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opOr"), &Color4::operator |)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opShl"), &Color4::operator ^)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opShl"), &Color4::operator <<)
        .Func<Color4 (Color4::*)(const Color4 &) const>(_SC("opShr"), &Color4::operator >>)

        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opAddS"), &Color4::operator +)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opSubS"), &Color4::operator -)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opMulS"), &Color4::operator *)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opDivS"), &Color4::operator /)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opModS"), &Color4::operator %)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opAndS"), &Color4::operator &)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opOrS"), &Color4::operator |)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opShlS"), &Color4::operator ^)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opShlS"), &Color4::operator <<)
        .Func<Color4 (Color4::*)(Color4::Value) const>(_SC("opShrS"), &Color4::operator >>)

        .Func<Color4 (Color4::*)(void) const>(_SC("opUnPlus"), &Color4::operator +)
        .Func<Color4 (Color4::*)(void) const>(_SC("opUnMinus"), &Color4::operator -)
        .Func<Color4 (Color4::*)(void) const>(_SC("opCom"), &Color4::operator ~)

        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opEqual"), &Color4::operator ==)
        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opNotEqual"), &Color4::operator !=)
        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opLessThan"), &Color4::operator <)
        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opGreaterThan"), &Color4::operator >)
        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opLessEqual"), &Color4::operator <=)
        .Func<bool (Color4::*)(const Color4 &) const>(_SC("opGreaterEqual"), &Color4::operator >=)
    );

    LogDbg("Registration of <Color4> type was successful");

    return true;
}

} // Namespace:: SqMod
