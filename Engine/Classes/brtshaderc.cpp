#include <cstdio>
#include <bx/file.h>
#include <vector>
#include <fcntl.h>

#include "../Core/Engine.h"
#include "IO.h"
#include "Helpers.h"
#include "../Core/Debug.h"

// hack to fix the multiple definition link errors
#define getUniformTypeName getUniformTypeName_shaderc
#define nameToUniformTypeEnum nameToUniformTypeEnum_shaderc
#define s_uniformTypeName s_uniformTypeName_shaderc

// hack to defined stuff
#define fprintf printError
#define main fakeMain
#define g_allocator g_shaderc_allocator

// fix warnings
#undef BX_TRACE
#undef BX_WARN
#undef BX_CHECK

// include original shaderc code files
#include "../Bgfx/bgfx/tools/shaderc/shaderc.cpp"
#include "../Bgfx/bgfx/tools/shaderc/shaderc_hlsl.cpp"
#include "../Bgfx/bgfx/tools/shaderc/shaderc_glsl.cpp"

#ifdef _WIN32
#include <io.h>
#endif

namespace bgfx 
{
    bool compilePSSLShader(const Options&, uint32_t, const std::string&, bx::WriterI*)
    {
        return false;
    }

    bool compileSPIRVShader(const Options&, uint32_t, const std::string&, bx::WriterI*)
    {
        return false;
    }

    const char* getPsslPreamble()
    {
        return "";
    }
}

#include "brtshaderc.h"

using namespace bgfx;

class StdOutPipe
{
private:
    char buffer[65536] = { 0 };
    int out_pipe[2] = { 0 };
    int saved_stdout = -1;

public:
    void beginRead()
    {
        if (GX::Engine::getSingleton()->getIsRuntimeMode())
            return;

#ifdef _WIN32
        saved_stdout = dup(_fileno(stdout));
        if (_pipe(out_pipe, sizeof(buffer), O_BINARY) != 0) {}
#else
        saved_stdout = dup(fileno(stdout));
        if (pipe(out_pipe) != 0) {}
#endif

#ifdef _WIN32
        dup2(out_pipe[1], _fileno(stdout));
#else
        dup2(out_pipe[1], fileno(stdout));
#endif
        
        close(out_pipe[1]);
    }

    void endRead()
    {
        if (GX::Engine::getSingleton()->getIsRuntimeMode())
            return;

        if (saved_stdout > -1)
        {
#ifdef _WIN32
            dup2(saved_stdout, _fileno(stdout));
#else
            dup2(saved_stdout, fileno(stdout));
#endif
        }
    }

    std::string getBuffer()
    {
        if (GX::Engine::getSingleton()->getIsRuntimeMode())
            return "";

        fflush(stdout);
        read(out_pipe[0], buffer, 65535); /* read from pipe into buffer */
        return buffer;
    }
};

namespace shaderc
{
    /// not a real FileWriter, but a hack to redirect write() to a memory block.
    class BufferWriter : public bx::FileWriter
    {
    public:

        BufferWriter()
        {
        }

        ~BufferWriter()
        {
        }

        bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override
        {
            return true;
        }

        const bgfx::Memory* finalize()
        {
            if(_buffer.size() > 0)
            {
                _buffer.push_back('\0');

                const bgfx::Memory* mem = bgfx::alloc(_buffer.size());
                bx::memCopy(mem->data, _buffer.data(), _buffer.size());
                return mem;
            }

            return nullptr;
        }

        int32_t write(const void* _data, int32_t _size, bx::Error* _err) override
        {
            const char* data = (const char*)_data;
            _buffer.insert(_buffer.end(), data, data+_size);
            return _size;
        }

    private:
        BX_ALIGN_DECL(16, uint8_t) m_internal[64];
        typedef std::vector<uint8_t> Buffer;
        Buffer _buffer;
    };

