/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "quuid.h"

#include "qdatastream.h"
#include "qendian.h"
#include "qdebug.h"

#ifndef QT_BOOTSTRAPPED
#include "qcryptographichash.h"
#endif
QT_BEGIN_NAMESPACE

#ifndef QT_NO_QUUID_STRING
template <class Char, class Integral>
void _q_toHex(Char *&dst, Integral value)
{
    static const char digits[] = "0123456789abcdef";

    value = qToBigEndian(value);

    const char* p = reinterpret_cast<const char*>(&value);

    for (uint i = 0; i < sizeof(Integral); ++i, dst += 2) {
        uint j = (p[i] >> 4) & 0xf;
        dst[0] = Char(digits[j]);
        j = p[i] & 0xf;
        dst[1] = Char(digits[j]);
    }
}

template <class Char, class Integral>
bool _q_fromHex(const Char *&src, Integral &value)
{
    value = 0;

    for (uint i = 0; i < sizeof(Integral) * 2; ++i) {
        int ch = *src++;
        int tmp;
        if (ch >= '0' && ch <= '9')
            tmp = ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            tmp = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            tmp = ch - 'A' + 10;
        else
            return false;

        value = value * 16 + tmp;
    }

    return true;
}

template <class Char>
void _q_uuidToHex(Char *&dst, const uint &d1, const ushort &d2, const ushort &d3, const uchar (&d4)[8])
{
    *dst++ = Char('{');
    _q_toHex(dst, d1);
    *dst++ = Char('-');
    _q_toHex(dst, d2);
    *dst++ = Char('-');
    _q_toHex(dst, d3);
    *dst++ = Char('-');
    for (int i = 0; i < 2; i++)
        _q_toHex(dst, d4[i]);
    *dst++ = Char('-');
    for (int i = 2; i < 8; i++)
        _q_toHex(dst, d4[i]);
    *dst = Char('}');
}

template <class Char>
bool _q_uuidFromHex(const Char *&src, uint &d1, ushort &d2, ushort &d3, uchar (&d4)[8])
{
    if (*src == Char('{'))
        src++;
    if (!_q_fromHex(src, d1)
            || *src++ != Char('-')
            || !_q_fromHex(src, d2)
            || *src++ != Char('-')
            || !_q_fromHex(src, d3)
            || *src++ != Char('-')
            || !_q_fromHex(src, d4[0])
            || !_q_fromHex(src, d4[1])
            || *src++ != Char('-')
            || !_q_fromHex(src, d4[2])
            || !_q_fromHex(src, d4[3])
            || !_q_fromHex(src, d4[4])
            || !_q_fromHex(src, d4[5])
            || !_q_fromHex(src, d4[6])
            || !_q_fromHex(src, d4[7])) {
        return false;
    }

    return true;
}
#endif

#ifndef QT_BOOTSTRAPPED
static QUuid createFromName(const QUuid &ns, const QByteArray &baseData, QCryptographicHash::Algorithm algorithm, int version)
{
    QByteArray hashResult;

    // create a scope so later resize won't reallocate
    {
        QCryptographicHash hash(algorithm);
        hash.addData(ns.toRfc4122());
        hash.addData(baseData);
        hashResult = hash.result();
    }
    hashResult.resize(16); // Sha1 will be too long

    QUuid result = QUuid::fromRfc4122(hashResult);

    result.data3 &= 0x0FFF;
    result.data3 |= (version << 12);
    result.data4[0] &= 0x3F;
    result.data4[0] |= 0x80;

    return result;
}
#endif

