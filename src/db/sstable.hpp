#ifndef SRC_DB_SSTABLE_H_
#define SRC_DB_SSTABLE_H_
#include <string>

namespace fver {

namespace db {

class SSTable {
 public:

  SSTable();
  ~SSTable();
  /*
    @brief : null
    @return : return sstable => fd
  */
  auto getFd() -> int;

  /*
    @return : return init success
    @path: the new sstable_path
    @name: the new sstable_name
  */
  bool Init(const std::string& path, const std::string& name,
            const uint32_t level, const uint32_t number);

  /*
    close the sstable.
  */
  void Close();

  // 获取当前的 Level
  auto getLevel();

  // 获取当前 SSTable Level
  void setLevel(const uint32_t n);

  // 获取当前的 number
  void setNumber(const uint32_t n);

  // 获取当前的 sstable number
  uint32_t getNumber();

  // 获取文件大小
  uint32_t getFileSize();

  // 启动当前文件 mmap 映射
  bool InitMmap();


  void CloseMmap();

  char* getMmapPtr();

 private:
  
  // 是否初始化 mmap
  bool isMmap_;
  // mmap 所使用的指针
  char* mmapBasePtr_;
  // 当前文件的长度.
  size_t fileLength_;
  // 是否打开
  bool isOpen_;
  //
  int fd_;
  uint32_t sstable_number_;
  uint32_t sstable_level_;
  std::string fullfilename_;
  //TODO, use std::string_view instead of std::string in Init.
  std::string fileName_;
  std::string filePath_;
};

}  // namespace db

}  // namespace fver

#endif