#include "SplineEditor.h"

#include <bgfx/bgfx.h>

#include "PropVector3.h"
#include "PropBool.h"

#include "../Windows/MainWindow.h"
#include "../Windows/SceneWindow.h"
#include "../Windows/InspectorWindow.h"

#include "../Engine/Core/GameObject.h"
#include "../Engine/Classes/Helpers.h"
#include "../Engine/Assets/Texture.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Spline.h"
#include "../Engine/Components/Camera.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Core/InputManager.h"
#include "../Engine/Gizmo/Gizmo.h"

#include "../Classes/Undo.h"
#include "../Classes/TreeView.h"
#include "../Classes/TreeNode.h"

namespace GX
{
	SplineEditor::SplineEditor()
	{
		setEditorName("SplineEditor");

		Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
		cb1 = gizmo->addManipulateStartCallback([=](std::vector<Transform*> nodes) { onGizmoManipulateStart(nodes); });
		cb2 = gizmo->addManipulateEndCallback([=](std::vector<Transform*> nodes) { onGizmoManipulateEnd(nodes); });
	}

	SplineEditor::~SplineEditor()
	{
		Renderer::getSingleton()->removeRenderCallback(cb);

		Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
		gizmo->removeManipulateStartCallback(cb1);
		gizmo->removeManipulateEndCallback(cb2);

		for (auto& t : transforms)
		{
			for (auto& tt : t.second)
				delete tt;

			t.second.clear();
		}

		transforms.clear();
	}

	void SplineEditor::init(std::vector<Component*> comps)
	{
		ComponentEditor::init(comps);

		cb = Renderer::getSingleton()->addRenderCallback([=](int viewId, int viewLayer, Camera* camera) { drawGizmos(viewId, viewLayer, camera); });

		splinePointTex = Texture::load(Helper::ExePath(), "Editor/Icons/Gizmo/particle_system.png", false, Texture::CompressionMethod::None);

		Spline* spline = (Spline*)comps[0];

		PropBool* closed = new PropBool(this, "Closed", spline->getClosed());
		closed->setOnChangeCallback([=](Property* prop, bool val) { onChangeClosed(val); });
		addProperty(closed);

		Transform* trans = spline->getGameObject()->getTransform();

		pointsGroup = nullptr;
		updatePoints();

		for (auto& spline1 : components)
		{
			Spline* _spline = (Spline*)spline1;
			Transform* trans1 = _spline->getGameObject()->getTransform();

			int j = 0;
			auto& points = _spline->getPoints();
			for (auto& point : points)
			{
				Transform* t = new Transform();
				t->setPosition(trans1->getTransformMatrix() * glm::vec4(point, 1.0f));
				transforms[_spline].push_back(t);
				t->setOnChangeCallback([=]() { onChangeTransformPosition(t, _spline, j); });

				++j;
			}
		}
	}

	void SplineEditor::update()
	{
		ComponentEditor::update();

		bool changed = false;
		if (MainWindow::getSceneWindow()->isFocused())
		{
			if (InputManager::getSingleton()->getMouseButtonUp(0))
				updatePoints();
		}


		for (auto& spline1 : components)
		{
			Spline* _spline = (Spline*)spline1;
			Transform* trans1 = _spline->getGameObject()->getTransform();

			if (prevTransforms[_spline] != trans1->getTransformMatrix())
			{
				prevTransforms[_spline] = trans1->getTransformMatrix();

				int j = 0;
				auto& points = _spline->getPoints();
				for (auto& point : points)
				{
					Transform* t = transforms[_spline][j];
					t->setPosition(trans1->getTransformMatrix() * glm::vec4(point, 1.0f));

					++j;
				}
			}
		}
	}

	void SplineEditor::updatePoints()
	{
		if (pointsGroup != nullptr)
			removeProperty(pointsGroup);

		pointsGroup = new Property(this, "Points");
		pointsGroup->setHasButton(true);
		pointsGroup->setButtonText("+");
		pointsGroup->setOnButtonClickCallback([=](TreeNode* prop) { onAddPoint(); });

		addProperty(pointsGroup);

		Spline* spline = (Spline*)components[0];
		std::vector<glm::vec3>& points = spline->getPoints();

		int i = 0;
		for (auto& point : points)
		{
			PropVector3* pointProp = new PropVector3(this, "Point " + std::to_string(i), point);
			pointProp->setOnChangeCallback([=](Property* prop, glm::vec3 val) { onChangePoint(i, val); });
			pointProp->setPopupMenu({ "Remove" }, [=](TreeNode* node, int val) { onPointPopup(i, val); });

			pointsGroup->addChild(pointProp);

			++i;
		}
	}

	void SplineEditor::updateGizmo()
	{
		Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();

		MainWindow::addOnEndUpdateCallback([=]
			{
				std::vector<Transform*> comps;
				for (auto& spline : components)
					comps.push_back(spline->getGameObject()->getTransform());

				gizmo->selectObjects(comps, nullptr, false);

				comps.clear();
			}
		);
	}

