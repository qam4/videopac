#ifndef RECENT_FILES_LIST_H
#define RECENT_FILES_LIST_H

#include <string>
#include <vector>
#include <deque>

// Manages a list of recently accessed files with a maximum size limit
// Automatically evicts oldest entries when the limit is exceeded
class RecentFilesList {
public:
    // Create a recent files list with a maximum size (default 10)
    explicit RecentFilesList(size_t max_size = 10);

    // Add a file to the recent list
    // If the file already exists, it's moved to the front
    // If the list is full, the oldest entry is removed
    void add(const std::string& filepath);

    // Remove a file from the recent list
    // Returns true if the file was found and removed
    bool remove(const std::string& filepath);

    // Check if a file exists in the recent list
    bool contains(const std::string& filepath) const;

    // Get all files in the recent list (most recent first)
    std::vector<std::string> get_all() const;

    // Get the number of files in the list
    size_t size() const;

    // Check if the list is empty
    bool empty() const;

    // Clear all files from the list
    void clear();

    // Get the maximum size of the list
    size_t max_size() const;

private:
    std::deque<std::string> files_;
    size_t max_size_;
};

#endif // RECENT_FILES_LIST_H
