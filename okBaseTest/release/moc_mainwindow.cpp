/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_SensorDataObserver_t {
    QByteArrayData data[10];
    char stringdata0[123];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SensorDataObserver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SensorDataObserver_t qt_meta_stringdata_SensorDataObserver = {
    {
QT_MOC_LITERAL(0, 0, 18), // "SensorDataObserver"
QT_MOC_LITERAL(1, 19, 11), // "sgupdateImg"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 8), // "pBuffer1"
QT_MOC_LITERAL(4, 41, 8), // "pBuffer2"
QT_MOC_LITERAL(5, 50, 14), // "sgupdateBuffer"
QT_MOC_LITERAL(6, 65, 20), // "const unsigned char*"
QT_MOC_LITERAL(7, 86, 8), // "pBuffer3"
QT_MOC_LITERAL(8, 95, 15), // "onUpdateDisplay"
QT_MOC_LITERAL(9, 111, 11) // "onRectReady"

    },
    "SensorDataObserver\0sgupdateImg\0\0"
    "pBuffer1\0pBuffer2\0sgupdateBuffer\0"
    "const unsigned char*\0pBuffer3\0"
    "onUpdateDisplay\0onRectReady"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SensorDataObserver[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,
       5,    3,   44,    2, 0x06 /* Public */,
       5,    2,   51,    2, 0x26 /* Public | MethodCloned */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   56,    2, 0x09 /* Protected */,
       9,    2,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage, QMetaType::QImage,    3,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6, 0x80000000 | 6,    3,    4,    7,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    2,    2,

       0        // eod
};

void SensorDataObserver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SensorDataObserver *_t = static_cast<SensorDataObserver *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sgupdateImg((*reinterpret_cast< const QImage(*)>(_a[1])),(*reinterpret_cast< const QImage(*)>(_a[2]))); break;
        case 1: _t->sgupdateBuffer((*reinterpret_cast< const unsigned char*(*)>(_a[1])),(*reinterpret_cast< const unsigned char*(*)>(_a[2])),(*reinterpret_cast< const unsigned char*(*)>(_a[3]))); break;
        case 2: _t->sgupdateBuffer((*reinterpret_cast< const unsigned char*(*)>(_a[1])),(*reinterpret_cast< const unsigned char*(*)>(_a[2]))); break;
        case 3: _t->onUpdateDisplay(); break;
        case 4: _t->onRectReady((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SensorDataObserver::*_t)(const QImage & , const QImage & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SensorDataObserver::sgupdateImg)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (SensorDataObserver::*_t)(const unsigned char * , const unsigned char * , const unsigned char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SensorDataObserver::sgupdateBuffer)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject SensorDataObserver::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SensorDataObserver.data,
      qt_meta_data_SensorDataObserver,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SensorDataObserver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SensorDataObserver::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SensorDataObserver.stringdata0))
        return static_cast<void*>(const_cast< SensorDataObserver*>(this));
    if (!strcmp(_clname, "Observer"))
        return static_cast< Observer*>(const_cast< SensorDataObserver*>(this));
    return QWidget::qt_metacast(_clname);
}