/*!
    \class QUuid
    \brief The QUuid class stores a Universally Unique Identifier (UUID).

    \reentrant

    Using \e{U}niversally \e{U}nique \e{ID}entifiers (UUID) is a
    standard way to uniquely identify entities in a distributed
    computing environment. A UUID is a 16-byte (128-bit) number
    generated by some algorithm that is meant to guarantee that the
    UUID will be unique in the distributed computing environment where
    it is used. The acronym GUID is often used instead, \e{G}lobally
    \e{U}nique \e{ID}entifiers, but it refers to the same thing.

    \target Variant field
    Actually, the GUID is one \e{variant} of UUID. Multiple variants
    are in use. Each UUID contains a bit field that specifies which
    type (variant) of UUID it is. Call variant() to discover which
    type of UUID an instance of QUuid contains. It extracts the three
    most signifcant bits of byte 8 of the 16 bytes. In QUuid, byte 8
    is \c{QUuid::data4[0]}. If you create instances of QUuid using the
    constructor that accepts all the numeric values as parameters, use
    the following table to set the three most significant bits of
    parameter \c{b1}, which becomes \c{QUuid::data4[0]} and contains
    the variant field in its three most significant bits. In the
    table, 'x' means \e {don't care}.

    \table
    \header
    \li msb0
    \li msb1
    \li msb2
    \li Variant

    \row
    \li 0
    \li x
    \li x
    \li NCS (Network Computing System)

    \row
    \li 1
    \li 0
    \li x
    \li DCE (Distributed Computing Environment)

    \row
    \li 1
    \li 1
    \li 0
    \li Microsoft (GUID)

    \row
    \li 1
    \li 1
    \li 1
    \li Reserved for future expansion

    \endtable

    \target Version field
    If variant() returns QUuid::DCE, the UUID also contains a
    \e{version} field in the four most significant bits of
    \c{QUuid::data3}, and you can call version() to discover which
    version your QUuid contains. If you create instances of QUuid
    using the constructor that accepts all the numeric values as
    parameters, use the following table to set the four most
    significant bits of parameter \c{w2}, which becomes
    \c{QUuid::data3} and contains the version field in its four most
    significant bits.

    \table
    \header
    \li msb0
    \li msb1
    \li msb2
    \li msb3
    \li Version

    \row
    \li 0
    \li 0
    \li 0
    \li 1
    \li Time

    \row
    \li 0
    \li 0
    \li 1
    \li 0
    \li Embedded POSIX

    \row
    \li 0
    \li 0
    \li 1
    \li 1
    \li Md5(Name)

    \row
    \li 0
    \li 1
    \li 0
    \li 0
    \li Random

    \row
    \li 0
    \li 1
    \li 0
    \li 1
    \li Sha1

    \endtable

    The field layouts for the DCE versions listed in the table above
    are specified in the \l{http://www.ietf.org/rfc/rfc4122.txt}
    {Network Working Group UUID Specification}.
    
    Most platforms provide a tool for generating new UUIDs, e.g. \c
    uuidgen and \c guidgen. You can also use createUuid().  UUIDs
    generated by createUuid() are of the random type.  Their
    QUuid::Version bits are set to QUuid::Random, and their
    QUuid::Variant bits are set to QUuid::DCE. The rest of the UUID is
    composed of random numbers. Theoretically, this means there is a
    small chance that a UUID generated by createUuid() will not be
    unique. But it is
    \l{http://en.wikipedia.org/wiki/Universally_Unique_Identifier#Random_UUID_probability_of_duplicates}
    {a \e{very} small chance}.

    UUIDs can be constructed from numeric values or from strings, or
    using the static createUuid() function. They can be converted to a
    string with toString(). UUIDs have a variant() and a version(),
    and null UUIDs return true from isNull().
*/

/*!
    \fn QUuid::QUuid(const GUID &guid)

    Casts a Windows \a guid to a Qt QUuid.

    \warning This function is only for Windows platforms.
*/

/*!
    \fn QUuid &QUuid::operator=(const GUID &guid)

    Assigns a Windows \a guid to a Qt QUuid.

    \warning This function is only for Windows platforms.
*/

/*!
    \fn QUuid::operator GUID() const

    Returns a Windows GUID from a QUuid.

    \warning This function is only for Windows platforms.
*/

