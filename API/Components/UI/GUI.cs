using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace FalcoEngine
{
    public static class GUI
    {
        /// <summary>
        /// ImageType enum
        /// </summary>
        public enum ImageType
        {
            Simple,
            Sliced
        };

        /// <summary>
        /// TextHorizontalAlignment enum
        /// </summary>
        public enum TextHorizontalAlignment
        {
            Left,
            Center,
            Right
        };

        /// <summary>
        /// TextVerticalAlignment enum
        /// </summary>
        public enum TextVerticalAlignment
        {
            Top,
            Middle,
            Bottom
        };

        /// <summary>
        /// TextWordWrap enum
        /// </summary>
        public enum TextWordWrap
        {
            None,
            WordWrap,
            BreakWord
        };

        /// <summary>
        /// Draw text
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        public static void Text(string text, Rect rect)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, null, 14, 32, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        public static void Text(string text, Rect rect, Font font)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, 14, 32, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font and font size
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        public static void Text(string text, Rect rect, Font font, float fontSize)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, fontSize, 32, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size and font resolution
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size, font resolution and line spacing
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        /// <param name="lineSpacing">Line spacing</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution, float lineSpacing)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, lineSpacing, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size, font resolution, line spacing and word wrap
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        /// <param name="lineSpacing">Line spacing</param>
        /// <param name="wordWrap">Word wrap</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution, float lineSpacing, TextWordWrap wordWrap)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, lineSpacing, (int)wordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size, font resolution, line spacing, word wrap, horizontal alignment and vertical alignment
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        /// <param name="lineSpacing">Line spacing</param>
        /// <param name="wordWrap">Word wrap</param>
        /// <param name="horizontalAlignment">Horizontal alignment</param>
        /// <param name="verticalAlignment">Vertical alignment</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution, float lineSpacing, TextWordWrap wordWrap, TextHorizontalAlignment horizontalAlignment, TextVerticalAlignment verticalAlignment)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, lineSpacing, (int)wordWrap, (int)horizontalAlignment, (int)verticalAlignment, ref color);
        }

        /// <summary>
        /// Draw text with font, font size, font resolution, line spacing, word wrap, horizontal alignment, vertical alignment and text color
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        /// <param name="lineSpacing">Line spacing</param>
        /// <param name="wordWrap">Word wrap</param>
        /// <param name="horizontalAlignment">Horizontal alignment</param>
        /// <param name="verticalAlignment">Vertical alignment</param>
        /// <param name="color">Text color</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution, float lineSpacing, TextWordWrap wordWrap, TextHorizontalAlignment horizontalAlignment, TextVerticalAlignment verticalAlignment, Color color)
        {
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, lineSpacing, (int)wordWrap, (int)horizontalAlignment, (int)verticalAlignment, ref color);
        }

        /// <summary>
        /// Draw text with font size and text color
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="color">Text color</param>
        public static void Text(string text, Rect rect, float fontSize, Color color)
        {
            INTERNAL_text(text, ref rect, null, fontSize, 32, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size and text color
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="color">Text color</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, Color color)
        {
            INTERNAL_text(text, ref rect, font, fontSize, 32, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw text with font, font size, font resolution and text color
        /// </summary>
        /// <param name="text">Text to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="font">Font</param>
        /// <param name="fontSize">Font size</param>
        /// <param name="fontResolution">Font resolution</param>
        /// <param name="color">Text color</param>
        public static void Text(string text, Rect rect, Font font, float fontSize, float fontResolution, Color color)
        {
            INTERNAL_text(text, ref rect, font, fontSize, fontResolution, 0.0f, (int)TextWordWrap.WordWrap, (int)TextHorizontalAlignment.Left, (int)TextVerticalAlignment.Top, ref color);
        }

        /// <summary>
        /// Draw image
        /// </summary>
        /// <param name="texture">Texture to draw</param>
        /// <param name="rect">Rect on the screen</param>
        public static void Image(Texture texture, Rect rect)
        {
            Color color = new Color(1, 1, 1, 1);
            INTERNAL_image(texture, ref rect, ref color, (int)ImageType.Simple);
        }

        /// <summary>
        /// Draw image with color tint
        /// </summary>
        /// <param name="texture">Texture to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="color">Color tint</param>
        public static void Image(Texture texture, Rect rect, Color color)
        {
            INTERNAL_image(texture, ref rect, ref color, (int)ImageType.Simple);
        }

        /// <summary>
        /// Draw image with color tint and image type
        /// </summary>
        /// <param name="texture">Texture to draw</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="color">Color tint</param>
        /// <param name="imageType">Image type</param>
        public static void Image(Texture texture, Rect rect, Color color, ImageType imageType)
        {
            INTERNAL_image(texture, ref rect, ref color, (int)imageType);
        }

        /// <summary>
        /// Draw enabled or disabled button
        /// </summary>
        /// <param name="id">Button id</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="interactable">Button interactability</param>
        /// <returns></returns>
        public static bool Button(int id, Rect rect, bool interactable)
        {
            Color color = Color.white;
            Color colorHov = new Color(0.8f, 0.8f, 0.8f, 1.0f);
            Color colorAct = new Color(0.6f, 0.6f, 0.6f, 1.0f);
            Color colorDis = new Color(0.4f, 0.4f, 0.4f, 1.0f);

            return INTERNAL_button(id, ref rect, null, null, null, null, ref color, ref colorHov, ref colorAct, ref colorDis, (int)ImageType.Simple, interactable);
        }

        /// <summary>
        /// Draw enabled or disabled button with tint colors
        /// </summary>
        /// <param name="id">Button id</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="normalColor">Normal color</param>
        /// <param name="hoverColor">Hover color</param>
        /// <param name="activeColor">Active color</param>
        /// <param name="disabledColor">Disabled color</param>
        /// <param name="interactable">Button interactability</param>
        /// <returns></returns>
        public static bool Button(int id, Rect rect, Color normalColor, Color hoverColor, Color activeColor, Color disabledColor, bool interactable)
        {
            return INTERNAL_button(id, ref rect, null, null, null, null, ref normalColor, ref hoverColor, ref activeColor, ref disabledColor, (int)ImageType.Simple, interactable);
        }

        /// <summary>
        /// Draw enabled or disabled button with textures and tint colors
        /// </summary>
        /// <param name="id">Button id</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="normalColor">Normal color</param>
        /// <param name="hoverColor">Hover color</param>
        /// <param name="activeColor">Active color</param>
        /// <param name="disabledColor">Disabled color</param>
        /// <param name="normalTexture">Normal texture</param>
        /// <param name="hoverTexture">Hover texture</param>
        /// <param name="activeTexture">Active texture</param>
        /// <param name="disabledTexture">Disabled texture</param>
        /// <param name="imageType">Image type</param>
        /// <param name="interactable">Button interactability</param>
        /// <returns></returns>
        public static bool Button(int id,
            Rect rect,
            Color normalColor,
            Color hoverColor,
            Color activeColor,
            Color disabledColor,
            Texture normalTexture,
            Texture hoverTexture,
            Texture activeTexture,
            Texture disabledTexture,
            ImageType imageType,
            bool interactable)
        {
            return INTERNAL_button(id, ref rect, normalTexture, hoverTexture, activeTexture, disabledTexture, ref normalColor, ref hoverColor, ref activeColor, ref disabledColor, (int)imageType, interactable);
        }

        /// <summary>
        /// Draw enabled or disabled button with textures
        /// </summary>
        /// <param name="id">Button id</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="normalTexture">Normal texture</param>
        /// <param name="hoverTexture">Hover texture</param>
        /// <param name="activeTexture">Active texture</param>
        /// <param name="disabledTexture">Disabled texture</param>
        /// <param name="imageType">Image type</param>
        /// <param name="interactable">Button interactability</param>
        /// <returns></returns>
        public static bool Button(int id,
            Rect rect,
            Texture normalTexture,
            Texture hoverTexture,
            Texture activeTexture,
            Texture disabledTexture,
            ImageType imageType,
            bool interactable)
        {
            Color color = Color.white;
            Color colorHov = new Color(0.8f, 0.8f, 0.8f, 1.0f);
            Color colorAct = new Color(0.6f, 0.6f, 0.6f, 1.0f);
            Color colorDis = new Color(0.4f, 0.4f, 0.4f, 1.0f);

            return INTERNAL_button(id, ref rect, normalTexture, hoverTexture, activeTexture, disabledTexture, ref color, ref colorHov, ref colorAct, ref colorDis, (int)imageType, interactable);
        }

        /// <summary>
        /// Draw enabled or disabled button with one texture and tint colors
        /// </summary>
        /// <param name="id">Button id</param>
        /// <param name="rect">Rect on the screen</param>
        /// <param name="normalColor">Normal color</param>
        /// <param name="hoverColor">Hover color</param>
        /// <param name="activeColor">Active color</param>
        /// <param name="disabledColor">Disabled color</param>
        /// <param name="texture">Texture</param>
        /// <param name="imageType">Image type</param>
        /// <param name="interactable">Button interactability</param>
        /// <returns></returns>
        public static bool Button(int id,
            Rect rect,
            Color normalColor,
            Color hoverColor,
            Color activeColor,
            Color disabledColor,
            Texture texture,
            ImageType imageType,
            bool interactable)
        {
            return INTERNAL_button(id, ref rect, texture, texture, texture, texture, ref normalColor, ref hoverColor, ref activeColor, ref disabledColor, (int)imageType, interactable);
        }

        // INTERNAL CALLS //

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_image(Texture texture, ref Rect rect, ref Color color, int imageType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void INTERNAL_text(string text,
            ref Rect rect,
	        Font font,
	        float fontSize,
	        float fontResolution,
	        float lineSpacing,
	        int wordWrap,
	        int horizontalTextAlignment,
	        int verticalTextAlignment,
	        ref Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool INTERNAL_button(int id,
            ref Rect rect,
            Texture texNormal,
            Texture texHover,
            Texture texActive,
            Texture texDisabled,
            ref Color color,
            ref Color colHover,
            ref Color colActive,
            ref Color colDisabled,
            int imageType,
            bool interactable);
    }
}
