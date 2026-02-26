#include "ImageEditor.h"

#include "../Engine/Core/Engine.h"
#include "../Engine/Components/Image.h"
#include "../Engine/Assets/Texture.h"

#include "../Windows/MainWindow.h"
#include "../Windows/AssetsWindow.h"
#include "../Windows/InspectorWindow.h"

#include "PropSampler.h"
#include "PropComboBox.h"

#include "../Classes/Undo.h"

namespace GX
{
	ImageEditor::ImageEditor()
	{
		setEditorName("ImageEditor");
	}

	void ImageEditor::init(std::vector<Component*> comps)
	{
		UIElementEditor::init(comps);

		Image* image = (Image*)comps[0];

		PropComboBox* imageType = new PropComboBox(this, "Image type", { "Simple", "Sliced" });
		imageType->setCurrentItem(static_cast<int>(image->getImageType()));
		imageType->setOnChangeCallback([=](Property* prop, int val) { onChangeImageType(prop, val); });

		PropSampler* texture = new PropSampler(this, "Texture", image->getTexture());
		texture->setSupportedFormats(Engine::getImageFileFormats());
		texture->setOnDropCallback([=](TreeNode* prop, TreeNode* from) { onChangeTexture(prop, from); });
		texture->setOnClearCallback([=](Property* prop) { onClearTexture(prop); });
		texture->setOnClickCallback([=](Property* prop)
			{
				Texture* tex = image->getTexture();
				if (tex != nullptr)
					MainWindow::getSingleton()->getAssetsWindow()->focusOnFile(tex->getName());
			}
		);

		addProperty(imageType);
		addProperty(texture);
	}

	void ImageEditor::onChangeTexture(TreeNode* prop, TreeNode* from)
	{
		std::string path = from->getPath();
		Texture* tex = Texture::load(Engine::getSingleton()->getAssetsPath(), path, false, Texture::CompressionMethod::None);
		((PropSampler*)prop)->setValue(tex);

		//Undo
		UndoData* undoData = Undo::addUndo("Change image texture");
		undoData->stringData.resize(4);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Image* element = (Image*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTexture(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[2])
			{
				Image* element = (Image*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[3][element], false, Texture::CompressionMethod::None);
				element->setTexture(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Image* image = (Image*)*it;

			Texture* texture = image->getTexture();
			undoData->stringData[0][image] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][image] = texture != nullptr ? texture->getName() : "";
			undoData->stringData[2][image] = Engine::getSingleton()->getAssetsPath();
			undoData->stringData[3][image] = path;

			image->setTexture(tex);
		}
	}

	void ImageEditor::onClearTexture(TreeNode* prop)
	{
		((PropSampler*)prop)->setValue(nullptr);

		//Undo
		UndoData* undoData = Undo::addUndo("Clear image texture");
		undoData->stringData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Image* element = (Image*)d.first;

				Texture* texture = Texture::load(d.second, data->stringData[1][element], false, Texture::CompressionMethod::None);
				element->setTexture(texture);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->stringData[0])
			{
				Image* element = (Image*)d.first;
				element->setTexture(nullptr);
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Image* image = (Image*)*it;

			Texture* texture = image->getTexture();
			undoData->stringData[0][image] = texture != nullptr ? texture->getLocation() : "";
			undoData->stringData[1][image] = texture != nullptr ? texture->getName() : "";

			image->setTexture(nullptr);
		}
	}

	void ImageEditor::onChangeImageType(Property* prop, int value)
	{
		//Undo
		UndoData* undoData = Undo::addUndo("Change image type");
		undoData->intData.resize(2);

		undoData->undoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[0])
			{
				Image* element = (Image*)d.first;
				element->setImageType(static_cast<Image::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};

		undoData->redoAction = [=](UndoData* data)
		{
			for (auto& d : data->intData[1])
			{
				Image* element = (Image*)d.first;
				element->setImageType(static_cast<Image::ImageType>(d.second));
			}

			MainWindow::getInspectorWindow()->updateCurrentEditor();
		};
		//

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			Image* image = (Image*)*it;

			undoData->intData[0][image] = static_cast<int>(image->getImageType());
			undoData->intData[1][image] = value;

			image->setImageType(static_cast<Image::ImageType>(value));
		}
	}
}