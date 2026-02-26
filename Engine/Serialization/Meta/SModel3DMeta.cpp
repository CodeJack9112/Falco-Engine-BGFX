#include "SModel3DMeta.h"

#include <iostream>
#include <fstream>

#include "Core/Debug.h"

namespace GX
{
    void SModel3DMeta::save(std::string path)
    {
        std::ofstream ofs(path, std::ios::binary);
        BinarySerializer s;
        s.serialize(&ofs, this, "3D Model Meta");
        ofs.close();

        filePath = path;
    }

    void SModel3DMeta::load(std::string path)
    {
        try
        {
            std::ifstream ofs(path, std::ios::binary);
            BinarySerializer s;
            s.deserialize(&ofs, this, "3D Model Meta");
            ofs.close();
        }
        catch (std::exception e)
        {
            Debug::log(std::string("[" + path + "] Error loading asset meta file: ") + e.what(), Debug::DbgColorRed);
        }

        filePath = path;
    }
}