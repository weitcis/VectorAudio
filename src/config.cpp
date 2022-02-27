#include "config.h"

#ifdef SFML_SYSTEM_MACOS
    #include "osx_resources.h"
#endif

#ifdef SFML_SYSTEM_WINDOWS
    #pragma comment(lib, "Shlwapi.lib")
    #include <windows.h>
    #include <Shlwapi.h>
#endif

#ifdef SFML_SYSTEM_LINUX
    #include <limits.h>
    #include <unistd.h> 
#endif

namespace afv_unix {
    toml::value configuration::config;

    void configuration::build_config() {
        #ifdef NDEBUG
            file_path = get_resource_folder() + file_path;
        #endif

        if (std::filesystem::exists(file_path)) {
            afv_unix::configuration::config = toml::parse(file_path);
        } else {
            spdlog::info("Did not find a config file, starting from scratch.");
        }
    }

    std::string configuration::get_resource_folder() {
        #ifndef NDEBUG
            return "../resources/";
        #else
            #ifdef SFML_SYSTEM_MACOS
                return afv_unix::native::osx_resourcePath();
            #endif

            #ifdef SFML_SYSTEM_LINUX
                char result[PATH_MAX];
                ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
                return std::string(result, (count > 0) ? count : 0);
            #endif

            #ifdef SFML_SYSTEM_WINDOWS
                wchar_t path[MAX_PATH] = { 0 };
                GetModuleFileNameW(NULL, path, MAX_PATH);
                PathRemoveFileSpecA(path);
                std::wstring ws(path);
                return std::string(ws.begin(), ws.end()) + std::string("/");
            #endif
        #endif
    }

    void configuration::write_config_async() {
        std::thread([](){
            std::ofstream ofs(afv_unix::configuration::file_path);
            ofs << afv_unix::configuration::config; 
            ofs.close();
        }).detach();
    }

    void configuration::build_logger() {
        spdlog::init_thread_pool(8192, 1);

        auto async_rotating_file_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("VectorAudio", configuration::get_resource_folder() + "vector_audio.log", 1024*1024*10, 3);
        
        #ifdef NDEBUG
            spdlog::set_level(spdlog::level::info);
        #else
            spdlog::set_level(spdlog::level::trace);
        #endif

        spdlog::flush_on(spdlog::level::info);
        spdlog::set_default_logger(async_rotating_file_logger);
    }
}