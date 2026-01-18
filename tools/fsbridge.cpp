#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <string>

namespace fs = std::filesystem;

#define DISK_BOOTSECTOR_SIZE 512
#define SKFS_WHOLEAREA_SIZE (4 * 1024 * 1024)

#define SKFS_MAX_NAME_LENGTH 32
#define SKFS_MAX_FILE_COUNT 64
#define SKFS_MAX_DATA_SIZE (4 * 1024)

#define SKFS_SIGNATURE 0x534B4653

struct skfs_file_t {
    char name[SKFS_MAX_NAME_LENGTH];
    char data[SKFS_MAX_DATA_SIZE];
};

struct skfs_directory_t {
    char name[SKFS_MAX_NAME_LENGTH];
    uint32_t file_count;
    struct skfs_file_t file[SKFS_MAX_FILE_COUNT];
};

struct skfs_root_t {
    uint32_t signature;
    uint32_t dir_count;
    struct skfs_directory_t dir[(SKFS_WHOLEAREA_SIZE - sizeof(uint64_t)) / sizeof(struct skfs_directory_t)];
};

char* skfs_WholeArea;
skfs_root_t* skfs_root;

uint32_t skfs_MaxDirCount = (SKFS_WHOLEAREA_SIZE - sizeof(uint64_t)) / sizeof(skfs_directory_t);

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "\033[1;31mError:\033[0m Not enough arguments provided." << std::endl;
        return 1;
    }

    char* bootsec = new char[DISK_BOOTSECTOR_SIZE];

    {
        std::ifstream disk_image(argv[2], std::ios::binary);
        if (!disk_image.is_open()) {
            std::cerr << "\033[1;31mError:\033[0m Could not open the disk image." << std::endl;
            delete[] bootsec;
            return 1;
        }
        disk_image.read(bootsec, DISK_BOOTSECTOR_SIZE);
    }

    if ((std::string)argv[1] == "-l") {
        std::ifstream disk_image(argv[2], std::ios::binary);
        if (!disk_image.is_open()) {
            std::cerr << "\033[1;31mError:\033[0m Could not open the disk image." << std::endl;
            delete[] bootsec;
            return 1;
        }

        size_t disk_image_size = DISK_BOOTSECTOR_SIZE + SKFS_WHOLEAREA_SIZE;
        char* image_buffer = new char[disk_image_size + 1];
        disk_image.read(image_buffer, disk_image_size);
        image_buffer[disk_image_size] = '\0';
        disk_image.close();

        skfs_WholeArea = new char[SKFS_WHOLEAREA_SIZE];

        std::copy(
            image_buffer + DISK_BOOTSECTOR_SIZE,
            image_buffer + DISK_BOOTSECTOR_SIZE + SKFS_WHOLEAREA_SIZE,
            skfs_WholeArea
        );
        delete[] image_buffer;

        skfs_root = (skfs_root_t*)skfs_WholeArea;

        if (skfs_root->signature != SKFS_SIGNATURE) {
            std::cerr << "\033[1;31mError:\033[0m Invalid signature." << std::endl;
            delete[] skfs_WholeArea;
            delete[] bootsec;
            return 1;
        }

        std::cout << "/" << std::endl;
        for (int i = 0; i < skfs_root->dir_count; ++i) {
            try {
                std::ostringstream fdirname;
                fdirname << argv[3] << "/" << skfs_root->dir[i].name;
                std::string dirname = fdirname.str();
                if (fs::create_directory(dirname)) {
                    std::cout << "    " << (std::string)(skfs_root->dir[i].name) << "/" << std::endl;
                } else {
                    std::cout << "    \033[1;30mFail:\033[0m " << (std::string)(skfs_root->dir[i].name) << "/" << std::endl;
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "\033[1;31mFatal error:\033[0m " << e.what() << std::endl;
                delete[] skfs_WholeArea;
                delete[] bootsec;
                return 1;
            }
            for (int j = 0; j < skfs_root->dir[i].file_count; ++j) {
                std::ostringstream ffilename;
                ffilename << argv[3] << "/" << skfs_root->dir[i].name << "/" << skfs_root->dir[i].file[j].name;
                std::string filename = ffilename.str();
                std::ofstream file(filename);
                if (file.is_open()) {
                    file << skfs_root->dir[i].file[j].data;
                    file.close();
                    std::cout << "        " << (std::string)(skfs_root->dir[i].file[j].name) << std::endl;
                } else {
                    std::cout << "        \033[1;31mFail:\033[0m " << (std::string)(skfs_root->dir[i].file[j].name) << std::endl;
                }
            }
        }

        delete[] skfs_WholeArea;
    } else if ((std::string)argv[1] == "-s") {
        skfs_WholeArea = new char[SKFS_WHOLEAREA_SIZE];
        std::fill(skfs_WholeArea, skfs_WholeArea + SKFS_WHOLEAREA_SIZE, 0);

        skfs_root = (skfs_root_t*)skfs_WholeArea;

        skfs_root->signature = SKFS_SIGNATURE;

        fs::path dirname = argv[3];

        std::vector<fs::path> dirs;

        try {
            if (fs::exists(dirname) && fs::is_directory(dirname)) {
                for (const auto& entry : fs::directory_iterator(dirname)) {
                    if (fs::is_directory(entry)) {
                        dirs.push_back(entry.path());
                    }
                }
                std::cout << "/" << std::endl;
                skfs_root->dir_count = 0;
                for (const auto& dir : dirs) {
                    std::string sdirname = dir.string();
                    std::cout << "    " << sdirname.c_str() + strlen(argv[3]) + 1 << "/" << std::endl;
                    std::strncpy(skfs_root->dir[skfs_root->dir_count].name, sdirname.c_str() + strlen(argv[3]) + 1, SKFS_MAX_NAME_LENGTH);
                    for (const auto& entry : fs::directory_iterator(dir)) {
                        if (fs::is_regular_file(entry.status())) {
                            std::string sfilename = entry.path().filename().string();
                            std::cout << "        " << sfilename << std::endl;
                            std::strncpy(skfs_root->dir[skfs_root->dir_count].file[skfs_root->dir[skfs_root->dir_count].file_count].name, sfilename.c_str(), SKFS_MAX_NAME_LENGTH);
                            std::ifstream ifile(entry.path(), std::ios::binary);
                            if (ifile.is_open()) {
                                ifile.seekg(0, std::ios::end);
                                size_t ifile_size = ifile.tellg();
                                ifile.seekg(0, std::ios::beg);
                                ifile.read(skfs_root->dir[skfs_root->dir_count].file[skfs_root->dir[skfs_root->dir_count].file_count].data, SKFS_MAX_DATA_SIZE);
                                skfs_root->dir[skfs_root->dir_count].file[skfs_root->dir[skfs_root->dir_count].file_count].data[SKFS_MAX_DATA_SIZE - 1] = '\0';
                            }
                            ifile.close();
                            skfs_root->dir[skfs_root->dir_count].file_count++;
                        }
                    }
                    skfs_root->dir_count++;
                }
            } else {
                std::cerr << "\033[1;31mError:\033[0m Target directory invalid." << std::endl;
                delete[] skfs_WholeArea;
                delete[] bootsec;
                return 1;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "\033[1;31mFatal error:\033[0m " << e.what() << std::endl;
            delete[] skfs_WholeArea;
            delete[] bootsec;
            return 1;
        }

        std::ofstream disk_image(argv[2], std::ios::binary);
        if (!disk_image || !disk_image.is_open()) {
            std::cerr << "\033[1;31mError:\033[0m Could not open the disk image." << std::endl;
            delete[] skfs_WholeArea;
            delete[] bootsec;
            return 1;
        }

        size_t disk_image_size = DISK_BOOTSECTOR_SIZE + SKFS_WHOLEAREA_SIZE;
        char* buffer = new char[disk_image_size];

        std::copy(bootsec, bootsec + DISK_BOOTSECTOR_SIZE, buffer);
        std::copy(skfs_WholeArea, skfs_WholeArea + SKFS_WHOLEAREA_SIZE, buffer + DISK_BOOTSECTOR_SIZE);

        disk_image.write(buffer, disk_image_size);
        if (!disk_image) {
            std::cerr << "\033[1;31mError:\033[0m An error occured while writing file system to disk." << std::endl;
            delete[] skfs_WholeArea;
            delete[] bootsec;
            return 1;
        }

        disk_image.close();
        
        delete[] skfs_WholeArea;
        delete[] bootsec;
    } else { std::cerr << "\033[1;31mError:\033[0m Unknown option: '" << argv[1] << "'" << std::endl; return 1; }
    return 0;
}