    const bgfx::Memory* compileShaderFromSources(ShaderType type, const char* filePath, const char* src, const char* defines, const char* varyingSrc, const char* profile)
    {
        bgfx::Options options;

        options.inputFilePath = filePath;
        options.shaderType = type;

        std::string wdir = GX::IO::GetFilePath(filePath);

        // set platform
#if BX_PLATFORM_LINUX
        options.platform = "linux";
#elif BX_PLATFORM_WINDOWS
        options.platform = "windows";
#elif BX_PLATFORM_ANDROID
        options.platform = "android";
#elif BX_PLATFORM_EMSCRIPTEN
        options.platform = "asm.js";
#elif BX_PLATFORM_IOS
        options.platform = "ios";
#elif BX_PLATFORM_OSX
        options.platform = "osx";
#endif

        // set profile
        if (profile)
        {
            // user profile
            options.profile = profile;
        }
        else
        {
            // set default profile for current running renderer.
            bgfx::RendererType::Enum renderType = bgfx::getRendererType();

            switch (renderType)
            {
            default:
            case bgfx::RendererType::Noop:         //!< No rendering.
                break;
            case bgfx::RendererType::Direct3D9:    //!< Direct3D 9.0
            {
                if (type == 'v')
                    options.profile = "vs_3_0";
                else if (type == 'f')
                    options.profile = "ps_3_0";
                else if (type == 'c')
                    options.profile = "ps_3_0";
            }
            break;
            case bgfx::RendererType::Direct3D11:   //!< Direct3D 11.0
            {
                if (type == 'v')
                    options.profile = "vs_4_0";
                else if (type == 'f')
                    options.profile = "ps_4_0";
                else if (type == 'c')
                    options.profile = "cs_5_0";
            }
            break;
            case bgfx::RendererType::Direct3D12:   //!< Direct3D 12.0
            {
                if (type == 'v')
                    options.profile = "vs_5_0";
                else if (type == 'f')
                    options.profile = "ps_5_0";
                else if (type == 'c')
                    options.profile = "cs_5_0";
            }
            case bgfx::RendererType::Gnm:          //!< GNM
                break;
            case bgfx::RendererType::Metal:        //!< Metal
                break;
            case bgfx::RendererType::OpenGLES:     //!< OpenGL ES 2.0+
                break;
            case bgfx::RendererType::OpenGL:       //!< OpenGL 2.1+
            {
                if (type == 'v' || type == 'f')
                    options.profile = "120";
                else if (type == 'c')
                    options.profile = "430";
            }
            break;
            case bgfx::RendererType::Vulkan:       //!< Vulkan
                break;
            };
        }

        // include current dir
        std::string dir;
        {
            bx::FilePath fp(wdir.c_str());
            bx::StringView path(fp.getPath());

            dir.assign(path.getPtr(), path.getTerm());
            options.includeDirs.push_back(dir);
            options.includeDirs.push_back(GX::Engine::getSingleton()->getBuiltinResourcesPath() + "Shaders/");
            options.includeDirs.push_back(GX::Helper::ExePath() + "Shaders/");
        }

        // set defines
        while (NULL != defines && '\0' != *defines)
        {
            defines = bx::strLTrimSpace(defines).getPtr();
            bx::StringView eol = bx::strFind(defines, ';');
            std::string define(defines, eol.getPtr());
            options.defines.push_back(define.c_str());
            defines = ';' == *eol.getPtr() ? eol.getPtr() + 1 : eol.getPtr();
        }

        // add padding
        const size_t padding = 16384;
        uint32_t size = (uint32_t)std::string(src).length();
        char* data = new char[size + padding + 1];
        char* _src = const_cast<char*>(src);
        memcpy(data, _src, size);

        if (data[0] == '\xef'
            && data[1] == '\xbb'
            && data[2] == '\xbf')
        {
            bx::memMove(data, &data[3], size - 3);
            size -= 3;
        }

        // Compiler generates "error X3000: syntax error: unexpected end of file"
        // if input doesn't have empty line at EOF.
        data[size] = '\n';
        bx::memSet(&data[size + 1], 0, padding);

        std::string commandLineComment = "// shaderc command line:\n";

        StdOutPipe stdOutPipe;
        stdOutPipe.beginRead();

        // compile shader
        BufferWriter writer;
        if (bgfx::compileShader(varyingSrc, commandLineComment.c_str(), data, size, options, &writer))
        {
            stdOutPipe.endRead();
            // this will copy the compiled shader data to a memory block and return mem ptr
            return writer.finalize();
        }

        std::string buffer = stdOutPipe.getBuffer();
        stdOutPipe.endRead();
        if (!buffer.empty())
            GX::Debug::logInfo(buffer);

        return nullptr;
    }
}

// restore previous defines BX_TRACE BX_WARN and BX_CHECK
#include "../../bgfx/tools/shaderc/shaderc.h"
