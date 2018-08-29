/****************************************************************************
** Meta object code from reading C++ file 'doubleslider.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../doubleslider.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'doubleslider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DoubleSlider_t {
    QByteArrayData data[12];
    char stringdata0[131];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DoubleSlider_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DoubleSlider_t qt_meta_stringdata_DoubleSlider = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DoubleSlider"
QT_MOC_LITERAL(1, 13, 15), // "minValueChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 15), // "maxValueChanged"
QT_MOC_LITERAL(4, 46, 12), // "valueChanged"
QT_MOC_LITERAL(5, 59, 13), // "sliderPressed"
QT_MOC_LITERAL(6, 73, 14), // "sliderReleased"
QT_MOC_LITERAL(7, 88, 8), // "setLabel"
QT_MOC_LITERAL(8, 97, 5), // "label"
QT_MOC_LITERAL(9, 103, 11), // "setMaxValue"
QT_MOC_LITERAL(10, 115, 3), // "val"
QT_MOC_LITERAL(11, 119, 11) // "setMinValue"

    },
    "DoubleSlider\0minValueChanged\0\0"
    "maxValueChanged\0valueChanged\0sliderPressed\0"
    "sliderReleased\0setLabel\0label\0setMaxValue\0"
    "val\0setMinValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DoubleSlider[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       3,    1,   57,    2, 0x06 /* Public */,
       4,    1,   60,    2, 0x06 /* Public */,
       5,    0,   63,    2, 0x06 /* Public */,
       6,    0,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   65,    2, 0x0a /* Public */,
       9,    1,   68,    2, 0x0a /* Public */,
      11,    1,   71,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::ULong,    2,
    QMetaType::Void, QMetaType::ULong,    2,
    QMetaType::Void, QMetaType::ULong,    2,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::ULong,   10,
    QMetaType::Void, QMetaType::ULong,   10,

       0        // eod
};

void DoubleSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DoubleSlider *_t = static_cast<DoubleSlider *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->minValueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 1: _t->maxValueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 2: _t->valueChanged((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 3: _t->sliderPressed(); break;
        case 4: _t->sliderReleased(); break;
        case 5: _t->setLabel((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->setMaxValue((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        case 7: _t->setMinValue((*reinterpret_cast< ulong(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DoubleSlider::*_t)(unsigned long );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DoubleSlider::minValueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DoubleSlider::*_t)(unsigned long );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DoubleSlider::maxValueChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DoubleSlider::*_t)(unsigned long );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DoubleSlider::valueChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DoubleSlider::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DoubleSlider::sliderPressed)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (DoubleSlider::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DoubleSlider::sliderReleased)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject DoubleSlider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DoubleSlider.data,
      qt_meta_data_DoubleSlider,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DoubleSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DoubleSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DoubleSlider.stringdata0))
        return static_cast<void*>(const_cast< DoubleSlider*>(this));
    return QWidget::qt_metacast(_clname);
}

int DoubleSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DoubleSlider::minValueChanged(unsigned long _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DoubleSlider::maxValueChanged(unsigned long _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DoubleSlider::valueChanged(unsigned long _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DoubleSlider::sliderPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void DoubleSlider::sliderReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
