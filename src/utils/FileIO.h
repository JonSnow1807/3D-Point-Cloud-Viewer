#pragma once

#include "core/PointCloud.h"
#include <string>

namespace pcv {

class FileIO {
public:
    enum class Format {
        AUTO,     // Auto-detect from extension
        XYZ,      // Simple XYZ format
        XYZRGB,   // XYZ with RGB colors
        PLY,      // Stanford PLY format
        PCD       // Point Cloud Data format
    };
    
    // Load point cloud from file
    static bool load(const std::string& filename, PointCloud& cloud, Format format = Format::AUTO);
    
    // Save point cloud to file
    static bool save(const std::string& filename, const PointCloud& cloud, Format format = Format::AUTO);
    
    // Get format from file extension
    static Format getFormatFromExtension(const std::string& filename);
    
private:
    // Format-specific loaders
    static bool loadXYZ(const std::string& filename, PointCloud& cloud);
    static bool loadPLY(const std::string& filename, PointCloud& cloud);
    static bool loadPCD(const std::string& filename, PointCloud& cloud);
    
    // Format-specific savers
    static bool saveXYZ(const std::string& filename, const PointCloud& cloud);
    static bool savePLY(const std::string& filename, const PointCloud& cloud);
    static bool savePCD(const std::string& filename, const PointCloud& cloud);
};

} // namespace pcv