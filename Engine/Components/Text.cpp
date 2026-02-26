#include "Text.h"

#include <imgui.h>

#include "../Core/APIManager.h"
#include "../Components/Canvas.h"
#include "../Assets/Font.h"
#include "../Assets/Texture.h"
#include "../Classes/StringConverter.h"

#include <codecvt>
#include <algorithm>

#include <boost/algorithm/string.hpp>

namespace GX
{
	std::string Text::COMPONENT_TYPE = "Text";

	Text::Text() : UIElement(APIManager::getSingleton()->text_class)
	{
	}

	Text::~Text()
	{
		
	}

	std::string Text::getComponentType()
	{
		return COMPONENT_TYPE;
	}

	Component* Text::onClone()
	{
		Text* newComponent = new Text();
		newComponent->enabled = enabled;
		newComponent->anchor = anchor;
		newComponent->color = color;
		newComponent->size = size;
		newComponent->horizontalAlignment = horizontalAlignment;
		newComponent->verticalAlignment = verticalAlignment;
		newComponent->text = text;
		newComponent->str32 = str32;
		newComponent->fontSize = fontSize;
		newComponent->fontResolution = fontResolution;
		newComponent->lineSpacing = lineSpacing;
		newComponent->font = font;
		newComponent->horizontalTextAlignment = horizontalTextAlignment;
		newComponent->verticalTextAlignment = verticalTextAlignment;
		newComponent->wordWrap = wordWrap;
		newComponent->raycastTarget = raycastTarget;

		return newComponent;
	}

	void Text::onRender(ImDrawList* drawList)
	{
		UIElement::onRender(drawList);

		float zoom = 1.0f;
		
		if (canvas != nullptr)
			zoom = canvas->getZoom();

		glm::vec4 rect = getRect();

		render(drawList, rect, str32, font, fontSize, (float)fontResolution, lineSpacing, wordWrap, horizontalTextAlignment, verticalTextAlignment, color, zoom);
	}

