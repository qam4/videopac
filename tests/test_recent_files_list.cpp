#include <gtest/gtest.h>
#include "ui/recent_files_list.h"

// Test fixture for RecentFilesList tests
class RecentFilesListTest : public ::testing::Test {
protected:
    RecentFilesList list{10};  // Default max size of 10
};

// Test adding files to the list
TEST_F(RecentFilesListTest, AddFile) {
    list.add("/path/to/file1.bin");
    
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list.contains("/path/to/file1.bin"));
    
    auto files = list.get_all();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "/path/to/file1.bin");
}

// Test adding multiple files
TEST_F(RecentFilesListTest, AddMultipleFiles) {
    list.add("/path/to/file1.bin");
    list.add("/path/to/file2.bin");
    list.add("/path/to/file3.bin");
    
    EXPECT_EQ(list.size(), 3);
    
    auto files = list.get_all();
    ASSERT_EQ(files.size(), 3);
    // Most recent should be first
    EXPECT_EQ(files[0], "/path/to/file3.bin");
    EXPECT_EQ(files[1], "/path/to/file2.bin");
    EXPECT_EQ(files[2], "/path/to/file1.bin");
}

// Test that adding an existing file moves it to the front
TEST_F(RecentFilesListTest, AddExistingFileMovesToFront) {
    list.add("/path/to/file1.bin");
    list.add("/path/to/file2.bin");
    list.add("/path/to/file3.bin");
    
    // Add file1 again - it should move to front
    list.add("/path/to/file1.bin");
    
    EXPECT_EQ(list.size(), 3);  // Size should not increase
    
    auto files = list.get_all();
    ASSERT_EQ(files.size(), 3);
    EXPECT_EQ(files[0], "/path/to/file1.bin");  // Now at front
    EXPECT_EQ(files[1], "/path/to/file3.bin");
    EXPECT_EQ(files[2], "/path/to/file2.bin");
}

// Test list size limit (max 10 entries)
TEST_F(RecentFilesListTest, ListSizeLimit) {
    // Add 15 files
    for (int i = 0; i < 15; i++) {
        list.add("/path/to/file" + std::to_string(i) + ".bin");
    }
    
    // Should only keep 10 most recent
    EXPECT_EQ(list.size(), 10);
    
    auto files = list.get_all();
    ASSERT_EQ(files.size(), 10);
    
    // Most recent should be file14, oldest should be file5
    EXPECT_EQ(files[0], "/path/to/file14.bin");
    EXPECT_EQ(files[9], "/path/to/file5.bin");
    
    // file0-file4 should have been evicted
    EXPECT_FALSE(list.contains("/path/to/file0.bin"));
    EXPECT_FALSE(list.contains("/path/to/file4.bin"));
}

// Test eviction of oldest entry when limit is exceeded
TEST_F(RecentFilesListTest, EvictionPolicy) {
    // Fill the list to capacity
    for (int i = 0; i < 10; i++) {
        list.add("/path/to/file" + std::to_string(i) + ".bin");
    }
    
    EXPECT_EQ(list.size(), 10);
    EXPECT_TRUE(list.contains("/path/to/file0.bin"));
    
    // Add one more - should evict file0 (oldest)
    list.add("/path/to/file10.bin");
    
    EXPECT_EQ(list.size(), 10);
    EXPECT_FALSE(list.contains("/path/to/file0.bin"));  // Evicted
    EXPECT_TRUE(list.contains("/path/to/file1.bin"));   // Still present
    EXPECT_TRUE(list.contains("/path/to/file10.bin"));  // Newly added
}

// Test removing a file from the list
TEST_F(RecentFilesListTest, RemoveFile) {
    list.add("/path/to/file1.bin");
    list.add("/path/to/file2.bin");
    list.add("/path/to/file3.bin");
    
    EXPECT_EQ(list.size(), 3);
    
    // Remove file2
    bool removed = list.remove("/path/to/file2.bin");
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(list.size(), 2);
    EXPECT_FALSE(list.contains("/path/to/file2.bin"));
    
    auto files = list.get_all();
    ASSERT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "/path/to/file3.bin");
    EXPECT_EQ(files[1], "/path/to/file1.bin");
}