/*!
    \fn QUuid::QUuid()

    Creates the null UUID. toString() will output the null UUID
    as "{00000000-0000-0000-0000-000000000000}".
*/

/*!
    \fn QUuid::QUuid(uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8)

    Creates a UUID with the value specified by the parameters, \a l,
    \a w1, \a w2, \a b1, \a b2, \a b3, \a b4, \a b5, \a b6, \a b7, \a
    b8.

    Example:
    \snippet code/src_corelib_plugin_quuid.cpp 0
*/

#ifndef QT_NO_QUUID_STRING
/*!
  Creates a QUuid object from the string \a text, which must be
  formatted as five hex fields separated by '-', e.g.,
  "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" where 'x' is a hex
  digit. The curly braces shown here are optional, but it is normal to
  include them. If the conversion fails, a null UUID is created.  See
  toString() for an explanation of how the five hex fields map to the
  public data members in QUuid.

    \sa toString(), QUuid()
*/
QUuid::QUuid(const QString &text)
{
    if (text.length() < 36) {
        *this = QUuid();
        return;
    }

    const ushort *data = reinterpret_cast<const ushort *>(text.unicode());

    if (*data == '{' && text.length() < 37) {
        *this = QUuid();
        return;
    }

    if (!_q_uuidFromHex(data, data1, data2, data3, data4)) {
        *this = QUuid();
        return;
    }
}

/*!
    \internal
*/
QUuid::QUuid(const char *text)
{
    if (!text) {
        *this = QUuid();
        return;
    }

    if (!_q_uuidFromHex(text, data1, data2, data3, data4)) {
        *this = QUuid();
        return;
    }
}

/*!
  Creates a QUuid object from the QByteArray \a text, which must be
  formatted as five hex fields separated by '-', e.g.,
  "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" where 'x' is a hex
  digit. The curly braces shown here are optional, but it is normal to
  include them. If the conversion fails, a null UUID is created.  See
  toByteArray() for an explanation of how the five hex fields map to the
  public data members in QUuid.

    \since 4.8

    \sa toByteArray(), QUuid()
*/
QUuid::QUuid(const QByteArray &text)
{
    if (text.length() < 36) {
        *this = QUuid();
        return;
    }

    const char *data = text.constData();

    if (*data == '{' && text.length() < 37) {
        *this = QUuid();
        return;
    }

    if (!_q_uuidFromHex(data, data1, data2, data3, data4)) {
        *this = QUuid();
        return;
    }
}

#endif

/*!
  \since 5.0
  \fn QUuid QUuid::createUuidV3(const QUuid &ns, const QByteArray &baseData);

  This functions returns a new UUID with variant QUuid::DCE and version QUuid::MD5.
  \a ns is the namespace and \a name is the name as described by RFC 4122.

  \sa variant(), version(), createUuidV5()
*/

/*!
  \since 5.0
  \fn QUuid QUuid::createUuidV5(const QUuid &ns, const QByteArray &baseData);

  This functions returns a new UUID with variant QUuid::DCE and version QUuid::SHA1.
  \a ns is the namespace and \a name is the name as described by RFC 4122.

  \sa variant(), version(), createUuidV3()
*/
#ifndef QT_BOOTSTRAPPED
QUuid QUuid::createUuidV3(const QUuid &ns, const QByteArray &baseData)
{
    return createFromName(ns, baseData, QCryptographicHash::Md5, 3);
}

QUuid QUuid::createUuidV5(const QUuid &ns, const QByteArray &baseData)
{
    return createFromName(ns, baseData, QCryptographicHash::Sha1, 5);
}
#endif

