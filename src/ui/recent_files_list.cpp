#include "ui/recent_files_list.h"
#include <algorithm>
#include <filesystem>

RecentFilesList::RecentFilesList(size_t max_size)
    : max_size_(max_size) {
}

void RecentFilesList::add(const std::string& filepath) {
    // Remove if already exists (to move it to front)
    auto it = std::find(files_.begin(), files_.end(), filepath);
    if (it != files_.end()) {
        files_.erase(it);
    }

    // Add to front
    files_.push_front(filepath);

    // Evict oldest entry if we exceed max size
    if (files_.size() > max_size_) {
        files_.pop_back();
    }
}

bool RecentFilesList::remove(const std::string& filepath) {
    auto it = std::find(files_.begin(), files_.end(), filepath);
    if (it != files_.end()) {
        files_.erase(it);
        return true;
    }
    return false;
}

bool RecentFilesList::contains(const std::string& filepath) const {
    return std::find(files_.begin(), files_.end(), filepath) != files_.end();
}

std::vector<std::string> RecentFilesList::get_all() const {
    return std::vector<std::string>(files_.begin(), files_.end());
}

size_t RecentFilesList::size() const {
    return files_.size();
}

bool RecentFilesList::empty() const {
    return files_.empty();
}

void RecentFilesList::clear() {
    files_.clear();
}

size_t RecentFilesList::max_size() const {
    return max_size_;
}
