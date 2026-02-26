#include "CSGModel.h"

namespace GX
{
	std::string CSGModel::COMPONENT_TYPE = "CSGModel";


	CSGModel::CSGModel() : Component(nullptr)
	{

	}

	Component* CSGModel::onClone()
	{
		CSGModel* newComponent = new CSGModel();

		return newComponent;
	}
}