/*!
  Creates a QUuid object from the binary representation of the UUID, as
  specified by RFC 4122 section 4.1.2. See toRfc4122() for a further
  explanation of the order of bytes required.

  The byte array accepted is NOT a human readable format.

  If the conversion fails, a null UUID is created.

    \since 4.8

    \sa toRfc4122(), QUuid()
*/
QUuid QUuid::fromRfc4122(const QByteArray &bytes)
{
    if (bytes.isEmpty() || bytes.length() != 16)
        return QUuid();

    uint d1;
    ushort d2, d3;
    uchar d4[8];

    const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());

    d1 = qFromBigEndian<quint32>(data);
    data += sizeof(quint32);
    d2 = qFromBigEndian<quint16>(data);
    data += sizeof(quint16);
    d3 = qFromBigEndian<quint16>(data);
    data += sizeof(quint16);

    for (int i = 0; i < 8; ++i) {
        d4[i] = *(data);
        data++;
    }

    return QUuid(d1, d2, d3, d4[0], d4[1], d4[2], d4[3], d4[4], d4[5], d4[6], d4[7]);
}

/*!
    \fn bool QUuid::operator==(const QUuid &other) const

    Returns true if this QUuid and the \a other QUuid are identical;
    otherwise returns false.
*/

/*!
    \fn bool QUuid::operator!=(const QUuid &other) const

    Returns true if this QUuid and the \a other QUuid are different;
    otherwise returns false.
*/
#ifndef QT_NO_QUUID_STRING
/*!
    Returns the string representation of this QUuid. The string is
    formatted as five hex fields separated by '-' and enclosed in
    curly braces, i.e., "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" where
    'x' is a hex digit.  From left to right, the five hex fields are
    obtained from the four public data members in QUuid as follows:

    \table
    \header
    \li Field #
    \li Source
    
    \row
    \li 1
    \li data1
    
    \row
    \li 2
    \li data2
    
    \row
    \li 3
    \li data3
    
    \row
    \li 4
    \li data4[0] .. data4[1]
    
    \row
    \li 5
    \li data4[2] .. data4[7]

    \endtable
*/
QString QUuid::toString() const
{
    QString result(38, Qt::Uninitialized);
    ushort *data = (ushort *)result.unicode();

    _q_uuidToHex(data, data1, data2, data3, data4);

    return result;
}

/*!
    Returns the binary representation of this QUuid. The byte array is
    formatted as five hex fields separated by '-' and enclosed in
    curly braces, i.e., "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" where
    'x' is a hex digit.  From left to right, the five hex fields are
    obtained from the four public data members in QUuid as follows:

    \table
    \header
    \li Field #
    \li Source

    \row
    \li 1
    \li data1

    \row
    \li 2
    \li data2

    \row
    \li 3
    \li data3

    \row
    \li 4
    \li data4[0] .. data4[1]

    \row
    \li 5
    \li data4[2] .. data4[7]

    \endtable

    \since 4.8
*/
QByteArray QUuid::toByteArray() const
{
    QByteArray result(38, Qt::Uninitialized);
    char *data = result.data();

    _q_uuidToHex(data, data1, data2, data3, data4);

    return result;
}
#endif

