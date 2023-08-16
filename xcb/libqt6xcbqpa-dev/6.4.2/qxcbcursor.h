// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBCURSOR_H
#define QXCBCURSOR_H

#include <qpa/qplatformcursor.h>
#include "qxcbscreen.h"

#include <QtCore/QCache>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_CURSOR

struct QXcbCursorCacheKey
{
    explicit QXcbCursorCacheKey(const QCursor &c);
    explicit QXcbCursorCacheKey(Qt::CursorShape s) : shape(s), bitmapCacheKey(0), maskCacheKey(0) {}
    QXcbCursorCacheKey() : shape(Qt::CustomCursor), bitmapCacheKey(0), maskCacheKey(0) {}

    Qt::CursorShape shape;
    qint64 bitmapCacheKey;
    qint64 maskCacheKey;
};

inline bool operator==(const QXcbCursorCacheKey &k1, const QXcbCursorCacheKey &k2)
{
    return k1.shape == k2.shape && k1.bitmapCacheKey == k2.bitmapCacheKey && k1.maskCacheKey == k2.maskCacheKey;
}

inline size_t qHash(const QXcbCursorCacheKey &k, size_t seed) noexcept
{
    return (size_t(k.shape) + size_t(k.bitmapCacheKey) + size_t(k.maskCacheKey)) ^ seed;
}

#endif // !QT_NO_CURSOR

class QXcbCursor : public QXcbObject, public QPlatformCursor
{
public:
    QXcbCursor(QXcbConnection *conn, QXcbScreen *screen);
    ~QXcbCursor();
#ifndef QT_NO_CURSOR
    void changeCursor(QCursor *cursor, QWindow *window) override;
#endif
    QPoint pos() const override;
    void setPos(const QPoint &pos) override;

    static void queryPointer(QXcbConnection *c, QXcbVirtualDesktop **virtualDesktop, QPoint *pos, int *keybMask = nullptr);

#ifndef QT_NO_CURSOR
    xcb_cursor_t xcbCursor(const QCursor &c) const
        { return m_cursorHash.value(QXcbCursorCacheKey(c), xcb_cursor_t(0)); }
#endif

private:

#ifndef QT_NO_CURSOR
    typedef QHash<QXcbCursorCacheKey, xcb_cursor_t> CursorHash;

    struct CachedCursor
    {
        explicit CachedCursor(xcb_connection_t *conn, xcb_cursor_t c)
            : cursor(c), connection(conn) {}
        ~CachedCursor() { xcb_free_cursor(connection, cursor); }
        xcb_cursor_t cursor;
        xcb_connection_t *connection;
    };
    typedef QCache<QXcbCursorCacheKey, CachedCursor> BitmapCursorCache;

    xcb_cursor_t createFontCursor(int cshape);
    xcb_cursor_t createBitmapCursor(QCursor *cursor);
    xcb_cursor_t createNonStandardCursor(int cshape);
#endif

    QXcbScreen *m_screen;
#ifndef QT_NO_CURSOR
    CursorHash m_cursorHash;
    BitmapCursorCache m_bitmapCache;
#endif
#if QT_CONFIG(xcb_xlib) && QT_CONFIG(library)
    static void cursorThemePropertyChanged(QXcbVirtualDesktop *screen,
                                           const QByteArray &name,
                                           const QVariant &property,
                                           void *handle);
#endif
    bool m_gtkCursorThemeInitialized;
};

QT_END_NAMESPACE

#endif
