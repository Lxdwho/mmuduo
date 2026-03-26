/**
 * @brief 网络库缓冲类
 * @date 2026.03.22
 */

#pragma once

#include <unistd.h>
#include <vector>
#include <string>

/**
 * @brief 一个自适应缓冲区，用于对收发数据的缓存
 */
class Buffer {
public:
    static const size_t KCheapPrepend = 8;
    static const size_t KInitialSize = 1024;

    explicit Buffer(size_t initialSize = KInitialSize)
            : buffer_(KCheapPrepend + initialSize)
            , readerIndex_(KCheapPrepend)
            , writerIndex_(KCheapPrepend) {}
    /* 返回可读数据数量 */
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    /* 返回可写数据数量 */
    size_t writeableBytes() const { return buffer_.size() - writerIndex_; }
    /* 返回读指针位置 */
    size_t prependableBtes() const { return readerIndex_; }
    /* 返回首个可读数据指针 */
    const char* peek() const { return begin() + readerIndex_; }
    /* 设置buffer为初始状态 */
    void retrieveAll() { readerIndex_ = writerIndex_ = KCheapPrepend; }

    /**
     * @brief 读数据后重置读指针
     * @param len 被读取的字节数
     */
    void retrieve(size_t len) {
        if(len < readableBytes()) {
            readerIndex_ += len;
        }
        else {
            retrieveAll();
        }
    }
    /**
     * @brief 以字符串形式读
     * @param len 读的长度
     */
    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    /* 以字符串形式读取所有数据 */
    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

    /**
     * @brief 写入时检查空间是否足够
     * @param len 准备写的长度
     */
    void ensureWriteableBytes(size_t len) {
        if(writeableBytes() < len) {
            makeSpace(len);
        }
    }

    /* 返回写指针地址 */
    char* beginWrite() { return begin() + writerIndex_; }
    /* 返回写指针地址、常量指针 */
    const char* beginWrite() const { return begin() + writerIndex_; }

    /**
     * @brief 在buffer写指针后，追加
     * @param data 追加数据
     * @param len 追加长度
     */
    void append(const char* data, size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }
    
    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);
private:
    /* 获取buffer存储区的首地址 */
    char* begin() { return &*buffer_.begin(); }
    /* 获取buffer存储区的首地址，常量指针重载 */
    const char* begin() const { return &*buffer_.begin(); }

    /**
     * @brief 重新构建缓冲区
     * @param len 需要的空间
     */
    void makeSpace(size_t len) {
        if(writeableBytes() + prependableBtes() < len + KCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        }
        else {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + KCheapPrepend);
            readerIndex_ = KCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};