int SensorDataObserver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SensorDataObserver::sgupdateImg(const QImage & _t1, const QImage & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SensorDataObserver::sgupdateBuffer(const unsigned char * _t1, const unsigned char * _t2, const unsigned char * _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[27];
    char stringdata0[419];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 18), // "onPipeoutDataTimer"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 15), // "onPlaybackTimer"
QT_MOC_LITERAL(4, 47, 15), // "onButtonClicked"
QT_MOC_LITERAL(5, 63, 5), // "index"
QT_MOC_LITERAL(6, 69, 14), // "onValueChanged"
QT_MOC_LITERAL(7, 84, 8), // "uint32_t"
QT_MOC_LITERAL(8, 93, 5), // "value"
QT_MOC_LITERAL(9, 99, 15), // "HHSliderWidget*"
QT_MOC_LITERAL(10, 115, 6), // "slider"
QT_MOC_LITERAL(11, 122, 17), // "onBtnPlayReleased"
QT_MOC_LITERAL(12, 140, 21), // "onBtnRotateLRReleased"
QT_MOC_LITERAL(13, 162, 21), // "onBtnRotateUDReleased"
QT_MOC_LITERAL(14, 184, 19), // "onBtnRepeatReleased"
QT_MOC_LITERAL(15, 204, 17), // "onBtnSaveReleased"
QT_MOC_LITERAL(16, 222, 20), // "onBtnSaveBmpReleased"
QT_MOC_LITERAL(17, 243, 16), // "onSetCurrentTime"
QT_MOC_LITERAL(18, 260, 20), // "onSliderValueChanged"
QT_MOC_LITERAL(19, 281, 15), // "onSliderPressed"
QT_MOC_LITERAL(20, 297, 16), // "onSliderReleased"
QT_MOC_LITERAL(21, 314, 17), // "onMinValueChanged"
QT_MOC_LITERAL(22, 332, 17), // "onMaxValueChanged"
QT_MOC_LITERAL(23, 350, 19), // "onSensorModeChanged"
QT_MOC_LITERAL(24, 370, 4), // "mode"
QT_MOC_LITERAL(25, 375, 21), // "onDisplayMode1Changed"
QT_MOC_LITERAL(26, 397, 21) // "onDisplayMode2Changed"

    },
    "MainWindow\0onPipeoutDataTimer\0\0"
    "onPlaybackTimer\0onButtonClicked\0index\0"
    "onValueChanged\0uint32_t\0value\0"
    "HHSliderWidget*\0slider\0onBtnPlayReleased\0"
    "onBtnRotateLRReleased\0onBtnRotateUDReleased\0"
    "onBtnRepeatReleased\0onBtnSaveReleased\0"
    "onBtnSaveBmpReleased\0onSetCurrentTime\0"
    "onSliderValueChanged\0onSliderPressed\0"
    "onSliderReleased\0onMinValueChanged\0"
    "onMaxValueChanged\0onSensorModeChanged\0"
    "mode\0onDisplayMode1Changed\0"
    "onDisplayMode2Changed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  109,    2, 0x0a /* Public */,
       3,    0,  110,    2, 0x0a /* Public */,
       4,    1,  111,    2, 0x0a /* Public */,
       6,    2,  114,    2, 0x0a /* Public */,
      11,    0,  119,    2, 0x0a /* Public */,
      12,    0,  120,    2, 0x0a /* Public */,
      13,    0,  121,    2, 0x0a /* Public */,
      14,    0,  122,    2, 0x0a /* Public */,
      15,    0,  123,    2, 0x0a /* Public */,
      16,    0,  124,    2, 0x0a /* Public */,
      17,    0,  125,    2, 0x0a /* Public */,
      18,    1,  126,    2, 0x0a /* Public */,
      19,    0,  129,    2, 0x0a /* Public */,
      20,    0,  130,    2, 0x0a /* Public */,
      21,    1,  131,    2, 0x0a /* Public */,
      22,    1,  134,    2, 0x0a /* Public */,
      23,    1,  137,    2, 0x0a /* Public */,
      25,    1,  140,    2, 0x0a /* Public */,
      26,    1,  143,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 9,    8,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::ULong,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::ULong,    8,
    QMetaType::Void, QMetaType::ULong,    8,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onPipeoutDataTimer(); break;
        case 1: _t->onPlaybackTimer(); break;
        case 2: _t->onButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onValueChanged((*reinterpret_cast< uint32_t(*)>(_a[1])),(*reinterpret_cast< HHSliderWidget*(*)>(_a[2]))); break;
        case 4: _t->onBtnPlayReleased(); break;
        case 5: _t->onBtnRotateLRReleased(); break;
        case 6: _t->onBtnRotateUDReleased(); break;
        case 7: _t->onBtnRepeatReleased(); break;
        case 8: _t->onBtnSaveReleased(); break;
        case 9: _t->onBtnSaveBmpReleased(); break;
        case 10: _t->onSetCurrentTime(); break;
        case 11: _t->onSliderValueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 12: _t->onSliderPressed(); break;
        case 13: _t->onSliderReleased(); break;
        case 14: _t->onMinValueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 15: _t->onMaxValueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 16: _t->onSensorModeChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 17: _t->onDisplayMode1Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->onDisplayMode2Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