/*!
    Returns the binary representation of this QUuid. The byte array is in big
    endian format, and formatted according to RFC 4122, section 4.1.2 -
    "Layout and byte order".

    The order is as follows:

    \table
    \header
    \li Field #
    \li Source

    \row
    \li 1
    \li data1

    \row
    \li 2
    \li data2

    \row
    \li 3
    \li data3

    \row
    \li 4
    \li data4[0] .. data4[7]

    \endtable

    \since 4.8
*/
QByteArray QUuid::toRfc4122() const
{
    // we know how many bytes a UUID has, I hope :)
    QByteArray bytes(16, Qt::Uninitialized);
    uchar *data = reinterpret_cast<uchar*>(bytes.data());

    qToBigEndian(data1, data);
    data += sizeof(quint32);
    qToBigEndian(data2, data);
    data += sizeof(quint16);
    qToBigEndian(data3, data);
    data += sizeof(quint16);

    for (int i = 0; i < 8; ++i) {
        *(data) = data4[i];
        data++;
    }

    return bytes;
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QUuid
    Writes the UUID \a id to the data stream \a s.
*/
QDataStream &operator<<(QDataStream &s, const QUuid &id)
{
    QByteArray bytes;
    if (s.byteOrder() == QDataStream::BigEndian) {
        bytes = id.toRfc4122();
    } else {
        // we know how many bytes a UUID has, I hope :)
        bytes = QByteArray(16, Qt::Uninitialized);
        uchar *data = reinterpret_cast<uchar*>(bytes.data());

        qToLittleEndian(id.data1, data);
        data += sizeof(quint32);
        qToLittleEndian(id.data2, data);
        data += sizeof(quint16);
        qToLittleEndian(id.data3, data);
        data += sizeof(quint16);

        for (int i = 0; i < 8; ++i) {
            *(data) = id.data4[i];
            data++;
        }
    }

    if (s.writeRawData(bytes.data(), 16) != 16) {
        s.setStatus(QDataStream::WriteFailed);
    }
    return s;
}

/*!
    \relates QUuid
    Reads a UUID from the stream \a s into \a id.
*/
QDataStream &operator>>(QDataStream &s, QUuid &id)
{
    QByteArray bytes(16, Qt::Uninitialized);
    if (s.readRawData(bytes.data(), 16) != 16) {
        s.setStatus(QDataStream::ReadPastEnd);
        return s;
    }

    if (s.byteOrder() == QDataStream::BigEndian) {
        id = QUuid::fromRfc4122(bytes);
    } else {
        const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());

        id.data1 = qFromLittleEndian<quint32>(data);
        data += sizeof(quint32);
        id.data2 = qFromLittleEndian<quint16>(data);
        data += sizeof(quint16);
        id.data3 = qFromLittleEndian<quint16>(data);
        data += sizeof(quint16);

        for (int i = 0; i < 8; ++i) {
            id.data4[i] = *(data);
            data++;
        }
    }

    return s;
}
#endif // QT_NO_DATASTREAM

/*!
    Returns true if this is the null UUID
    {00000000-0000-0000-0000-000000000000}; otherwise returns false.
*/
bool QUuid::isNull() const
{
    return data4[0] == 0 && data4[1] == 0 && data4[2] == 0 && data4[3] == 0 &&
           data4[4] == 0 && data4[5] == 0 && data4[6] == 0 && data4[7] == 0 &&
           data1 == 0 && data2 == 0 && data3 == 0;
}

/*!
    \enum QUuid::Variant

    This enum defines the values used in the \l{Variant field}
    {variant field} of the UUID. The value in the variant field
    determines the layout of the 128-bit value.

    \value VarUnknown Variant is unknown
    \value NCS Reserved for NCS (Network Computing System) backward compatibility
    \value DCE Distributed Computing Environment, the scheme used by QUuid
    \value Microsoft Reserved for Microsoft backward compatibility (GUID)
    \value Reserved Reserved for future definition
*/

/*!
    \enum QUuid::Version

    This enum defines the values used in the \l{Version field}
    {version field} of the UUID. The version field is meaningful
    only if the value in the \l{Variant field} {variant field}
    is QUuid::DCE.

    \value VerUnknown Version is unknown
    \value Time Time-based, by using timestamp, clock sequence, and
    MAC network card address (if available) for the node sections
    \value EmbeddedPOSIX DCE Security version, with embedded POSIX UUIDs
    \value Name Name-based, by using values from a name for all sections
    \value Random Random-based, by using random numbers for all sections
*/

/*!
    \fn QUuid::Variant QUuid::variant() const

    Returns the value in the \l{Variant field} {variant field} of the
    UUID. If the return value is QUuid::DCE, call version() to see
    which layout it uses. The null UUID is considered to be of an
    unknown variant.

    \sa version()
*/
QUuid::Variant QUuid::variant() const
{
    if (isNull())
        return VarUnknown;
    // Check the 3 MSB of data4[0]
    if ((data4[0] & 0x80) == 0x00) return NCS;
    else if ((data4[0] & 0xC0) == 0x80) return DCE;
    else if ((data4[0] & 0xE0) == 0xC0) return Microsoft;
    else if ((data4[0] & 0xE0) == 0xE0) return Reserved;
    return VarUnknown;
}

