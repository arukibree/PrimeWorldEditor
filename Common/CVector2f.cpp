#include "CVector2f.h"

CVector2f::CVector2f()
{
    x = y = 0.f;
}

CVector2f::CVector2f(float xy)
{
    x = y = xy;
}

CVector2f::CVector2f(float _x, float _y)
{
    x = _x;
    y = _y;
}

CVector2f::CVector2f(CInputStream& Input)
{
    x = Input.ReadFloat();
    y = Input.ReadFloat();
}

void CVector2f::Write(COutputStream &Output)
{
    Output.WriteFloat(x);
    Output.WriteFloat(y);
}

CVector2f CVector2f::operator+(const CVector2f& src) const
{
    CVector2f out;
    out.x = this->x + src.x;
    out.y = this->y + src.y;
    return out;
}

CVector2f CVector2f::operator-(const CVector2f& src) const
{
    CVector2f out;
    out.x = this->x - src.x;
    out.y = this->y - src.y;
    return out;
}

CVector2f CVector2f::operator*(const CVector2f& src) const
{
    CVector2f out;
    out.x = this->x * src.x;
    out.y = this->y * src.y;
    return out;
}

CVector2f CVector2f::operator/(const CVector2f& src) const
{
    CVector2f out;
    out.x = this->x / src.x;
    out.y = this->y / src.y;
    return out;
}

void CVector2f::operator+=(const CVector2f& other)
{
    *this = *this + other;
}

void CVector2f::operator-=(const CVector2f& other)
{
    *this = *this - other;
}

void CVector2f::operator*=(const CVector2f& other)
{
    *this = *this * other;
}

void CVector2f::operator/=(const CVector2f& other)
{
    *this = *this / other;
}

CVector2f CVector2f::operator+(const float src) const
{
    CVector2f out;
    out.x = this->x + src;
    out.y = this->y + src;
    return out;
}

CVector2f CVector2f::operator-(const float src) const
{
    CVector2f out;
    out.x = this->x - src;
    out.y = this->y - src;
    return out;
}

CVector2f CVector2f::operator*(const float src) const
{
    CVector2f out;
    out.x = this->x * src;
    out.y = this->y * src;
    return out;
}

CVector2f CVector2f::operator/(const float src) const
{
    CVector2f out;
    out.x = this->x / src;
    out.y = this->y / src;
    return out;
}

void CVector2f::operator+=(const float other)
{
    *this = *this + other;
}

void CVector2f::operator-=(const float other)
{
    *this = *this - other;
}

void CVector2f::operator*=(const float other)
{
    *this = *this * other;
}

void CVector2f::operator/=(const float other)
{
    *this = *this / other;
}

bool CVector2f::operator==(const CVector2f& other) const
{
    return ((x == other.x) && (y == other.y));
}

bool CVector2f::operator!=(const CVector2f& other) const
{
    return (!(*this == other));
}

float& CVector2f::operator[](long index)
{
    return (&x)[index];
}

// ************ STATIC MEMBER INITIALIZATION ************
const CVector2f CVector2f::skZero  = CVector2f(0, 0);
const CVector2f CVector2f::skOne   = CVector2f(1, 1);
const CVector2f CVector2f::skUp    = CVector2f(0, 1);
const CVector2f CVector2f::skRight = CVector2f(1, 0);
const CVector2f CVector2f::skDown  = CVector2f(0,-1);
const CVector2f CVector2f::skLeft  = CVector2f(-1,0);
