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
    struct info {
        info(const std::string& text = "") : text(text) {}
        std::string text; // 内部统一存 UTF-8
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

    using OutputCallBack = void(*)(const char*);
    static void printf_f(const char* data) {
        printf("%s\n", data);
    }
    extern OutputCallBack outputCallBack_global;

    class Logger {
    public:
        static Logger& instance() {
            static Logger l;
            return l;
        }
        ~Logger() { stop(); }

        // 底层只接收 UTF-8 std::string
        void log(const std::string& text) {
            {
                std::lock_guard<std::mutex> lock(cv_mtx);
                infos.push(info(text));
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
                        outputCallBack_global(out.text.c_str());
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
    inline void log(const std::string& info) {
        Logger::instance().log(info);
    }
}

// ======================
// 全局日志接口
// ======================
inline void log(const std::string& info) {
    myStd::log(info);
}

inline void log(const char* data) {
    log(std::string(data));
}

// ✅ 使用你的转换函数
inline void log(const QString& info) {
    log(qt2std(info));
}