/*!
    \fn QUuid::Version QUuid::version() const

    Returns the \l{Version field} {version field} of the UUID, if the
    UUID's \l{Variant field} {variant field} is QUuid::DCE. Otherwise
    it returns QUuid::VerUnknown.

    \sa variant()
*/
QUuid::Version QUuid::version() const
{
    // Check the 4 MSB of data3
    Version ver = (Version)(data3>>12);
    if (isNull()
         || (variant() != DCE)
         || ver < Time
         || ver > Sha1)
        return VerUnknown;
    return ver;
}

/*!
    \fn bool QUuid::operator<(const QUuid &other) const

    Returns true if this QUuid has the same \l{Variant field}
    {variant field} as the \a other QUuid and is lexicographically
    \e{before} the \a other QUuid. If the \a other QUuid has a
    different variant field, the return value is determined by
    comparing the two \l{QUuid::Variant} {variants}.

    \sa variant()
*/
#define ISLESS(f1, f2) if (f1!=f2) return (f1<f2);
bool QUuid::operator<(const QUuid &other) const
{
    if (variant() != other.variant())
        return variant() < other.variant();

    ISLESS(data1, other.data1);
    ISLESS(data2, other.data2);
    ISLESS(data3, other.data3);
    for (int n = 0; n < 8; n++) {
        ISLESS(data4[n], other.data4[n]);
    }
    return false;
}

/*!
    \fn bool QUuid::operator>(const QUuid &other) const

    Returns true if this QUuid has the same \l{Variant field}
    {variant field} as the \a other QUuid and is lexicographically
    \e{after} the \a other QUuid. If the \a other QUuid has a
    different variant field, the return value is determined by
    comparing the two \l{QUuid::Variant} {variants}.

    \sa variant()
*/
#define ISMORE(f1, f2) if (f1!=f2) return (f1>f2);
bool QUuid::operator>(const QUuid &other) const
{
    if (variant() != other.variant())
        return variant() > other.variant();

    ISMORE(data1, other.data1);
    ISMORE(data2, other.data2);
    ISMORE(data3, other.data3);
    for (int n = 0; n < 8; n++) {
        ISMORE(data4[n], other.data4[n]);
    }
    return false;
}

/*!
    \fn QUuid QUuid::createUuid()

    On any platform other than Windows, this function returns a new
    UUID with variant QUuid::DCE and version QUuid::Random.  If
    the /dev/urandom device exists, then the numbers used to construct
    the UUID will be of cryptographic quality, which will make the UUID
    unique.  Otherwise, the numbers of the UUID will be obtained from
    the local pseudo-random number generator (qrand(), which is seeded
    by qsrand()) which is usually not of cryptograhic quality, which
    means that the UUID can't be guaranteed to be unique.

    On a Windows platform, a GUID is generated, which almost certainly
    \e{will} be unique, on this or any other system, networked or not.

    \sa variant(), version()
*/
#if defined(Q_OS_WIN32)

QT_BEGIN_INCLUDE_NAMESPACE
#include <objbase.h> // For CoCreateGuid
QT_END_INCLUDE_NAMESPACE

QUuid QUuid::createUuid()
{
    GUID guid;
    CoCreateGuid(&guid);
    QUuid result = guid;
    return result;
}

#else // !Q_OS_WIN32

QT_BEGIN_INCLUDE_NAMESPACE
#include "qdatetime.h"
#include "qfile.h"
#include "qthreadstorage.h"
#include <stdlib.h> // for RAND_MAX
QT_END_INCLUDE_NAMESPACE

