#include "src/storage/log_entry.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/storage/log_entry.h"

namespace graphchaindb {

TEST(LogEntryTest, SetSerialization) {
    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(100, TEST_KEY_1, TEST_VALUE_1);

    int total_size = LogEntry::HEADER_SIZE + 2 * sizeof(uint32_t) +
                     TEST_KEY_1.size() + TEST_VALUE_1.size();

    EXPECT_EQ(log_entry->Size(), total_size);

    char* serializedLogEntry = new char[log_entry->Size()];
    log_entry->SerializeTo(serializedLogEntry);

    EXPECT_EQ(log_entry->GetType(),
              *reinterpret_cast<LogEntryType*>(serializedLogEntry));
    EXPECT_EQ(log_entry->GetLogNumber(),
              *reinterpret_cast<ln_t*>(serializedLogEntry +
                                       LogEntry::LOG_NUMBER_OFFSET));
    EXPECT_EQ(log_entry->Size(),
              *reinterpret_cast<uint32_t*>(serializedLogEntry +
                                           LogEntry::SIZE_OFFSET));

    EXPECT_EQ(log_entry->GetKeySize(),
              *reinterpret_cast<uint32_t*>(serializedLogEntry +
                                           LogEntry::HEADER_SIZE));

    std::string key{
        reinterpret_cast<char*>(serializedLogEntry + LogEntry::HEADER_SIZE +
                                sizeof(uint32_t)),
        log_entry->GetKeySize()};

    LOG(INFO) << "LogEntryTest::SetSerialization: Key from serialized buffer: "
              << log_entry->GetKey().data() << " " << key;

    EXPECT_EQ(log_entry->GetKey(), key);

    EXPECT_EQ(log_entry->GetValueSize(),
              *reinterpret_cast<uint32_t*>(
                  serializedLogEntry + LogEntry::HEADER_SIZE +
                  sizeof(uint32_t) + log_entry->GetKeySize()));

    std::string value{
        reinterpret_cast<char*>(serializedLogEntry + LogEntry::HEADER_SIZE +
                                2 * sizeof(uint32_t) + log_entry->GetKeySize()),
        log_entry->GetValueSize()};

    LOG(INFO)
        << "LogEntryTest::SetSerialization: Value from serialized buffer: "
        << log_entry->GetValue().value().data() << " " << value;

    EXPECT_EQ(log_entry->GetValue().value(), value);
}

TEST(LogEntryTest, DeleteSerialization) {
    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(100, TEST_KEY_1);

    int total_size =
        LogEntry::HEADER_SIZE + sizeof(uint32_t) + TEST_KEY_1.size();

    EXPECT_EQ(log_entry->Size(), total_size);

    char* serializedLogEntry = new char[log_entry->Size()];
    log_entry->SerializeTo(serializedLogEntry);

    EXPECT_EQ(log_entry->GetType(),
              *reinterpret_cast<LogEntryType*>(serializedLogEntry));
    EXPECT_EQ(log_entry->GetLogNumber(),
              *reinterpret_cast<ln_t*>(serializedLogEntry +
                                       LogEntry::LOG_NUMBER_OFFSET));
    EXPECT_EQ(log_entry->Size(),
              *reinterpret_cast<uint32_t*>(serializedLogEntry +
                                           LogEntry::SIZE_OFFSET));
    EXPECT_EQ(log_entry->GetKeySize(),
              *reinterpret_cast<uint32_t*>(serializedLogEntry +
                                           LogEntry::HEADER_SIZE));

    std::string key{
        reinterpret_cast<char*>(serializedLogEntry + LogEntry::HEADER_SIZE +
                                sizeof(uint32_t)),
        log_entry->GetKeySize()};

    LOG(INFO)
        << "LogEntryTest::DeleteSerialization: Key from serialized buffer: "
        << log_entry->GetKey().data() << " " << key;

    EXPECT_EQ(log_entry->GetKey(), key);
}

TEST(LogEntryTest, SetDeserialization) {
    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(100, TEST_KEY_1, TEST_VALUE_1);

    int total_size = LogEntry::HEADER_SIZE + 2 * sizeof(uint32_t) +
                     TEST_KEY_1.size() + TEST_VALUE_1.size();

    EXPECT_EQ(log_entry->Size(), total_size);

    char* serializedLogEntry = new char[log_entry->Size()];
    log_entry->SerializeTo(serializedLogEntry);

    std::unique_ptr<LogEntry> deserialized_log_entry =
        LogEntry::DeserializeFrom(serializedLogEntry);

    EXPECT_EQ(log_entry->Size(), deserialized_log_entry->Size());
    EXPECT_EQ(log_entry->GetType(), deserialized_log_entry->GetType());
    EXPECT_EQ(log_entry->GetLogNumber(),
              deserialized_log_entry->GetLogNumber());
    EXPECT_EQ(log_entry->GetKeySize(), deserialized_log_entry->GetKeySize());
    EXPECT_EQ(log_entry->GetValueSize(),
              deserialized_log_entry->GetValueSize());

    LOG(INFO) << "LogEntryTest::SetDeserialization: Key: "
              << log_entry->GetKey().data() << " "
              << deserialized_log_entry->GetKey().data();

    EXPECT_EQ(log_entry->GetKey(), deserialized_log_entry->GetKey());

    LOG(INFO) << "LogEntryTest::SetDeserialization: Value: "
              << log_entry->GetValue().value().data() << " "
              << deserialized_log_entry->GetValue().value().data();

    EXPECT_EQ(log_entry->GetValue().value(),
              deserialized_log_entry->GetValue().value());
}

TEST(LogEntryTest, DeleteDeserialization) {
    std::unique_ptr<LogEntry> log_entry =
        std::make_unique<LogEntry>(100, TEST_KEY_1);

    int total_size =
        LogEntry::HEADER_SIZE + sizeof(uint32_t) + TEST_KEY_1.size();

    EXPECT_EQ(log_entry->Size(), total_size);

    char* serializedLogEntry = new char[log_entry->Size()];
    log_entry->SerializeTo(serializedLogEntry);

    std::unique_ptr<LogEntry> deserialized_log_entry =
        LogEntry::DeserializeFrom(serializedLogEntry);

    EXPECT_EQ(log_entry->Size(), deserialized_log_entry->Size());
    EXPECT_EQ(log_entry->GetType(), deserialized_log_entry->GetType());
    EXPECT_EQ(log_entry->GetLogNumber(),
              deserialized_log_entry->GetLogNumber());
    EXPECT_EQ(log_entry->GetKeySize(), deserialized_log_entry->GetKeySize());

    LOG(INFO) << "LogEntryTest::DeleteDeserialization: Key: "
              << log_entry->GetKey().data() << " "
              << deserialized_log_entry->GetKey().data();

    EXPECT_EQ(log_entry->GetKey(), deserialized_log_entry->GetKey());
}

}  // namespace graphchaindb