// Test removing a non-existent file
TEST_F(RecentFilesListTest, RemoveNonExistentFile) {
    list.add("/path/to/file1.bin");
    
    bool removed = list.remove("/path/to/nonexistent.bin");
    
    EXPECT_FALSE(removed);
    EXPECT_EQ(list.size(), 1);
}

// Test contains method
TEST_F(RecentFilesListTest, ContainsFile) {
    list.add("/path/to/file1.bin");
    list.add("/path/to/file2.bin");
    
    EXPECT_TRUE(list.contains("/path/to/file1.bin"));
    EXPECT_TRUE(list.contains("/path/to/file2.bin"));
    EXPECT_FALSE(list.contains("/path/to/file3.bin"));
}

// Test empty list
TEST_F(RecentFilesListTest, EmptyList) {
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    
    auto files = list.get_all();
    EXPECT_TRUE(files.empty());
    
    list.add("/path/to/file1.bin");
    EXPECT_FALSE(list.empty());
}

// Test clear method
TEST_F(RecentFilesListTest, ClearList) {
    list.add("/path/to/file1.bin");
    list.add("/path/to/file2.bin");
    list.add("/path/to/file3.bin");
    
    EXPECT_EQ(list.size(), 3);
    
    list.clear();
    
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_FALSE(list.contains("/path/to/file1.bin"));
}

// Test max_size method
TEST_F(RecentFilesListTest, MaxSize) {
    EXPECT_EQ(list.max_size(), 10);
    
    RecentFilesList small_list(5);
    EXPECT_EQ(small_list.max_size(), 5);
}

// Test custom max size
TEST_F(RecentFilesListTest, CustomMaxSize) {
    RecentFilesList small_list(3);
    
    small_list.add("/path/to/file1.bin");
    small_list.add("/path/to/file2.bin");
    small_list.add("/path/to/file3.bin");
    small_list.add("/path/to/file4.bin");
    
    // Should only keep 3 most recent
    EXPECT_EQ(small_list.size(), 3);
    EXPECT_FALSE(small_list.contains("/path/to/file1.bin"));  // Evicted
    EXPECT_TRUE(small_list.contains("/path/to/file4.bin"));
}

// Test separate ROM and BIOS lists
TEST_F(RecentFilesListTest, SeparateLists) {
    RecentFilesList rom_list(10);
    RecentFilesList bios_list(10);
    
    rom_list.add("/path/to/game1.bin");
    rom_list.add("/path/to/game2.bin");
    
    bios_list.add("/path/to/bios1.bin");
    bios_list.add("/path/to/bios2.bin");
    
    // Lists should be independent
    EXPECT_EQ(rom_list.size(), 2);
    EXPECT_EQ(bios_list.size(), 2);
    
    EXPECT_TRUE(rom_list.contains("/path/to/game1.bin"));
    EXPECT_FALSE(rom_list.contains("/path/to/bios1.bin"));
    
    EXPECT_TRUE(bios_list.contains("/path/to/bios1.bin"));
    EXPECT_FALSE(bios_list.contains("/path/to/game1.bin"));
}

// Test with special characters in filenames
TEST_F(RecentFilesListTest, SpecialCharactersInFilenames) {
    list.add("/path/to/file with spaces.bin");
    list.add("/path/to/file-with-dashes.bin");
    list.add("/path/to/file_with_underscores.bin");
    list.add("/path/to/file(with)parens.bin");
    
    EXPECT_EQ(list.size(), 4);
    EXPECT_TRUE(list.contains("/path/to/file with spaces.bin"));
    EXPECT_TRUE(list.contains("/path/to/file-with-dashes.bin"));
    EXPECT_TRUE(list.contains("/path/to/file_with_underscores.bin"));
    EXPECT_TRUE(list.contains("/path/to/file(with)parens.bin"));
}