#if !defined(QT_BOOTSTRAPPED) && defined(Q_OS_UNIX)
Q_GLOBAL_STATIC(QThreadStorage<QFile *>, devUrandomStorage);
#endif

QUuid QUuid::createUuid()
{
    QUuid result;
    uint *data = &(result.data1);

#if defined(Q_OS_UNIX)
    QFile *devUrandom;
#  if !defined(QT_BOOTSTRAPPED)
    devUrandom = devUrandomStorage()->localData();
    if (!devUrandom) {
        devUrandom = new QFile(QLatin1String("/dev/urandom"));
        devUrandom->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        devUrandomStorage()->setLocalData(devUrandom);
    }
# else
    QFile file(QLatin1String("/dev/urandom"));
    devUrandom = &file;
    devUrandom->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
# endif
    enum { AmountToRead = 4 * sizeof(uint) };
    if (devUrandom->isOpen()
        && devUrandom->read((char *) data, AmountToRead) == AmountToRead) {
        // we got what we wanted, nothing more to do
        ;
    } else
#endif
    {
        static const int intbits = sizeof(int)*8;
        static int randbits = 0;
        if (!randbits) {
            int r = 0;
            int max = RAND_MAX;
            do { ++r; } while ((max=max>>1));
            randbits = r;
        }

        // Seed the PRNG once per thread with a combination of current time, a
        // stack address and a serial counter (since thread stack addresses are
        // re-used).
#ifndef QT_BOOTSTRAPPED
        static QThreadStorage<int *> uuidseed;
        if (!uuidseed.hasLocalData())
        {
            int *pseed = new int;
            static QBasicAtomicInt serial = Q_BASIC_ATOMIC_INITIALIZER(2);
            qsrand(*pseed = QDateTime::currentDateTime().toTime_t()
                   + quintptr(&pseed)
                   + serial.fetchAndAddRelaxed(1));
            uuidseed.setLocalData(pseed);
        }
#else
        static bool seeded = false;
        if (!seeded)
            qsrand(QDateTime::currentDateTime().toTime_t()
                   + quintptr(&seeded));
#endif

        int chunks = 16 / sizeof(uint);
        while (chunks--) {
            uint randNumber = 0;
            for (int filled = 0; filled < intbits; filled += randbits)
                randNumber |= qrand()<<filled;
            *(data+chunks) = randNumber;
        }
    }

    result.data4[0] = (result.data4[0] & 0x3F) | 0x80;        // UV_DCE
    result.data3 = (result.data3 & 0x0FFF) | 0x4000;        // UV_Random

    return result;
}
#endif // !Q_OS_WIN32

/*!
    \fn bool QUuid::operator==(const GUID &guid) const

    Returns true if this UUID is equal to the Windows GUID \a guid;
    otherwise returns false.
*/

/*!
    \fn bool QUuid::operator!=(const GUID &guid) const

    Returns true if this UUID is not equal to the Windows GUID \a
    guid; otherwise returns false.
*/

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QUuid &id)
{
#ifndef QT_NO_QUUID_STRING
    dbg.nospace() << "QUuid(" << id.toString() << ')';
#else
    Q_UNUSED(id)
    dbg.nospace() << "QUuid(QT_NO_QUUID_STRING)";
#endif
    return dbg.space();
}
#endif

/*!
    \since 5.0
    \relates QUuid
    Returns a hash of the UUID \a uuid, using \a seed to seed the calculation.
*/
uint qHash(const QUuid &uuid, uint seed)
{
    return uuid.data1 ^ uuid.data2 ^ (uuid.data3 << 16)
            ^ ((uuid.data4[0] << 24) | (uuid.data4[1] << 16) | (uuid.data4[2] << 8) | uuid.data4[3])
            ^ ((uuid.data4[4] << 24) | (uuid.data4[5] << 16) | (uuid.data4[6] << 8) | uuid.data4[7])
            ^ seed;
}


QT_END_NAMESPACE