	void SplineEditor::onChangeTransformPosition(Transform* t, Spline* spline, int index)
	{
		Transform* mt = spline->getGameObject()->getTransform();
		auto& points = spline->getPoints();
		points[index] = mt->getTransformMatrixInverse() * glm::vec4(t->getPosition(), 1.0f);
	}

	void SplineEditor::onGizmoManipulateStart(std::vector<Transform*> nodes)
	{
		int cnt = 0;
		for (auto& obj : nodes)
		{
			auto it = std::find_if(transforms.begin(), transforms.end(), [=](std::pair<Spline* const, std::vector<Transform*>>& v) -> bool
				{
					return (std::find(v.second.begin(), v.second.end(), obj) != v.second.end());
				}
			);

			if (it == transforms.end())
				continue;

			++cnt;
		}

		if (cnt == 0)
			return;

		undoData = Undo::addUndo("Move spline point");
		undoData->vec3ListData.resize(2);

		for (auto& obj : nodes)
		{
			auto it = std::find_if(transforms.begin(), transforms.end(), [=](std::pair<Spline* const, std::vector<Transform*>>& v) -> bool
				{
					return (std::find(v.second.begin(), v.second.end(), obj) != v.second.end());
				}
			);

			if (it == transforms.end())
				continue;

			auto tt = std::find(it->second.begin(), it->second.end(), obj);
			int ind = std::distance(it->second.begin(), tt);

			Transform* mt = it->first->getGameObject()->getTransform();
			undoData->vec3ListData[0][it->first][ind] = mt->getTransformMatrixInverse() * glm::vec4(obj->getPosition(), 1.0f);
		}

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& obj : data->vec3ListData[0])
			{
				auto& pts = ((Spline* const)obj.first)->getPoints();
				for (auto& p : obj.second)
					pts[p.first] = p.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->vec3ListData[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};
	}

	void SplineEditor::onGizmoManipulateEnd(std::vector<Transform*> nodes)
	{
		if (undoData == nullptr)
			return;

		for (auto& obj : nodes)
		{
			auto it = std::find_if(transforms.begin(), transforms.end(), [=](std::pair<Spline* const, std::vector<Transform*>>& v) -> bool
				{
					return (std::find(v.second.begin(), v.second.end(), obj) != v.second.end());
				}
			);

			if (it == transforms.end())
				continue;

			auto tt = std::find(it->second.begin(), it->second.end(), obj);
			int ind = std::distance(it->second.begin(), tt);

			Transform* mt = it->first->getGameObject()->getTransform();
			undoData->vec3ListData[1][it->first][ind] = mt->getTransformMatrixInverse() * glm::vec4(obj->getPosition(), 1.0f);
		}

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& obj : data->vec3ListData[1])
			{
				auto& pts = ((Spline* const)obj.first)->getPoints();
				for (auto& p : obj.second)
					pts[p.first] = p.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->vec3ListData[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};

		undoData = nullptr;
	}

	void SplineEditor::drawGizmos(int viewId, int viewLayer, Camera* camera)
	{
		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CCW;

		//int i = 0;
		for (auto& spline1 : components)
		{
			Spline* spline = (Spline*)spline1;
			Transform* trans = spline->getGameObject()->getTransform();

			std::vector<glm::vec3> points;

			for (float t = 0.0f; t < 0.995f; t += 0.005f)
			{
				glm::vec3 p1 = spline->getSplinePoint(t);
				glm::vec3 p2 = spline->getSplinePoint(t + 0.005f);

				glm::mat4x4 mtx = glm::identity<glm::mat4x4>();
				Primitives::line(mtx, p1, p2, Color(1, 1, 0, 1), viewId, state | BGFX_STATE_PT_LINES, Renderer::getSingleton()->getTransparentProgram(), camera);
			}

			points.clear();

			Transform* camTrans = camera->getTransform();

			auto& pts = spline->getPoints();

			while (transforms[spline].size() < pts.size())
			{
				int j = transforms[spline].size();

				Transform* t = new Transform();
				t->setPosition(trans->getPosition());
				transforms[spline].push_back(t);
				t->setOnChangeCallback([=]() { onChangeTransformPosition(t, spline, j); });
			}

			for (int i = 0; i < pts.size(); ++i)
			{
				Transform* ct = transforms[spline][i];

				auto& sel = MainWindow::getSceneWindow()->getGizmo()->getSelectedObjects();
				bool found = std::find(sel.begin(), sel.end(), ct) != sel.end();

				float dist = glm::distance(camTrans->getPosition(), ct->getPosition());
				Color color = Color::Blue;
				if (found)
					color = Color(0.0f, 0.7f, 0.0f, 1.0f);

				Primitives::billboard(ct, splinePointTex, color, glm::abs(dist * 0.02f), viewId, state, Renderer::getSingleton()->getTransparentProgram(), camera);
			}
		}
	}

