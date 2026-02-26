#include "LayerMask.h"

#include <algorithm>

#include "../Engine/Core/Engine.h"
#include "../Engine/Serialization/Settings/ProjectSettings.h"

#include "../Engine/Classes/StringConverter.h"

namespace GX
{
	const int LayerMask::MAX_LAYERS = 32;

	bool LayerMask::getLayer(size_t layer)
	{
		return layerMask.test(layer);
	}

	void LayerMask::setLayer(size_t layer, bool enable)
	{
		layerMask.set(layer, enable);
	}

	unsigned long LayerMask::toULong()
	{
		return layerMask.to_ulong();
	}

	void LayerMask::fromULong(unsigned long value)
	{
		layerMask = std::bitset<MAX_LAYERS>(value);
	}

	std::vector<std::string> LayerMask::getAllLayers()
	{
		ProjectSettings* projectSettings = Engine::getSingleton()->getSettings();
		std::vector<std::string> layers = projectSettings->getLayers();
		layers.insert(layers.begin(), "Default");

		return layers;
	}

	std::string LayerMask::layerToName(int layer)
	{
		auto layers = getAllLayers();

		std::string _layer = "Default";
		if (layer >= 0 && layer < layers.size())
			_layer = layers[layer];

		return _layer;
	}

	int LayerMask::nameToLayer(std::string name)
	{
		auto layers = getAllLayers();

		int l = -1;

		auto it = std::find(layers.begin(), layers.end(), name);
		if (it != layers.end())
			l = std::distance(layers.begin(), it);

		return l;
	}
}