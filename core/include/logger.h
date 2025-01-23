//
// Created by peter on 2025/1/22.
//

#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include "google/glog/logging.h"
#include <string>

#define LOG_INFO(a) COMPACT_GOOGLE_LOG_INFO.stream()<<a;
#define LOG_ERROR(a) COMPACT_GOOGLE_LOG_ERROR.stream()<<a;
#define LOG_TRACE(a) COMPACT_GOOGLE_LOG_INFO.stream()<<a;

namespace logger{
    struct log_options{
        std::string dir;//存储目录
        int clean_days;//间隔清理时间 天
        int size_each_file;//每个日志文件的大小 M
    };
    static void InitLog(const char *argv0, const log_options& options){
        google::InitGoogleLogging(argv0);
        google::EnableLogCleaner(options.clean_days);
        FLAGS_log_dir = options.dir;
        FLAGS_max_log_size = options.size_each_file;
        FLAGS_logbuflevel = -1;
        //FLAGS_timestamp_in_logfile_name = false;
    }
}


#endif //SERVER_LOGGER_H
