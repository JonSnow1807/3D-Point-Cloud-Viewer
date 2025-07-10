#include "utils/FileIO.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace pcv {

bool FileIO::load(const std::string& filename, PointCloud& cloud, Format format) {
    if (format == Format::AUTO) {
        format = getFormatFromExtension(filename);
    }
    
    switch (format) {
        case Format::XYZ:
        case Format::XYZRGB:
            return loadXYZ(filename, cloud);
        case Format::PLY:
            return loadPLY(filename, cloud);
        case Format::PCD:
            return loadPCD(filename, cloud);
        default:
            std::cerr << "Unsupported file format" << std::endl;
            return false;
    }
}

bool FileIO::save(const std::string& filename, const PointCloud& cloud, Format format) {
    if (format == Format::AUTO) {
        format = getFormatFromExtension(filename);
    }
    
    switch (format) {
        case Format::XYZ:
        case Format::XYZRGB:
            return saveXYZ(filename, cloud);
        case Format::PLY:
            return savePLY(filename, cloud);
        case Format::PCD:
            return savePCD(filename, cloud);
        default:
            std::cerr << "Unsupported file format" << std::endl;
            return false;
    }
}

FileIO::Format FileIO::getFormatFromExtension(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return Format::AUTO;
    }
    
    std::string ext = filename.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "xyz" || ext == "txt") {
        return Format::XYZRGB;
    } else if (ext == "ply") {
        return Format::PLY;
    } else if (ext == "pcd") {
        return Format::PCD;
    }
    
    return Format::AUTO;
}

bool FileIO::loadXYZ(const std::string& filename, PointCloud& cloud) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    cloud.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        float x, y, z, r = 1.0f, g = 1.0f, b = 1.0f;
        
        if (iss >> x >> y >> z) {
            // Try to read color
            iss >> r >> g >> b;
            cloud.addPoint(glm::vec3(x, y, z), glm::vec3(r, g, b));
        }
    }
    
    file.close();
    return !cloud.empty();
}

bool FileIO::saveXYZ(const std::string& filename, const PointCloud& cloud) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    file << "# Point Cloud Data\n";
    file << "# Format: X Y Z R G B\n";
    file << "# Points: " << cloud.size() << "\n";
    
    file << std::fixed << std::setprecision(6);
    
    for (const auto& point : cloud) {
        file << point.position.x << " " 
             << point.position.y << " " 
             << point.position.z << " "
             << point.color.r << " "
             << point.color.g << " "
             << point.color.b << "\n";
    }
    
    file.close();
    return true;
}

bool FileIO::loadPLY(const std::string& filename, PointCloud& cloud) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open PLY file: " << filename << std::endl;
        return false;
    }
    
    cloud.clear();
    std::string line;
    
    // Read header
    bool header_ended = false;
    size_t vertex_count = 0;
    bool has_color = false;
    
    while (std::getline(file, line) && !header_ended) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        
        if (keyword == "element") {
            std::string element_type;
            iss >> element_type;
            if (element_type == "vertex") {
                iss >> vertex_count;
            }
        } else if (keyword == "property") {
            std::string type, name;
            iss >> type >> name;
            if (name == "red" || name == "r") {
                has_color = true;
            }
        } else if (keyword == "end_header") {
            header_ended = true;
        }
    }
    
    // Read vertices
    cloud.reserve(vertex_count);
    
    for (size_t i = 0; i < vertex_count; ++i) {
        float x, y, z, r = 1.0f, g = 1.0f, b = 1.0f;
        
        file >> x >> y >> z;
        if (has_color) {
            int ir, ig, ib;
            file >> ir >> ig >> ib;
            r = ir / 255.0f;
            g = ig / 255.0f;
            b = ib / 255.0f;
        }
        
        cloud.addPoint(glm::vec3(x, y, z), glm::vec3(r, g, b));
    }
    
    file.close();
    return !cloud.empty();
}

bool FileIO::savePLY(const std::string& filename, const PointCloud& cloud) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open PLY file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write header
    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << cloud.size() << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "property uchar red\n";
    file << "property uchar green\n";
    file << "property uchar blue\n";
    file << "end_header\n";
    
    // Write vertices
    for (const auto& point : cloud) {
        file << point.position.x << " " 
             << point.position.y << " " 
             << point.position.z << " "
             << static_cast<int>(point.color.r * 255) << " "
             << static_cast<int>(point.color.g * 255) << " "
             << static_cast<int>(point.color.b * 255) << "\n";
    }
    
    file.close();
    return true;
}

bool FileIO::loadPCD(const std::string& filename, PointCloud& cloud) {
    // Simplified PCD loader - full implementation would handle all PCD features
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open PCD file: " << filename << std::endl;
        return false;
    }
    
    cloud.clear();
    std::string line;
    size_t points = 0;
    bool data_started = false;
    
    while (std::getline(file, line)) {
        if (!data_started) {
            if (line.find("POINTS") == 0) {
                std::istringstream iss(line);
                std::string keyword;
                iss >> keyword >> points;
                cloud.reserve(points);
            } else if (line == "DATA ascii") {
                data_started = true;
            }
        } else {
            // Read point data
            std::istringstream iss(line);
            float x, y, z;
            if (iss >> x >> y >> z) {
                cloud.addPoint(glm::vec3(x, y, z));
            }
        }
    }
    
    file.close();
    return !cloud.empty();
}

bool FileIO::savePCD(const std::string& filename, const PointCloud& cloud) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open PCD file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write PCD header
    file << "# .PCD v0.7 - Point Cloud Data file format\n";
    file << "VERSION 0.7\n";
    file << "FIELDS x y z rgb\n";
    file << "SIZE 4 4 4 4\n";
    file << "TYPE F F F U\n";
    file << "COUNT 1 1 1 1\n";
    file << "WIDTH " << cloud.size() << "\n";
    file << "HEIGHT 1\n";
    file << "VIEWPOINT 0 0 0 1 0 0 0\n";
    file << "POINTS " << cloud.size() << "\n";
    file << "DATA ascii\n";
    
    // Write point data
    for (const auto& point : cloud) {
        // Pack RGB into single uint32
        uint32_t rgb = ((uint32_t)(point.color.r * 255) << 16) |
                       ((uint32_t)(point.color.g * 255) << 8) |
                       ((uint32_t)(point.color.b * 255));
        
        file << point.position.x << " " 
             << point.position.y << " " 
             << point.position.z << " "
             << rgb << "\n";
    }
    
    file.close();
    return true;
}

} // namespace pcv