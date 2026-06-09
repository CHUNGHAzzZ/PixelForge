#pragma once

#define PIXELFORGE_QML_PROPERTY_DETAIL_FULL(Type, PropertyName, PascalName, SetterArgumentType)        \
    Q_PROPERTY(Type PropertyName READ PropertyName WRITE set##PascalName NOTIFY PropertyName##Changed) \
    Type PropertyName() const;                                                                         \
    void set##PascalName(SetterArgumentType value);                                                    \
Q_SIGNALS:                                                                                            \
    void PropertyName##Changed();                                                                     \
public:

#define PIXELFORGE_QML_VALUE_PROPERTY(Type, PropertyName, PascalName)                                  \
    PIXELFORGE_QML_PROPERTY_DETAIL_FULL(                                                               \
        Type,                                                                                         \
        PropertyName,                                                                                 \
        PascalName,                                                                                   \
        Type)

#define PIXELFORGE_QML_QT_PROPERTY(Type, PropertyName, PascalName)                                     \
    PIXELFORGE_QML_PROPERTY_DETAIL_FULL(                                                               \
        Type,                                                                                         \
        PropertyName,                                                                                 \
        PascalName,                                                                                   \
        const Type &)

#define PIXELFORGE_QML_POINTER_PROPERTY(Type, PropertyName, PascalName)                                \
    PIXELFORGE_QML_PROPERTY_DETAIL_FULL(                                                               \
        Type *,                                                                                        \
        PropertyName,                                                                                 \
        PascalName,                                                                                   \
        Type *)