	void SplineEditor::onChangeClosed(bool val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change spline closed");
		undoData->boolData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[0])
			{
				Spline* comp = (Spline*)d.first;
				comp->setClosed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->boolData[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->boolData[1])
			{
				Spline* comp = (Spline*)d.first;
				comp->setClosed(d.second);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->boolData[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};
		//

		for (auto& spline1 : components)
		{
			Spline* spline = (Spline*)spline1;

			undoData->boolData[0][spline] = spline->getClosed();
			undoData->boolData[1][spline] = val;

			spline->setClosed(val);
		}
	}

	void SplineEditor::onAddPoint()
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Add spline point");
		undoData->objectData.resize(1);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Spline* comp = (Spline*)d;
				std::vector<glm::vec3>& pts = comp->getPoints();
				pts.erase(pts.begin() + pts.size() - 1);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->objectData[0])
			{
				Spline* comp = (Spline*)d;
				std::vector<glm::vec3>& pts = comp->getPoints();

				if (pts.size() == 0)
					pts.push_back(glm::vec3(0, 0, 0));
				else
					pts.push_back(pts[pts.size() - 1]);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			MainWindow::getSceneWindow()->getGizmo()->updatePosition();
		};
		//

		for (auto& spline1 : components)
		{
			Spline* spline = (Spline*)spline1;

			undoData->objectData[0].push_back(spline);

			std::vector<glm::vec3>& points = spline->getPoints();

			if (points.size() == 0)
				points.push_back(glm::vec3(0, 0, 0));
			else
				points.push_back(points[points.size() - 1]);
		}

		updateGizmo();
		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void SplineEditor::onPointPopup(int idx, int popup)
	{
		if (popup == 0)
		{
			//Undo
			UndoData* undoData = Undo::addUndo("Remove spline point");
			undoData->vec3Data.resize(1);
			undoData->intData.resize(1);

			undoData->intData[0][nullptr] = idx;

			undoData->undoAction = [=](UndoData* data)
			{
				int ind = data->intData[0][nullptr];

				for (auto& d : data->vec3Data[0])
				{
					Spline* comp = (Spline*)d.first;
					std::vector<glm::vec3>& pts = comp->getPoints();

					pts.insert(pts.begin() + ind, d.second);
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::getSceneWindow()->getGizmo()->updatePosition();
			};

			undoData->redoAction = [=](UndoData* data)
			{
				int ind = data->intData[0][nullptr];

				for (auto& d : data->vec3Data[0])
				{
					Spline* comp = (Spline*)d.first;
					std::vector<glm::vec3>& pts = comp->getPoints();
					pts.erase(pts.begin() + ind);
				}

				MainWindow::getInspectorWindow()->updateCurrentEditor();
				MainWindow::getSceneWindow()->getGizmo()->updatePosition();
			};
			//

			for (auto& spline1 : components)
			{
				Spline* spline = (Spline*)spline1;
				std::vector<glm::vec3>& points = spline->getPoints();

				undoData->vec3Data[0][spline] = points[idx];

				points.erase(points.begin() + idx);
			}
		}

		updateGizmo();
		MainWindow::getInspectorWindow()->updateCurrentEditor();
	}

	void SplineEditor::onChangePoint(int idx, glm::vec3 val)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change spline point");
		undoData->vec3Data.resize(2);
		undoData->intData.resize(1);

		undoData->intData[0][nullptr] = idx;

		undoData->undoAction = [=](UndoData* data)
		{
			int ind = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[0])
			{
				Spline* comp = (Spline*)d.first;
				Transform* mt = comp->getGameObject()->getTransform();
				auto& pts = comp->getPoints();
				pts[ind] = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->vec3Data[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			int ind = data->intData[0][nullptr];

			for (auto& d : data->vec3Data[1])
			{
				Spline* comp = (Spline*)d.first;
				Transform* mt = comp->getGameObject()->getTransform();
				auto& pts = comp->getPoints();
				pts[ind] = d.second;
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
			
			std::vector<Transform*> comps;
			for (auto& obj : data->vec3Data[0])
				comps.push_back(((Spline*)obj.first)->getGameObject()->getTransform());

			Gizmo* gizmo = MainWindow::getSceneWindow()->getGizmo();
			gizmo->selectObjects(comps, nullptr, false);
			comps.clear();
		};
		//

		int i = 0;
		for (auto& spline1 : components)
		{
			Spline* spline = (Spline*)spline1;
			Transform* mt = spline->getGameObject()->getTransform();
			std::vector<glm::vec3>& points = spline->getPoints();

			undoData->vec3Data[0][spline] = points[idx];
			undoData->vec3Data[1][spline] = val;
			
			points[idx] = val;
			transforms[spline][idx]->setPosition(mt->getTransformMatrix() * glm::vec4(val, 1.0f));

			++i;
		}
	}
}