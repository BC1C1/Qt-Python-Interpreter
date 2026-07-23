#pragma once
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <qqueue.h>
#include <qmutex.h>
#include <string>
#include <QString>
#include <QByteArray>
#include <qdatetime.h>

#define DEBUG_MODE
#define PRINT_MOD
#define INFO_MOD
#define ERROR_MOD


// string cast. UTF-8 is the only storage type
inline std::string qt2std(const QString& qStr)
{
    QByteArray utf8Bytes = qStr.toUtf8();
    return std::string(utf8Bytes.constData(), utf8Bytes.size());
}

inline QString std2qt(const std::string& sStr)
{
    return QString::fromUtf8(sStr.data(), static_cast<int>(sStr.size()));
}

namespace myStd {
    enum class LogLevel {
        INFO_ = 0,
        WARNING_,
        ERROR_,
        DEBUG_
    };
    struct info {
        info(const std::string& text = "", LogLevel level = LogLevel::DEBUG_, const QString& timeStamp = QString()) 
            : text(text), level(level), timeStamp(timeStamp)
        {}
        std::string text; 
        LogLevel level;
        QString timeStamp;
    };

    class LoggerQueue {
    public:
        void push(const info& newInfo) {
            QMutexLocker lock(&mtx);
            queue.enqueue(newInfo);
        }

        void pop(info& out_info) {
            QMutexLocker lock(&mtx);
            if (queue.isEmpty()) return;
            out_info = queue.head();
            queue.dequeue();
        }

        bool isEmpty() {
            QMutexLocker lock(&mtx);
            return queue.isEmpty();
        }

    private:
        QQueue<info> queue;
        QMutex mtx;
    };
    using Outputer_global = void*;
    extern Outputer_global outputer_global;
    using OutputCallBack = void(*)(const info&);
    static void printf_f(const info& data) {
        if (!outputer_global) return;
        fprintf((FILE*)outputer_global, "%s\n", data.text.c_str());
    }
    extern OutputCallBack outputCallBack_global;


    //using OutputCallBack = void(*)(const info&);
    //static void printf_f(const info& data) {
    //    printf("%s\n", data.text.c_str());
    //}
    //extern OutputCallBack outputCallBack_global;

    class Logger {
    public:
        static Logger& instance() {
            static Logger l;
            return l;
        }
        ~Logger() { stop(); }

        void log(const info& i) {
            {
                std::lock_guard<std::mutex> lock(cv_mtx);
                infos.push(i);
            }
            cv.notify_one();
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(cv_mtx);
                is_running = false;
            }
            cv.notify_one();
            if (output_thread.joinable())
                output_thread.join();
        }

    private:
        Logger() : is_running(true) {
            output_thread = std::thread(&Logger::outputLoop, this);
        }

        void outputLoop() {
            while (is_running) {
                std::unique_lock<std::mutex> lock(cv_mtx);
                cv.wait(lock, [this]() {
                    return !is_running || !infos.isEmpty();
                    });

                while (!infos.isEmpty()) {
                    info out;
                    infos.pop(out);
                    if (outputCallBack_global) {
                        outputCallBack_global(out);
                    }
                }
            }
        }

    private:
        LoggerQueue infos;
        std::thread output_thread;
        std::atomic<bool> is_running;
        std::condition_variable cv;
        std::mutex cv_mtx;
    };

    // 内部命名空间 log
    inline void log(const std::string& info, LogLevel level) 
    {
        auto time = QDateTime::currentDateTime().toString();
        Logger::instance().log(myStd::info(info, level, time));
    }
}

// ======================
// 全局日志接口
// ======================
inline void logDebug(const QString& text) {
    myStd::log(qt2std(text), myStd::LogLevel::DEBUG_);
}
inline void logInfo(const QString& text) {
    myStd::log(qt2std(text), myStd::LogLevel::INFO_);
}
inline void logWarn(const QString& text) {
    myStd::log(qt2std(text), myStd::LogLevel::WARNING_);
}
inline void logError(const QString& text) {
    myStd::log(qt2std(text), myStd::LogLevel::ERROR_);
}

inline void log(const QString& text) {
    logInfo(text);
}
inline void log(const char* text) {
    logInfo(QString(text));
}