	void Text::render(ImDrawList* drawList,
		glm::vec4 rect,
		std::u32string str32,
		Font* font,
		float fontSize,
		float fontResolution,
		float lineSpacing,
		TextWordWrap wordWrap,
		TextHorizontalAlignment horizontalTextAlignment,
		TextVerticalAlignment verticalTextAlignment,
		Color color,
		float zoom)
	{
		ImU32 elemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r(), color.g(), color.b(), color.a()));
		const ImVec4 clipRect = ImVec4(rect.x, rect.y, rect.z, rect.w);

		Font* _font = font;
		
		if (_font == nullptr)
			_font = Font::getDefaultFont();

		if (_font == nullptr)
			return;

		FontAtlas* fontAtlas = _font->getFontAtlas(fontResolution);
		float scale = fontSize / fontResolution;

		if (fontAtlas != nullptr && fontAtlas->texture != nullptr)
		{
			Texture* atlasTexture = fontAtlas->texture;

			if (atlasTexture->getHandle().idx == bgfx::kInvalidHandle)
				return;

			//Calc sizes for each line
			std::vector<std::pair<float, float>> lineSizes; // Width, height
			float lw = 0;
			float lh = 0;
			for (int i = 0; i < str32.length(); ++i)
			{
				if (str32[i] == ' ')
				{
					float advance = fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

					if (wordWrap == TextWordWrap::BreakWord)
					{
						float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

						if (lw + advance > rw)
						{
							lineSizes.push_back(std::make_pair(lw, lh));
							lw = 0;
							lh = 0;
						}
						else
						{
							lw += advance;
							lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
						}
					}
					else if (wordWrap == TextWordWrap::WordWrap)
					{
						if (i + 1 < str32.length())
						{
							float wordW = advance;
							for (int j = i + 1; j < str32.length(); ++j)
							{
								if (str32[j] == ' ' || str32[j] == '\n')
									break;

								if (fontAtlas->glyphs.find(uint_least32_t(str32[j])) == fontAtlas->glyphs.end())
									continue;

								wordW += fontAtlas->glyphs[str32[j]].advance * scale * zoom;
							}

							float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

							if (lw + wordW > rw)
							{
								lineSizes.push_back(std::make_pair(lw, lh));
								lw = 0;
								lh = 0;
							}
							else
							{
								lw += advance;
								lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
							}
						}
						else
						{
							lw += advance;
							lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
						}
					}
					else
					{
						lw += advance;
						lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
					}

					continue;
				}

				if (str32[i] == '\n')
				{
					lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
					lineSizes.push_back(std::make_pair(lw, lh));
					lw = 0;
					lh = 0;
					continue;
				}

				int charCode = uint_least32_t(str32[i]);
				if (fontAtlas->glyphs.find(charCode) == fontAtlas->glyphs.end())
					continue;

				GlyphInfo& glyph = fontAtlas->glyphs[charCode];
				float advance = glyph.advance * scale * zoom;

				if (wordWrap == TextWordWrap::BreakWord)
				{
					float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

					if (lw + advance > rw)
					{
						if (i > 0 && str32[i - 1] == ' ')
							lw -= fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

						lineSizes.push_back(std::make_pair(lw, lh));
						lw = 0;
						lh = 0;
					}
				}

				lw += advance;
				lh = std::max(glyph.size.y * scale * zoom, lh);
			}
			lineSizes.push_back(std::make_pair(lw, lh));

			//Draw text
			drawList->PushClipRect(ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), true);
			glm::vec2 cursor = glm::vec2(0, lineSizes[0].second);
			int line = 0;
			for (int i = 0; i < str32.length(); ++i)
			{
				float proportion = 1.0f / fontAtlas->texture->getWidth();

				int charCode = uint_least32_t(str32[i]);

				if (str32[i] == ' ')
				{
					float advance = fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

					if (wordWrap == TextWordWrap::BreakWord)
					{
						float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

						if (cursor.x + advance > rw)
						{
							cursor.x = 0.0f;
							cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
							++line;
						}
						else
							cursor.x += advance;
					}
					else if (wordWrap == TextWordWrap::WordWrap)
					{
						if (i + 1 < str32.length())
						{
							float wordW = advance;
							for (int j = i + 1; j < str32.length(); ++j)
							{
								if (str32[j] == ' ' || str32[j] == '\n')
									break;

								if (fontAtlas->glyphs.find(uint_least32_t(str32[j])) == fontAtlas->glyphs.end())
									continue;

								wordW += fontAtlas->glyphs[str32[j]].advance * scale * zoom;
							}

							float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

							if (cursor.x + wordW > rw)
							{
								cursor.x = 0.0f;
								cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
								++line;
							}
							else
								cursor.x += advance;
						}
						else
							cursor.x += advance;
					}
					else
						cursor.x += advance;

					continue;
				}

				if (str32[i] == '\n')
				{
					cursor.x = 0;
					cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
					++line;
					continue;
				}

				if (fontAtlas->glyphs.find(charCode) == fontAtlas->glyphs.end())
					continue;

				GlyphInfo& glyph = fontAtlas->glyphs[charCode];
				float advance = glyph.advance * scale * zoom;

				if (wordWrap == TextWordWrap::BreakWord)
				{
					float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

					if (cursor.x + advance > rw)
					{
						cursor.x = 0.0f;
						cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
						++line;
					}
				}

				float hAlign = 0;
				float vAlign = 0;

				//Horizontal alignment. Default: left
				if (horizontalTextAlignment == TextHorizontalAlignment::Center)
					hAlign = ((rect.z - rect.x) * 0.5f) - (lineSizes[line].first * 0.5f);
				if (horizontalTextAlignment == TextHorizontalAlignment::Right)
					hAlign = (rect.z - rect.x) - lineSizes[line].first;

				//Vertical alignment. Default: top
				if (verticalTextAlignment == TextVerticalAlignment::Middle)
				{
					float lineHeight = fontAtlas->maxGlyphHeight * scale * zoom;
					float textHeight = (lineSizes.size() * lineHeight) + ((lineSizes.size() - 1) * lineSpacing * scale * zoom) - (lineSizes[0].second * 0.25f);
					vAlign = ((rect.w - rect.y) * 0.5f) - (textHeight * 0.5f);
				}
				if (verticalTextAlignment == TextVerticalAlignment::Bottom)
				{
					float lineHeight = (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
					float textHeight = (lineSizes.size() * lineHeight) - lineSizes[0].second * 0.25f;
					vAlign = (rect.w - rect.y) - textHeight;
				}

				//Calc glyph position
				glm::vec2 glyphSize = glyph.size * scale * zoom;
				glm::vec4 offset = glm::vec4(0, glyph.yOffset, 0, glyph.yOffset) * scale * zoom;
				glm::vec4 alignment = glm::vec4(hAlign, vAlign, hAlign, vAlign);
				glm::vec4 glyphRect = glm::vec4(rect.x, rect.y, rect.x, rect.y) - offset + alignment + glm::vec4(cursor, cursor + glyphSize);
				glm::vec4 glyphUv = glyph.rect * proportion;

				cursor.x += advance;

				//Draw glyph
				drawList->AddImage((void*)atlasTexture->getHandle().idx,
					ImVec2(glyphRect.x, glyphRect.y),
					ImVec2(glyphRect.z, glyphRect.w),
					ImVec2(glyphUv.x, glyphUv.y),
					ImVec2(glyphUv.z, glyphUv.w),
					elemColor
				);
			}
			drawList->PopClipRect();

			lineSizes.clear();
		}
	}

	std::vector<glm::vec2> Text::getCharPositions(glm::vec4 rect,
		std::u32string text,
		Font* font,
		float fontSize,
		float fontResolution,
		float lineSpacing,
		TextWordWrap wordWrap,
		TextHorizontalAlignment horizontalTextAlignment,
		TextVerticalAlignment verticalTextAlignment,
		float zoom)
	{
		std::u32string str32 = text;
		if (str32.length() == 0)
			str32 = std::u32string(U"i");

		std::vector<glm::vec2> charPos;

		Font* _font = font;

		if (_font == nullptr)
			_font = Font::getDefaultFont();

		FontAtlas* fontAtlas = nullptr;

		if (_font != nullptr)
			fontAtlas = _font->getFontAtlas(fontResolution);

		float scale = fontSize / fontResolution;

		if (fontAtlas != nullptr &&
			fontAtlas->texture != nullptr &&
			fontAtlas->texture->getHandle().idx != bgfx::kInvalidHandle)
		{
			Texture* atlasTexture = fontAtlas->texture;

			//Calc sizes for each line
			std::vector<std::pair<float, float>> lineSizes; // Width, height
			float lw = 0;
			float lh = 0;
			for (int i = 0; i < str32.length(); ++i)
			{
				if (str32[i] == ' ')
				{
					float advance = fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

					if (wordWrap == TextWordWrap::BreakWord)
					{
						float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

						if (lw + advance > rw)
						{
							lineSizes.push_back(std::make_pair(lw, lh));
							lw = 0;
							lh = 0;
						}
						else
						{
							lw += advance;
							lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
						}
					}
					else if (wordWrap == TextWordWrap::WordWrap)
					{
						if (i + 1 < str32.length())
						{
							float wordW = advance;
							for (int j = i + 1; j < str32.length(); ++j)
							{
								if (str32[j] == ' ' || str32[j] == '\n')
									break;

								if (fontAtlas->glyphs.find(uint_least32_t(str32[j])) == fontAtlas->glyphs.end())
									continue;

								wordW += fontAtlas->glyphs[str32[j]].advance * scale * zoom;
							}

							float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

							if (lw + wordW > rw)
							{
								lineSizes.push_back(std::make_pair(lw, lh));
								lw = 0;
								lh = 0;
							}
							else
							{
								lw += advance;
								lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
							}
						}
						else
						{
							lw += advance;
							lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
						}
					}
					else
					{
						lw += advance;
						lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
					}

					continue;
				}

				if (str32[i] == '\n')
				{
					lh = std::max(fontAtlas->glyphs[(int)'i'].size.y * scale * zoom, lh);
					lineSizes.push_back(std::make_pair(lw, lh));
					lw = 0;
					lh = 0;
					continue;
				}

				int charCode = uint_least32_t(str32[i]);
				if (fontAtlas->glyphs.find(charCode) == fontAtlas->glyphs.end())
					continue;

				GlyphInfo& glyph = fontAtlas->glyphs[charCode];
				float advance = glyph.advance * scale * zoom;

				if (wordWrap == TextWordWrap::BreakWord)
				{
					float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

					if (lw + advance > rw)
					{
						if (i > 0 && str32[i - 1] == ' ')
							lw -= fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

						lineSizes.push_back(std::make_pair(lw, lh));
						lw = 0;
						lh = 0;
					}
				}

				lw += advance;
				lh = std::max(glyph.size.y * scale * zoom, lh);
			}
			lineSizes.push_back(std::make_pair(lw, lh));

			//Calc positions
			glm::vec2 cursor = glm::vec2(0, lineSizes[0].second);
			int line = 0;
			for (int i = 0; i < str32.length(); ++i)
			{
				float proportion = 1.0f / fontAtlas->texture->getWidth();

				int charCode = uint_least32_t(str32[i]);

				GlyphInfo glyph = fontAtlas->glyphs[(int)'i'];
				
				if (str32[i] != '\n')
				{
					if (str32[i] == ' ')
					{
						glyph = fontAtlas->glyphs[(int)'i'];
					}
					else
					{
						if (fontAtlas->glyphs.find(charCode) == fontAtlas->glyphs.end())
							continue;

						glyph = fontAtlas->glyphs[charCode];
					}
				}

				float advance = glyph.advance * scale * zoom;

				float hAlign = 0;
				float vAlign = 0;

				//Horizontal alignment. Default: left
				if (horizontalTextAlignment == TextHorizontalAlignment::Center)
					hAlign = ((rect.z - rect.x) * 0.5f) - (lineSizes[line].first * 0.5f);
				if (horizontalTextAlignment == TextHorizontalAlignment::Right)
					hAlign = (rect.z - rect.x) - lineSizes[line].first;

				//Vertical alignment. Default: top
				if (verticalTextAlignment == TextVerticalAlignment::Middle)
				{
					float lineHeight = fontAtlas->maxGlyphHeight * scale * zoom;
					float textHeight = (lineSizes.size() * lineHeight) + ((lineSizes.size() - 1) * lineSpacing * scale * zoom) - (lineSizes[0].second * 0.25f);
					vAlign = ((rect.w - rect.y) * 0.5f) - (textHeight * 0.5f);
				}
				if (verticalTextAlignment == TextVerticalAlignment::Bottom)
				{
					float lineHeight = (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
					float textHeight = (lineSizes.size() * lineHeight) - lineSizes[0].second * 0.25f;
					vAlign = (rect.w - rect.y) - textHeight;
				}

				//Calc glyph position
				glm::vec2 glyphSize = glyph.size * scale * zoom;
				glm::vec4 offset = glm::vec4(0, glyph.yOffset, 0, glyph.yOffset) * scale * zoom;
				glm::vec4 alignment = glm::vec4(hAlign, vAlign, hAlign, vAlign);
				glm::vec4 glyphRect = glm::vec4(rect.x, rect.y, rect.x, rect.y) - offset + alignment + glm::vec4(cursor, cursor + glyphSize);

				charPos.push_back(glm::vec2(glyphRect.x, glyphRect.y));

				if (str32[i] != '\n')
					cursor.x += advance;

				if (str32[i] == ' ')
				{
					float advance = fontAtlas->glyphs[(int)'i'].advance * scale * zoom;

					if (wordWrap == TextWordWrap::BreakWord)
					{
						float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

						if (cursor.x > rw)
						{
							cursor.x = 0.0f;
							cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
							++line;
						}
					}
					else if (wordWrap == TextWordWrap::WordWrap)
					{
						if (i + 1 < str32.length())
						{
							float wordW = 0.0f;
							for (int j = i + 1; j < str32.length(); ++j)
							{
								if (str32[j] == ' ' || str32[j] == '\n')
									break;

								if (fontAtlas->glyphs.find(uint_least32_t(str32[j])) == fontAtlas->glyphs.end())
									continue;

								wordW += fontAtlas->glyphs[str32[j]].advance * scale * zoom;
							}

							float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

							if (cursor.x + wordW > rw)
							{
								cursor.x = 0.0f;
								cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
								++line;
							}
						}
					}
				}

				if (str32[i] != '\n')
				{
					if (wordWrap == TextWordWrap::BreakWord)
					{
						float rw = std::max(rect.z - rect.x, fontAtlas->maxGlyphWidth * scale * zoom);

						if (cursor.x > rw)
						{
							cursor.x = advance;
							cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
							++line;
						}
					}
				}

				if (str32[i] == '\n')
				{
					cursor.x = 0;
					cursor.y += (fontAtlas->maxGlyphHeight + lineSpacing) * scale * zoom;
					++line;
				}

				if (i == str32.length() - 1)
				{
					cursor.x -= advance;
					glyphRect = glm::vec4(rect.x, rect.y, rect.x, rect.y) - offset + alignment + glm::vec4(cursor, cursor + glyphSize);
					charPos.push_back(glm::vec2(glyphRect.z, glyphRect.y));
				}
			}

			lineSizes.clear();
		}

		return charPos;
	}

	void Text::setText(std::string value)
	{
		text = value;
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
		str32 = cv.from_bytes(text);
	}

	void Text::setFontSize(float value)
	{
		fontSize = value;
	}

	void Text::setFontResolution(int value)
	{
		if (value == 0) return;
		if (value > 128) return;

		Font* _font = font;

		if (_font == nullptr)
			_font = Font::getDefaultFont();

		if (_font != nullptr)
		{
			fontResolution = value;
			_font->clearFontAtlases();
		}
	}

	void Text::setFont(Font* value)
	{
		font = value;
	}
}