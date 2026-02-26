#include "TransformEditor.h"

#include "../Windows/MainWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Renderer/BatchedGeometry.h"
#include "../Engine/Renderer/CSGGeometry.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/CSGBrush.h"
#include "../Engine/Assets/Texture.h"

#include "../Engine/Classes/Helpers.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Gizmo/Gizmo.h"

#include "Property.h"
#include "PropVector3.h"

#include "../Classes/Undo.h"

namespace GX
{
	TransformEditor::TransformEditor()
	{
		setEditorName("TransformEditor");
	}

	TransformEditor::~TransformEditor()
	{
	}

	void TransformEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		Transform* currentTransform = (Transform*)comps[0];

		glm::vec3 eulerAngles = Mathf::toEuler(currentTransform->getLocalRotation());

		position = new PropVector3(this, "Position", currentTransform->getLocalPosition());
		rotation = new PropVector3(this, "Rotation", eulerAngles);
		scale = new PropVector3(this, "Scale", currentTransform->getLocalScale());

		position->setShowBadge(true);
		rotation->setShowBadge(true);
		scale->setShowBadge(true);

		rotation->setMaskX("°");
		rotation->setMaskY("°");
		rotation->setMaskZ("°");

		position->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangePosition(prop, val); });
		rotation->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeRotation(prop, val); });
		scale->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangeScale(prop, val); });

		addProperty(position);
		addProperty(rotation);
		addProperty(scale);
	}

	void TransformEditor::onChangePosition(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change object position");
		undoData->vec3Data.resize(2);

		for (auto& comp : components)
		{
			undoData->vec3Data[0][comp] = ((Transform*)comp)->getLocalPosition();
			undoData->vec3Data[1][comp] = val;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->vec3Data[0])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalPosition(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->vec3Data[1])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalPosition(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//

		bool updBatches = false;
		bool updCsg = false;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Transform* t = (Transform*)*it;
			t->setLocalPosition(val);

			if (!updBatches || !updCsg)
			{
				t->iterateChildren([&updBatches, &updCsg](Transform* ct) -> bool
					{
						if (ct->getGameObject()->getBatchingStatic())
							updBatches = true;

						CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (csgBrush != nullptr)
						{
							csgBrush->rebuild();
							updCsg = true;
						}

						return true;
					}
				);
			}
		}

		MainWindow::getSceneWindow()->getGizmo()->updatePosition();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void TransformEditor::onChangeRotation(Property* prop, glm::vec3 val)
	{
		glm::highp_quat rotation = Mathf::toQuaternion(val);

		//Undo
		UndoData* undoData = Undo::addUndo("Change object rotation");
		undoData->quatData.resize(2);

		for (auto& comp : components)
		{
			undoData->quatData[0][comp] = ((Transform*)comp)->getLocalRotation();
			undoData->quatData[1][comp] = rotation;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->quatData[0])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalRotation(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->quatData[1])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalRotation(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//

		bool updBatches = false;
		bool updCsg = false;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Transform* t = (Transform*)*it;
			t->setLocalRotation(rotation);

			if (!updBatches)
			{
				t->iterateChildren([&updBatches, &updCsg](Transform* ct) -> bool
					{
						if (ct->getGameObject()->getBatchingStatic())
							updBatches = true;

						CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (csgBrush != nullptr)
						{
							csgBrush->rebuild();
							updCsg = true;
						}

						return true;
					}
				);
			}
		}

		MainWindow::getSceneWindow()->getGizmo()->updatePosition();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void TransformEditor::onChangeScale(Property* prop, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change object scale");
		undoData->vec3Data.resize(2);

		for (auto& comp : components)
		{
			undoData->vec3Data[0][comp] = ((Transform*)comp)->getLocalScale();
			undoData->vec3Data[1][comp] = val;
		}

		undoData->undoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->vec3Data[0])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalScale(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};

		undoData->redoAction = [=](UndoData* data)
		{
			bool updBatches = false;
			bool updCsg = false;

			std::vector<Component*> comps;

			for (auto& d : data->vec3Data[1])
			{
				Transform* t = (Transform*)d.first;
				t->setLocalScale(d.second);

				if (!updBatches || !updCsg)
				{
					t->iterateChildren([&updBatches, &updCsg, &comps](Transform* ct) -> bool
						{
							if (ct->getGameObject()->getBatchingStatic())
								updBatches = true;

							CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
							if (csgBrush != nullptr)
							{
								comps.push_back(csgBrush);

								csgBrush->rebuild();
								updCsg = true;
							}

							return true;
						}
					);
				}
			}

			MainWindow::getInspectorWindow()->updateObjectEditorTransform();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();

			if (updBatches)
				MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

			if (updCsg)
				MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(comps); });
		};
		//

		bool updBatches = false;
		bool updCsg = false;

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Transform* t = (Transform*)*it;
			t->setLocalScale(val);

			if (!updBatches || !updCsg)
			{
				t->iterateChildren([&updBatches, &updCsg](Transform* ct) -> bool
					{
						if (ct->getGameObject()->getBatchingStatic())
							updBatches = true;

						CSGBrush* csgBrush = (CSGBrush*)ct->getGameObject()->getComponent(CSGBrush::COMPONENT_TYPE);
						if (csgBrush != nullptr)
						{
							csgBrush->rebuild();
							updCsg = true;
						}

						return true;
					}
				);
			}
		}

		MainWindow::getSceneWindow()->getGizmo()->updatePosition();

		if (updBatches)
			MainWindow::addOnEndUpdateCallback([=]() { BatchedGeometry::getSingleton()->rebuild(); });

		if (updCsg)
			MainWindow::addOnEndUpdateCallback([=]() { CSGGeometry::getSingleton()->rebuild(components); });
	}

	void TransformEditor::updateTransform()
	{
		Gizmo* gizmo = MainWindow::getSingleton()->getSceneWindow()->getGizmo();

		Transform* currentNode = (Transform*)components[0];
		Transform* parentNode = currentNode->getParent();

		glm::highp_quat q = glm::identity<glm::highp_quat>();
		glm::vec3 scl = glm::vec3(1, 1, 1);
		glm::vec3 pos = glm::vec3(0, 0, 0);
		glm::vec3 eulerAngles = glm::vec3(0, 0, 0);

		if (parentNode != nullptr)
		{
			glm::mat4x4 m1 = currentNode->getTransformMatrix();
			glm::mat4x4 m2 = parentNode->getTransformMatrix();
			glm::mat4x4 m3 = glm::inverse(m2) * m1;

			q = glm::inverse(parentNode->getRotation()) * currentNode->getRotation();
			glm::vec3 ds = parentNode->getScale();
			scl = glm::vec3(1.0f / ds.x, 1.0f / ds.y, 1.0f / ds.z) * currentNode->getScale();
			pos = m3[3];

			eulerAngles = Mathf::toEuler(q);
		}
		else
		{
			q = currentNode->getRotation();
			scl = currentNode->getScale();
			pos = currentNode->getPosition();
			eulerAngles = Mathf::toEuler(q);
		}

		position->setValue(pos);
		rotation->setValue(eulerAngles);
		scale->setValue(scl);
	}
}