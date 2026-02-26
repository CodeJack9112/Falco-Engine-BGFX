#pragma once

#include <bitset>
#include <string>
#include <vector>

namespace GX
{
    class LayerMask
    {
    private:
        std::bitset<32> layerMask = std::bitset<32>(0b1111111111111111'1111111111111111);

    public:
        static const int MAX_LAYERS;

        bool getLayer(size_t layer);
        void setLayer(size_t layer, bool enable);

        unsigned long toULong();
        void fromULong(unsigned long value);

        static std::vector<std::string> getAllLayers();

        static std::string layerToName(int layer);
        static int nameToLayer(std::string name);

        std::bitset<32>& getBits() { return layerMask; }
    };
}