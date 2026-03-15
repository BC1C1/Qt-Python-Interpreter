#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <QSharedPointer>

// utils begin
template <typename T, typename... Args>
QSharedPointer<T> makeShared(Args&&... args) {
    return QSharedPointer<T>::create(std::forward<Args>(args)...);
}
template <typename Target, typename Source>
QSharedPointer<Target> dynamicPointerCast(const QSharedPointer<Source>& source)
{
    if (source.isNull()) {
        return QSharedPointer<Target>();
    }

    Target* rawTarget = qobject_cast<Target*>(source.data());
    if (rawTarget == nullptr) {
        return QSharedPointer<Target>();
    }

    return QSharedPointer<Target>(source, rawTarget); // 这里的构造注意
}
// string cast. UTF-8 is the only storage type
inline std::string qt2std(const QString& qStr)
{
    QByteArray utf8Bytes = qStr.toUtf8();
    return std::string(utf8Bytes.constData(), utf8Bytes.size());
}

inline QString std2qt(const std::string& sStr)
{
    return QString::fromUtf8(sStr.data(), sStr.size());
}
// utils end

#endif // FUNCTIONS_H
